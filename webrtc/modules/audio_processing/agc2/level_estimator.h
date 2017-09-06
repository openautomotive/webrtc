#ifndef WEBRTC_MODULES_AUDIO_PROCESSING_AGC2_LEVEL_ESTIMATOR_H_
#define WEBRTC_MODULES_AUDIO_PROCESSING_AGC2_LEVEL_ESTIMATOR_H_

#include <memory>
#include <vector>

#include "webrtc/modules/audio_processing/agc2/float_frame.h"
#include "webrtc/rtc_base/constructormagic.h"

namespace webrtc {

class ApmDataDumper;
namespace agc2 {
class LevelEstimator {
 public:
  // TODO(aleloi): change to std::array<float, kNumSubframes> (and remove the
  // 'using') once we decide the kNumSubframes constant value.
  using EnvelopeFrame = std::vector<float>;

  LevelEstimator(LevelEstimator&&) = default;
  // TODO(aleloi): hard-code the constants when we have tuned them (namely,
  // number_of_sub_frames, attack_ms, and decay_ms).
  LevelEstimator(size_t number_of_sub_frames_in_frame, float attack_ms,
                 float decay_ms, size_t sample_rate_hz,
                 ApmDataDumper* apm_data_dumper);

  size_t number_of_sub_frames() const { return number_of_sub_frames_; }

  // The input level need not be normalized; the operation of LevelEstimator
  // is invariant under scaling of the input level.
  EnvelopeFrame ComputeLevel(const FloatFrame& float_frame);

 private:
  void CheckParameterCombination();

  // TODO(aleloi, alessiob): come up with a good name for this one.
  struct AttackDecayState {
    AttackDecayState(float attack_ms, float decay_ms,
                     float subframe_duration_ms);
    const float attack;
    const float decay;
    float last_level = 0.f;
  } filter_state_;

  const size_t number_of_sub_frames_;
  const size_t sample_rate_hz_;
  const size_t samples_in_frame_;

  ApmDataDumper* const apm_data_dumper_;

  RTC_DISALLOW_COPY_AND_ASSIGN(LevelEstimator);
};

}  // namespace agc2
}  // namespace webrtc

#endif  // WEBRTC_MODULES_AUDIO_PROCESSING_AGC2_LEVEL_ESTIMATOR_H_
