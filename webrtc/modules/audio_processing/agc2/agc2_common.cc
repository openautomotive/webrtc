#include "webrtc/modules/audio_processing/agc2/agc2_common.h"

#include <cmath>

#include "webrtc/rtc_base/checks.h"

namespace webrtc {

double DbfsToLinear(const double level) {
  return kInputLevelScaling * std::pow(10.0, level / 20.0);
}

double LinearToDbfs(const double level) {
  if (std::abs(level) <= kInputLevelScaling / 32768.0) {
    return kMinDbfs;
  }
  return 20.0 * std::log10(level / kInputLevelScaling);
}

std::vector<double> LinSpace(const double l, const double r,
                             size_t num_points) {
  RTC_CHECK(num_points >= 2);
  std::vector<double> points(num_points);
  const double step = (r - l) / (num_points - 1.0);
  points[0] = l;
  for (size_t i = 1; i < num_points - 1; i++) {
    points[i] = static_cast<double>(l) + i * step;
  }
  points[num_points - 1] = r;
  return points;
}

}  // namespace webrtc
