#include "level_estimator.h"

#include <algorithm>
#include <cmath>

#include "webrtc/modules/audio_processing/agc2/agc2_common.h"
#include "webrtc/modules/audio_processing/logging/apm_data_dumper.h"
#include "webrtc/rtc_base/checks.h"

namespace webrtc {
namespace {

// TODO(aleloi): Change to ComputeDecay if attack is changed to constantly 0.
float ComputeFilterConstant(float time_to_change_one_db_ms,
                            float subframe_duration_ms) {
  const float one_db_reduction_factor = std::pow(10.0, -1.0 / 20.0);
  RTC_CHECK_GE(time_to_change_one_db_ms, 0);
  return time_to_change_one_db_ms == 0.f
             ? 0
             : std::pow(one_db_reduction_factor,
                        subframe_duration_ms / time_to_change_one_db_ms);
}
}  // namespace

namespace agc2 {

LevelEstimator::LevelEstimator(size_t number_of_sub_frames_in_frame,
                               float attack_ms, float decay_ms,
                               size_t sample_rate_hz,
                               ApmDataDumper* apm_data_dumper)
    : filter_state_(
          attack_ms, decay_ms,
          static_cast<float>(kFrameDurationMs) / number_of_sub_frames_in_frame),
      number_of_sub_frames_(number_of_sub_frames_in_frame),
      sample_rate_hz_(sample_rate_hz),
      samples_in_frame_(rtc::CheckedDivExact(sample_rate_hz_ * kFrameDurationMs,
                                             static_cast<size_t>(1000))),
      apm_data_dumper_(apm_data_dumper) {
  CheckParameterCombination();
  // TODO(alessiob): Remove dump below once values become hard-coded.
  RTC_DCHECK(apm_data_dumper_);
  apm_data_dumper_->DumpRaw("agc2_level_estimator_num_subframes",
                            number_of_sub_frames_in_frame);
  apm_data_dumper_->DumpRaw("agc2_level_estimator_attack", attack_ms);
  apm_data_dumper_->DumpRaw("agc2_level_estimator_decay", decay_ms);
  apm_data_dumper_->DumpRaw("agc2_level_estimator_samplerate", sample_rate_hz);
}

LevelEstimator::AttackDecayState::AttackDecayState(
    float attack_ms, float decay_ms, float subframe_duration_ms)
    : attack(ComputeFilterConstant(attack_ms, subframe_duration_ms)),
      decay(ComputeFilterConstant(decay_ms, subframe_duration_ms)) {}

void LevelEstimator::CheckParameterCombination() {
  RTC_CHECK_LE(number_of_sub_frames_, samples_in_frame_);
  RTC_CHECK_EQ(samples_in_frame_ % number_of_sub_frames_, 0);
}

LevelEstimator::EnvelopeFrame LevelEstimator::ComputeLevel(
    const FloatFrame& float_frame) {
  RTC_CHECK_GT(float_frame.num_channels(), 0);
  RTC_CHECK_EQ(float_frame.samples_per_channel(), samples_in_frame_);

  // TODO(aleloi, alessiob): Also handle the single sample subframes case.
  const auto samples_in_sub_frame =
      rtc::CheckedDivExact(samples_in_frame_, number_of_sub_frames_);
  // Compute max envelope without smoothing.
  // TODO(aleloi) change to std::array once we hard-code its size.
  std::vector<float> envelope(number_of_sub_frames_);
  for (size_t channel_idx = 0; channel_idx < float_frame.num_channels();
       ++channel_idx) {
    RTC_DCHECK_EQ(float_frame.GetChannel(channel_idx).size(),
                  samples_in_frame_);
    const auto channel = float_frame.GetChannel(channel_idx);
    for (size_t sub_frame = 0; sub_frame < number_of_sub_frames_;
         ++sub_frame) {
      for (size_t sample_in_sub_frame = 0;
           sample_in_sub_frame < samples_in_sub_frame; ++sample_in_sub_frame) {
        envelope[sub_frame] =
            std::max(envelope[sub_frame],
                     std::abs(channel[sub_frame * samples_in_sub_frame +
                                      sample_in_sub_frame]));
      }
    }
  }

  // Make sure envelope increases happen one step earlier so that the
  // corresponding *gain decrease* doesn't miss a sudden signal
  // increase due to interpolation.
  // TODO(aleloi): consider adding 1 subframe delay instead.
  if (samples_in_sub_frame > 1) {
    for (size_t sub_frame = 0; sub_frame < number_of_sub_frames_ - 1;
         ++sub_frame) {
      if (envelope[sub_frame] < envelope[sub_frame + 1]) {
        envelope[sub_frame] = envelope[sub_frame + 1];
      }
    }
  }

  // Add attack / decay smoothing.
  for (size_t sub_frame = 0; sub_frame < number_of_sub_frames_; ++sub_frame) {
    const float envelope_value = envelope[sub_frame];
    if (envelope_value > filter_state_.last_level) {
      envelope[sub_frame] = envelope_value * (1 - filter_state_.attack) +
                            filter_state_.last_level * filter_state_.attack;
    } else {
      envelope[sub_frame] = envelope_value * (1 - filter_state_.decay) +
                            filter_state_.last_level * filter_state_.decay;
    }
    filter_state_.last_level = envelope[sub_frame];

    // Dump data for debug.
    RTC_DCHECK(apm_data_dumper_);
    const auto channel = float_frame.GetChannel(0);
    apm_data_dumper_->DumpRaw("agc2_level_estimator_samples",
                              samples_in_sub_frame,
                              &channel[sub_frame * samples_in_sub_frame]);
    const std::vector<float> envelope_dump(samples_in_sub_frame,
                                           envelope[sub_frame]);
    apm_data_dumper_->DumpRaw("agc2_level_estimator_level",
                              envelope_dump.size(), envelope_dump.data());
  }

  return envelope;
}

}  // namespace agc2
}  // namespace webrtc
