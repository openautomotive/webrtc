#include "fixed_gain_controller.h"
#include "testing/base/public/gunit.h"
#include "vector_float_frame.h"
#include "webrtc/modules/audio_processing/logging/apm_data_dumper.h"
#include "webrtc/rtc_base/array_view.h"

namespace webrtc {
namespace {

constexpr float kInputLevelLinear = 15000.f;

constexpr float kGainToApplyDb = 15.f;

// Level estimator params.
constexpr size_t kNumSubFrames = 10;
constexpr float kAttackMs = 0.05f;
constexpr float kDecayMs = 10.f;
constexpr size_t kSampleRate = 48000;
constexpr size_t kAudioFrameSamples = 480;

float RunFixedGainControllerWithConstantInput(FixedGainController* fixed_gc,
                                              const float input_level,
                                              const size_t num_frames) {
  // Give time to the level etimator to converge.
  for (size_t i = 0; i < num_frames; ++i) {
    VectorFloatFrame vectors_with_float_frame(1, kAudioFrameSamples,
                                              input_level);
    fixed_gc->Process(*vectors_with_float_frame.float_frame());
  }

  // Process the last frame with constant input level.
  VectorFloatFrame vectors_with_float_frame_last(1, kAudioFrameSamples,
                                                 input_level);
  fixed_gc->Process(*vectors_with_float_frame_last.float_frame());

  // Return the last sample from the last processed frame.
  const auto channel =
      vectors_with_float_frame_last.float_frame()->GetChannel(0);
  return channel[channel.size() - 1];
}

}  // namespace

TEST(AutomaticGainController2FixedDigital, CreateUseWithoutLimiter) {
  FixedGainController fixed_gc(kGainToApplyDb);
  VectorFloatFrame vectors_with_float_frame(1, kAudioFrameSamples,
                                            kInputLevelLinear);
  auto float_frame = *vectors_with_float_frame.float_frame();
  fixed_gc.Process(float_frame);
  const auto channel = float_frame.GetChannel(0);
  EXPECT_LT(kInputLevelLinear, channel[0]);
}

TEST(AutomaticGainController2FixedDigital, CreateUseWithLimiter) {
  FixedGainController fixed_gc(kGainToApplyDb, kNumSubFrames, kAttackMs,
                               kDecayMs, kSampleRate);
  VectorFloatFrame vectors_with_float_frame(1, kAudioFrameSamples,
                                            kInputLevelLinear);
  auto float_frame = *vectors_with_float_frame.float_frame();
  fixed_gc.Process(float_frame);
  const auto channel = float_frame.GetChannel(0);
  EXPECT_LT(kInputLevelLinear, channel[0]);
}

TEST(AutomaticGainController2FixedDigital, CheckSaturationBehaviorWithLimiter) {
  const float input_level = 32767.f;
  const size_t num_frames = 5;

  const auto gains_no_saturation =
      LinSpace(0.1, kLimiterMaxInputLevel - 0.01, 10);
  for (const auto gain : gains_no_saturation) {
    // The positive margin |kLimiterMaxInputLevel| - |gain| prevents
    // saturation (for input levels equal to or less than the margin).
    FixedGainController fixed_gc_no_saturation(gain, kNumSubFrames, kAttackMs,
                                               kDecayMs, kSampleRate);

    // Saturation not expected.
    SCOPED_TRACE(std::to_string(gain));
    EXPECT_LT(RunFixedGainControllerWithConstantInput(&fixed_gc_no_saturation,
                                                      input_level, num_frames),
              32767.f);
  }

  const auto gains_saturation = LinSpace(kLimiterMaxInputLevel + 0.01, 10, 10);
  for (const auto gain : gains_saturation) {
    // The role relationship is now reversed, that is the negative margin
    // |kLimiterMaxInputLevel| - |gain| leads  to saturation (for input levels
    // equal to or greater than the margin).
    FixedGainController fixed_gc_saturation(gain, kNumSubFrames, kAttackMs,
                                            kDecayMs, kSampleRate);

    // Saturation expected.
    SCOPED_TRACE(std::to_string(gain));
    EXPECT_FLOAT_EQ(RunFixedGainControllerWithConstantInput(
                        &fixed_gc_saturation, input_level, num_frames),
                    32767.f);
  }
}

TEST(AutomaticGainController2FixedDigital,
     CheckSaturationBehaviorWithLimiterSingleSample) {
  const float input_level = 32767.f;
  const size_t num_frames = 5;

  const auto gains_no_saturation =
      LinSpace(0.1, kLimiterMaxInputLevel - 0.01, 10);
  for (const auto gain : gains_no_saturation) {
    // The positive margin |kLimiterMaxInputLevel| - |gain| prevents saturation
    // (for input levels equal to or less than the margin).
    FixedGainController fixed_gc_no_saturation(
        gain, rtc::CheckedDivExact<size_t>(kSampleRate, 100), 0.f, kDecayMs,
        kSampleRate);

    // Saturation not expected.
    SCOPED_TRACE(std::to_string(gain));
    EXPECT_LT(RunFixedGainControllerWithConstantInput(&fixed_gc_no_saturation,
                                                      input_level, num_frames),
              32767.f);
  }

  const auto gains_saturation = LinSpace(kLimiterMaxInputLevel + 0.01, 10, 10);
  for (const auto gain : gains_saturation) {
    // The role relationship is now swapped, that is the negative margin
    // |kLimiterMaxInputLevel| - |gain| leads  to saturation (for input levels
    // equal to or greater than the margin).
    FixedGainController fixed_gc_saturation(gain, kNumSubFrames, kAttackMs,
                                            kDecayMs, kSampleRate);

    // Saturation expected.
    SCOPED_TRACE(std::to_string(gain));
    EXPECT_FLOAT_EQ(RunFixedGainControllerWithConstantInput(
                        &fixed_gc_saturation, input_level, num_frames),
                    32767.f);
  }
}

}  // namespace webrtc
