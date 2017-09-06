#include "agc2_common.h"
#include "testing/base/public/gunit.h"

namespace webrtc {
namespace {}  // namespace

TEST(AutomaticGainController2Common, TestLinSpace) {
  std::vector<double> points1 = LinSpace(-1.0, 2.0, 4);
  const std::vector<double> expected_points1{{-1.0, 0.0, 1.0, 2.0}};
  EXPECT_EQ(expected_points1, points1);

  std::vector<double> points2 = LinSpace(0.0, 1.0, 4);
  const std::vector<double> expected_points2{{0.0, 1.0 / 3.0, 2.0 / 3.0, 1.0}};
  EXPECT_EQ(points2, expected_points2);
}

}  // namespace webrtc
