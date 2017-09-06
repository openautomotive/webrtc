#include "webrtc/modules/audio_processing/agc2/fixed_gain_controller.h"

#include <algorithm>
#include <cmath>

#include "webrtc/modules/audio_processing/agc2/agc2_common.h"
#include "webrtc/modules/audio_processing/agc2/interpolated_gain_curve.h"
#include "webrtc/modules/audio_processing/agc2/level_estimator.h"
#include "webrtc/rtc_base/array_view.h"
#include "webrtc/rtc_base/checks.h"

namespace webrtc {

int FixedGainController::instance_count_ = 0;

FixedGainController::FixedGainController(float gain_to_apply_db)
    : gain_to_apply_(std::pow(10.0, gain_to_apply_db / 20.0)),
      apm_data_dumper_(instance_count_),
      interp_gain_curve_(),
      level_estimator_(),
      gain_curve_applier_() {
  RTC_DCHECK_LT(0.f, gain_to_apply_);
  ++instance_count_;
}

FixedGainController::FixedGainController(float gain_to_apply_db,
                                         size_t number_of_sub_frames_in_frame,
                                         float attack_ms, float decay_ms,
                                         size_t sample_rate_hz)
    : gain_to_apply_(std::pow(10.0, gain_to_apply_db / 20.0)),
      apm_data_dumper_(instance_count_),
      interp_gain_curve_(InterpolatedGainCurve(&apm_data_dumper_)),
      level_estimator_(agc2::LevelEstimator(number_of_sub_frames_in_frame,
                                            attack_ms,
                                            decay_ms, sample_rate_hz,
                                            &apm_data_dumper_)),
      gain_curve_applier_(
          {&*interp_gain_curve_, &*level_estimator_, &apm_data_dumper_}) {
  RTC_DCHECK_LT(0.f, gain_to_apply_);
  ++instance_count_;
}

FixedGainController::~FixedGainController() = default;

void FixedGainController::Process(FloatFrame signal) {
  // Apply fixed digital gain.
  if (gain_to_apply_ != 1.f) {
    for (size_t k = 0; k < signal.num_channels(); ++k) {
      rtc::ArrayView<float> channel_view = signal.GetChannel(k);
      for (auto& sample : channel_view) {
        sample *= gain_to_apply_;
      }
    }
  }

  // Use the limiter (if injected).
  if (gain_curve_applier_) {
    gain_curve_applier_->Process(signal);

    // Dump data for debug.
    const auto channel_view = signal.GetChannel(0);
    apm_data_dumper_.DumpRaw("agc2_fixed_digital_gain_curve_applier",
                             channel_view.size(), channel_view.data());
  }

  // Hard-clipping.
  for (size_t k = 0; k < signal.num_channels(); ++k) {
    rtc::ArrayView<float> channel_view = signal.GetChannel(k);
    for (auto& sample : channel_view) {
      sample = std::max(kMinSampleValue, std::min(sample,
                                                  kMaxSampleValue));
    }
  }
}

}  // namespace webrtc
