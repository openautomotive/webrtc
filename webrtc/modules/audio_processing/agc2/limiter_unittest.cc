#include "webrtc/modules/audio_processing/agc2/limiter.h"

#include "testing/base/public/gunit.h"

namespace webrtc {
namespace {

}  // namespace

TEST(FixedDigitalGainController2Limiter, ConstructDestruct) { Limiter l; }

TEST(FixedDigitalGainController2Limiter, GainCurveShouldBeMonotone) {
  Limiter l;
  float last_output_level = 0.f;
  bool has_last_output_level = false;
  for (float level = -90; level <= l.max_input_level_db(); level += 0.5f) {
    const float current_output_level = l.GetOutputLevelDbfs(level);
    if (!has_last_output_level) {
      last_output_level = current_output_level;
      has_last_output_level = true;
    }
    EXPECT_LE(last_output_level, current_output_level);
    last_output_level = current_output_level;
  }
}

TEST(FixedDigitalGainController2Limiter, GainCurveShouldBeContinuous) {
  Limiter l;
  float last_output_level = 0.f;
  bool has_last_output_level = false;
  constexpr float kMaxDelta = 0.5f;
  for (float level = -90; level <= l.max_input_level_db(); level += 0.5f) {
    const float current_output_level = l.GetOutputLevelDbfs(level);
    if (!has_last_output_level) {
      last_output_level = current_output_level;
      has_last_output_level = true;
    }
    EXPECT_LE(current_output_level, last_output_level + kMaxDelta);
    last_output_level = current_output_level;
  }
}

TEST(FixedDigitalGainController2Limiter, OutputGainShouldBeLessThanFullScale) {
  Limiter l;
  for (float level = -90; level <= l.max_input_level_db(); level += 0.5f) {
    const float current_output_level = l.GetOutputLevelDbfs(level);
    EXPECT_LE(current_output_level, 0.f);
  }
}

}  // namespace webrtc
