/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "webrtc/video_frame.h"

#include <string.h>

#include <algorithm>  // swap

#include "webrtc/base/bind.h"
#include "webrtc/base/checks.h"

namespace webrtc {

// FFmpeg's decoder, used by H264DecoderImpl, requires up to 8 bytes padding due
// to optimized bitstream readers. See avcodec_decode_video2.
const size_t EncodedImage::kBufferPaddingBytesH264 = 8;

VideoFrame::VideoFrame()
    : video_frame_buffer_(nullptr),
      timestamp_rtp_(0),
      ntp_time_ms_(0),
      timestamp_us_(0),
      rotation_(kVideoRotation_0) {}

VideoFrame::VideoFrame(const rtc::scoped_refptr<VideoFrameBuffer>& buffer,
                       webrtc::VideoRotation rotation,
                       int64_t timestamp_us)
    : video_frame_buffer_(buffer),
      timestamp_rtp_(0),
      ntp_time_ms_(0),
      timestamp_us_(timestamp_us),
      rotation_(rotation) {}

VideoFrame::VideoFrame(const rtc::scoped_refptr<VideoFrameBuffer>& buffer,
                       uint32_t timestamp,
                       int64_t render_time_ms,
                       VideoRotation rotation)
    : video_frame_buffer_(buffer),
      timestamp_rtp_(timestamp),
      ntp_time_ms_(0),
      timestamp_us_(render_time_ms * rtc::kNumMicrosecsPerMillisec),
      rotation_(rotation) {
  RTC_DCHECK(buffer);
}

int VideoFrame::width() const {
  return video_frame_buffer_ ? video_frame_buffer_->width() : 0;
}

int VideoFrame::height() const {
  return video_frame_buffer_ ? video_frame_buffer_->height() : 0;
}

bool VideoFrame::IsZeroSize() const {
  return !video_frame_buffer_;
}

const rtc::scoped_refptr<VideoFrameBuffer>& VideoFrame::video_frame_buffer()
    const {
  return video_frame_buffer_;
}

size_t EncodedImage::GetBufferPaddingBytes(VideoCodecType codec_type) {
  switch (codec_type) {
    case kVideoCodecVP8:
    case kVideoCodecVP9:
      return 0;
    case kVideoCodecH264:
      return kBufferPaddingBytesH264;
    case kVideoCodecI420:
    case kVideoCodecRED:
    case kVideoCodecULPFEC:
    case kVideoCodecGeneric:
    case kVideoCodecUnknown:
      return 0;
  }
  RTC_NOTREACHED();
  return 0;
}

}  // namespace webrtc
