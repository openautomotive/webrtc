#include "gain_curve_applier.h"

#include <utility>
#include <string>

#include "agc2_common.h"
#include "testing/base/public/gunit.h"
#include "vector_float_frame.h"
#include "webrtc/rtc_base/array_view.h"

namespace webrtc {
namespace {

// Sets of level estimator parameters.
const std::array<float, 3> kAttacksMs{{0.f, 1.f, 2.f}};
const std::array<float, 3> kDecayesMs{{0.f, 5.f, 10.f}};
const std::array<float, 4> kNumsSubFrames{{10.f, 20.f, 40.f, 480.f}};

std::vector<std::vector<float>> CreateLevelEstimatorConfigSets() {
  rtc::ArrayView<const float> attack_values(kAttacksMs);
  rtc::ArrayView<const float> decay_values(kDecayesMs);
  rtc::ArrayView<const float> num_subframe_values(kNumsSubFrames);
  const std::array<rtc::ArrayView<const float>, 3> sets{
      {num_subframe_values, attack_values, decay_values}};

  // Compute the cartesian product of the elements in |sets|.
  std::vector<std::vector<float>> result = {{}};
  for (const auto& set : sets) {
    std::vector<std::vector<float>> partial_result;
    for (const auto& item : result) {
      for (const auto item_to_add : set) {
        partial_result.push_back(item);
        partial_result.back().push_back(item_to_add);
      }
    }
    result = std::move(partial_result);
  }
  return result;
}

std::string LevelEstimatorConfigToString(const std::vector<float>& config) {
  std::ostringstream ss;
  ss << "n_sf=" << config[0] << " a=" << config[1] << " d=" << config[2];
  return ss.str();
}

}  // namespace

TEST(GainCurveApplier, GainCurveApplierShouldConstructAndRun) {
  const auto kLevelEstimatorConfigs = CreateLevelEstimatorConfigSets();
  const int sample_rate_hz = 48000;
  ApmDataDumper apm_data_dumper(0);

  for (const auto& config : kLevelEstimatorConfigs) {
    SCOPED_TRACE(LevelEstimatorConfigToString(config));
    InterpolatedGainCurve igc(&apm_data_dumper);
    LevelEstimator le(size_t(config[0]), config[1], config[2], sample_rate_hz,
                      &apm_data_dumper);
    GainCurveApplier gain_curve_applier(&igc, &le, &apm_data_dumper);

    VectorFloatFrame vectors_with_float_frame(1, sample_rate_hz / 100,
                                              kMaxSampleValue);
    gain_curve_applier.Process(*vectors_with_float_frame.float_frame());
  }
}

TEST(GainCurveApplier, OutputVolumeAboveThreshold) {
  const auto kLevelEstimatorConfigs = CreateLevelEstimatorConfigSets();
  const int sample_rate_hz = 48000;
  const float input_level =
      (kMaxSampleValue + DbfsToLinear(kLimiterMaxInputLevel)) / 2.f;
  ApmDataDumper apm_data_dumper(0);

  for (const auto& config : kLevelEstimatorConfigs) {
    SCOPED_TRACE(LevelEstimatorConfigToString(config));
    InterpolatedGainCurve igc(&apm_data_dumper);
    LevelEstimator le(size_t(config[0]), config[1], config[2], sample_rate_hz,
                      &apm_data_dumper);
    GainCurveApplier gain_curve_applier(&igc, &le, &apm_data_dumper);

    // Give the level estimator time to adapt.
    for (int i = 0; i < 5; ++i) {
      VectorFloatFrame vectors_with_float_frame(1, sample_rate_hz / 100,
                                                input_level);
      gain_curve_applier.Process(*vectors_with_float_frame.float_frame());
    }

    VectorFloatFrame vectors_with_float_frame(1, sample_rate_hz / 100,
                                              input_level);
    rtc::ArrayView<float> channel =
        vectors_with_float_frame.float_frame()->GetChannel(0);
    gain_curve_applier.Process(*vectors_with_float_frame.float_frame());

    for (const auto& sample : channel) {
      EXPECT_LT(0.9f * kInputLevelScaling, sample);
    }
  }
}

}  // namespace webrtc
