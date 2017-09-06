/*
 *  Copyright (c) 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "webrtc/modules/audio_processing/agc2/gain_controller2.h"

#include "webrtc/modules/audio_processing/audio_buffer.h"
#include "webrtc/modules/audio_processing/logging/apm_data_dumper.h"
#include "webrtc/rtc_base/atomicops.h"
#include "webrtc/rtc_base/checks.h"

namespace webrtc {

namespace {

constexpr float kGain = 0.5f;

}  // namespace

int GainController2::instance_count_ = 0;

GainController2::GainController2(int sample_rate_hz,
                                 const AudioProcessing::Config::GainController2& config)
    : sample_rate_hz_(sample_rate_hz),
      data_dumper_(new ApmDataDumper(
        rtc::AtomicOps::Increment(&instance_count_))),
      fixed_gain_controller_(config.gain, //gain to apply
                             20, // sub frames in frame
                             0, // attack ms
                             100, // decay ms
                             sample_rate_hz), // sample rate hz
      gain_(kGain) {
  RTC_DCHECK(sample_rate_hz_ == AudioProcessing::kSampleRate8kHz ||
             sample_rate_hz_ == AudioProcessing::kSampleRate16kHz ||
             sample_rate_hz_ == AudioProcessing::kSampleRate32kHz ||
             sample_rate_hz_ == AudioProcessing::kSampleRate48kHz);
  data_dumper_->InitiateNewSetOfRecordings();
  data_dumper_->DumpRaw("gain_", 1, &gain_);
}

GainController2::~GainController2() = default;

void GainController2::Process(AudioBuffer* audio) {
  fixed_gain_controller_.Process(FloatFrame(audio->channels_f(),
                                            audio->num_channels(),
                                            audio->num_frames()));
}

bool GainController2::Validate(
    const AudioProcessing::Config::GainController2& config) {
  return true;
}

std::string GainController2::ToString(
    const AudioProcessing::Config::GainController2& config) {
  std::stringstream ss;
  ss << "{"
     << "enabled: " << (config.enabled ? "true" : "false") << "}";
  return ss.str();
}

}  // namespace webrtc
