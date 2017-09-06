#ifndef WEBRTC_MODULES_AUDIO_PROCESSING_AGC2_VECTOR_FLOAT_FRAME_H_
#define WEBRTC_MODULES_AUDIO_PROCESSING_AGC2_VECTOR_FLOAT_FRAME_H_

#include <vector>

#include "webrtc/modules/audio_processing/agc2/float_frame.h"

namespace webrtc {

// A construct consisting of a multi-channel audio frame, and a FloatFrame view
// of it.
class VectorFloatFrame {
 public:
  VectorFloatFrame(int num_channels, int samples_per_channel,
                   float start_value);
  const FloatFrame* float_frame() const { return &float_frame_; }
  FloatFrame* float_frame() { return &float_frame_; }

 private:
  std::vector<std::vector<float>> channels_;
  std::vector<float*> channel_ptrs_;
  FloatFrame float_frame_;
};

}  // namespace webrtc

#endif  // WEBRTC_MODULES_AUDIO_PROCESSING_AGC2_VECTOR_FLOAT_FRAME_H_
