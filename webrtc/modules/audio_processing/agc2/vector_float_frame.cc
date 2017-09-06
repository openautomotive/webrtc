#include "webrtc/modules/audio_processing/agc2/vector_float_frame.h"

namespace webrtc {

namespace {

std::vector<float*> ConstructChannelPointers(
    std::vector<std::vector<float>>* x) {
  std::vector<float*> channel_ptrs;
  for (auto& v : *x) {
    channel_ptrs.push_back(v.data());
  }
  return channel_ptrs;
}
}  // namespace

VectorFloatFrame::VectorFloatFrame(int num_channels, int samples_per_channel,
                                   float start_value)
    : channels_(num_channels,
                std::vector<float>(samples_per_channel, start_value)),
      channel_ptrs_(ConstructChannelPointers(&channels_)),
      float_frame_(channel_ptrs_.data(), channels_.size(),
                   samples_per_channel) {}

}  // namespace webrtc
