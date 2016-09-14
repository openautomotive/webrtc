/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "webrtc/modules/video_coding/utility/quality_scaler.h"

#include <algorithm>
#include <cmath>

namespace webrtc {

namespace {
// Threshold constant used until first downscale (to permit fast rampup).
static const int kMeasureSecondsFastUpscale = 2;
static const int kMeasureSecondsUpscale = 5;
static const int kMeasureSecondsDownscale = 5;
static const int kFramedropPercentThreshold = 60;
// Min width/height to downscale to, set to not go below QVGA, but with some
// margin to permit "almost-QVGA" resolutions, such as QCIF.
static const int kMinDownscaleDimension = 140;
// Initial resolutions corresponding to a bitrate. Aa bit above their actual
// values to permit near-VGA and near-QVGA resolutions to use the same
// mechanism.
static const int kVgaBitrateThresholdKbps = 500;
static const int kVgaNumPixels = 700 * 500;  // 640x480
static const int kQvgaBitrateThresholdKbps = 250;
static const int kQvgaNumPixels = 400 * 300;  // 320x240
}  // namespace

// QP thresholds are chosen to be high enough to be hit in practice when quality
// is good, but also low enough to not cause a flip-flop behavior (e.g. going up
// in resolution shouldn't give so bad quality that we should go back down).

const int QualityScaler::kLowVp8QpThreshold = 29;
const int QualityScaler::kBadVp8QpThreshold = 95;

#if defined(WEBRTC_IOS)
const int QualityScaler::kLowH264QpThreshold = 32;
const int QualityScaler::kBadH264QpThreshold = 42;
#else
const int QualityScaler::kLowH264QpThreshold = 24;
const int QualityScaler::kBadH264QpThreshold = 37;
#endif

// Default values. Should immediately get set to something more sensible.
QualityScaler::QualityScaler()
    : average_qp_(kMeasureSecondsUpscale * 30),
      framedrop_percent_(kMeasureSecondsUpscale * 30),
      low_qp_threshold_(-1) {}

void QualityScaler::Init(int low_qp_threshold,
                         int high_qp_threshold,
                         int initial_bitrate_kbps,
                         int width,
                         int height,
                         int fps) {
  low_qp_threshold_ = low_qp_threshold;
  high_qp_threshold_ = high_qp_threshold;
  downscale_shift_ = 0;

  fast_rampup_ = true;

  ClearSamples();
  ReportFramerate(fps);

  const int init_width = width;
  const int init_height = height;
  if (initial_bitrate_kbps > 0) {
    int init_num_pixels = width * height;
    if (initial_bitrate_kbps < kVgaBitrateThresholdKbps)
      init_num_pixels = kVgaNumPixels;
    if (initial_bitrate_kbps < kQvgaBitrateThresholdKbps)
      init_num_pixels = kQvgaNumPixels;
    while (width * height > init_num_pixels) {
      ++downscale_shift_;
      width /= 2;
      height /= 2;
    }
  }
  UpdateTargetResolution(init_width, init_height);
  ReportFramerate(fps);
}

// Report framerate(fps) to estimate # of samples.
void QualityScaler::ReportFramerate(int framerate) {
  // Use a faster window for upscaling initially.
  // This enables faster initial rampups without risking strong up-down
  // behavior later.
  num_samples_upscale_ = framerate * (fast_rampup_ ? kMeasureSecondsFastUpscale
                                                   : kMeasureSecondsUpscale);
  num_samples_downscale_ = framerate * kMeasureSecondsDownscale;

  average_qp_ =
      MovingAverage(std::max(num_samples_upscale_, num_samples_downscale_));
  framedrop_percent_ =
      MovingAverage(std::max(num_samples_upscale_, num_samples_downscale_));
}

void QualityScaler::ReportQP(int qp) {
  framedrop_percent_.AddSample(0);
  average_qp_.AddSample(qp);
}

void QualityScaler::ReportDroppedFrame() {
  framedrop_percent_.AddSample(100);
}

void QualityScaler::OnEncodeFrame(int width, int height) {
  // Should be set through InitEncode -> Should be set by now.
  RTC_DCHECK_GE(low_qp_threshold_, 0);
  if (target_res_.width != width || target_res_.height != height) {
    UpdateTargetResolution(width, height);
  }

  // Check if we should scale down due to high frame drop.
  const auto drop_rate = framedrop_percent_.GetAverage(num_samples_downscale_);
  if (drop_rate && *drop_rate >= kFramedropPercentThreshold) {
    ScaleDown();
    return;
  }

  // Check if we should scale up or down based on QP.
  const auto avg_qp_down = average_qp_.GetAverage(num_samples_downscale_);
  if (avg_qp_down && *avg_qp_down > high_qp_threshold_) {
    ScaleDown();
    return;
  }
  const auto avg_qp_up = average_qp_.GetAverage(num_samples_upscale_);
  if (avg_qp_up && *avg_qp_up <= low_qp_threshold_) {
    // QP has been low. We want to try a higher resolution.
    ScaleUp();
    return;
  }
}

void QualityScaler::ScaleUp() {
  downscale_shift_ = std::max(0, downscale_shift_ - 1);
  ClearSamples();
}

void QualityScaler::ScaleDown() {
  downscale_shift_ = std::min(maximum_shift_, downscale_shift_ + 1);
  ClearSamples();
  // If we've scaled down, wait longer before scaling up again.
  if (fast_rampup_) {
    fast_rampup_ = false;
    num_samples_upscale_ = (num_samples_upscale_ / kMeasureSecondsFastUpscale) *
                           kMeasureSecondsUpscale;
  }
}

QualityScaler::Resolution QualityScaler::GetScaledResolution() const {
  const int frame_width = target_res_.width >> downscale_shift_;
  const int frame_height = target_res_.height >> downscale_shift_;
  return Resolution{frame_width, frame_height};
}

rtc::scoped_refptr<VideoFrameBuffer> QualityScaler::GetScaledBuffer(
    const rtc::scoped_refptr<VideoFrameBuffer>& frame) {
  Resolution res = GetScaledResolution();
  const int src_width = frame->width();
  const int src_height = frame->height();

  if (res.width == src_width && res.height == src_height)
    return frame;
  rtc::scoped_refptr<I420Buffer> scaled_buffer =
      pool_.CreateBuffer(res.width, res.height);

  scaled_buffer->ScaleFrom(frame);

  return scaled_buffer;
}

void QualityScaler::UpdateTargetResolution(int width, int height) {
  if (width < kMinDownscaleDimension || height < kMinDownscaleDimension) {
    maximum_shift_ = 0;
  } else {
    maximum_shift_ = static_cast<int>(
        std::log2(std::min(width, height) / kMinDownscaleDimension));
  }
  target_res_ = Resolution{width, height};
}

void QualityScaler::ClearSamples() {
  framedrop_percent_.Reset();
  average_qp_.Reset();
}


}  // namespace webrtc
