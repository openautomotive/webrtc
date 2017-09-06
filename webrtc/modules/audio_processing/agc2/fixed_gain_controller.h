#ifndef WEBRTC_MODULES_AUDIO_PROCESSING_AGC2_FIXED_GAIN_CONTROLLER_H_
#define WEBRTC_MODULES_AUDIO_PROCESSING_AGC2_FIXED_GAIN_CONTROLLER_H_

#include "webrtc/modules/audio_processing/agc2/float_frame.h"
#include "webrtc/modules/audio_processing/agc2/gain_curve_applier.h"
#include "webrtc/modules/audio_processing/agc2/interpolated_gain_curve.h"
#include "webrtc/modules/audio_processing/agc2/level_estimator.h"
#include "webrtc/rtc_base/optional.h"

namespace webrtc {

class FixedGainController {
 public:
  // TODO(alessiob): Once the params become hard-coded, switch to a single ctor
  // with two parameters: gain to apply and flag to enable the limiter.
  explicit FixedGainController(float gain_to_apply_db);  // Without limiter.
  FixedGainController(float gain_to_apply_db,            // With limiter.
                      // Level estimator params.
                      size_t number_of_sub_frames_in_frame, float attack_ms,
                      float decay_ms, size_t sample_rate_hz);
  ~FixedGainController();
  void Process(FloatFrame signal);

 private:
  static int instance_count_;
  const float gain_to_apply_;
  mutable ApmDataDumper apm_data_dumper_;
  rtc::Optional<InterpolatedGainCurve> interp_gain_curve_;
  rtc::Optional<agc2::LevelEstimator> level_estimator_;
  rtc::Optional<GainCurveApplier> gain_curve_applier_;

  RTC_DISALLOW_COPY_AND_ASSIGN(FixedGainController);
};

}  // namespace webrtc

#endif  // WEBRTC_MODULES_AUDIO_PROCESSING_AGC2_FIXED_GAIN_CONTROLLER_H_
