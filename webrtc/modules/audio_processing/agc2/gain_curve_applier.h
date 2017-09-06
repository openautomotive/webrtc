#ifndef WEBRTC_MODULES_AUDIO_PROCESSING_AGC2_GAIN_CURVE_APPLIER_H_
#define WEBRTC_MODULES_AUDIO_PROCESSING_AGC2_GAIN_CURVE_APPLIER_H_

#include <vector>

#include "webrtc/modules/audio_processing/agc2/float_frame.h"
#include "webrtc/modules/audio_processing/agc2/interpolated_gain_curve.h"
#include "webrtc/modules/audio_processing/agc2/level_estimator.h"
#include "webrtc/modules/audio_processing/logging/apm_data_dumper.h"
#include "webrtc/rtc_base/constructormagic.h"

namespace webrtc {

class GainCurveApplier {
 public:
  GainCurveApplier(GainCurveApplier&&);
  GainCurveApplier(const InterpolatedGainCurve* interp_gain_curve,
                   agc2::LevelEstimator* estimator,
                   ApmDataDumper* apm_data_dumper);

  ~GainCurveApplier();

  void Process(FloatFrame signal);
  InterpolatedGainCurve::Stats GetGainCurveStats() const;

 private:
  const InterpolatedGainCurve& interp_gain_curve_;
  agc2::LevelEstimator& level_estimator_;
  // Work array containing the sub-frame scaling factors to be interpolated.
  ApmDataDumper* const apm_data_dumper_;
  std::vector<float> scaling_factors_;
  float last_scaling_factor_ = 1.f;

  RTC_DISALLOW_COPY_AND_ASSIGN(GainCurveApplier);
};

}  // namespace webrtc

#endif  // WEBRTC_MODULES_AUDIO_PROCESSING_AGC2_GAIN_CURVE_APPLIER_H_
