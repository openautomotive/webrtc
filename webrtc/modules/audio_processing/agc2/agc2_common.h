#ifndef WEBRTC_MODULES_AUDIO_PROCESSING_AGC2_AGC2_COMMON_H_
#define WEBRTC_MODULES_AUDIO_PROCESSING_AGC2_AGC2_COMMON_H_

#include <cmath>
#include <vector>

#include "webrtc/rtc_base/basictypes.h"

namespace webrtc {

constexpr float kMinSampleValue = -32768.f;
constexpr float kMaxSampleValue = 32767.f;

constexpr double kInputLevelScaling = 32768.0;
const double kMinDbfs = -20.0 * std::log10(32768.0);

constexpr size_t kFrameDurationMs = 10;

// Limiter params.
constexpr double kLimiterMaxInputLevel = 1.0;
constexpr double kLimiterKneeSmoothness = 1.0;
constexpr double kLimiterCompressionRatio = 5.0;

// Number of interpolation points for each region of the limiter.
// These values have been tuned to limit the interpolated gain curve error given
// the limiter parameters and allowing a maximum error of +/- 32768^-1.
constexpr size_t kInterpolatedGainCurveKneePoints = 22;
constexpr size_t kInterpolatedGainCurveBeyondKneePoints = 10;

double DbfsToLinear(const double level);

double LinearToDbfs(const double level);

std::vector<double> LinSpace(const double l, const double r, size_t num_points);

}  // namespace webrtc

#endif  // WEBRTC_MODULES_AUDIO_PROCESSING_AGC2_AGC2_COMMON_H_
