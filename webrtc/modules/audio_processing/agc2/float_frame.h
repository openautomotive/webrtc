#ifndef WEBRTC_MODULES_AUDIO_PROCESSING_AGC2_FLOAT_FRAME_H_
#define WEBRTC_MODULES_AUDIO_PROCESSING_AGC2_FLOAT_FRAME_H_

#include "webrtc/api/array_view.h"
namespace webrtc {
class FloatFrame {
 public:
  FloatFrame(float* const * signal, size_t num_channels,
             size_t samples_per_channel)
      : signal_(signal),
        num_channels_(num_channels),
        samples_per_channel_(samples_per_channel) {
    RTC_DCHECK_LT(0, num_channels);
  }

  rtc::ArrayView<float> GetChannel(size_t channel) {
    RTC_DCHECK_LE(0, channel);
    RTC_DCHECK_LE(channel, num_channels_);
    return {signal_[channel], samples_per_channel_};
  }

  rtc::ArrayView<const float> GetChannel(size_t channel) const {
    RTC_DCHECK_LE(0, channel);
    RTC_DCHECK_LE(channel, num_channels_);
    return {signal_[channel], samples_per_channel_};
  }

  size_t num_channels() const { return num_channels_; }
  size_t samples_per_channel() const { return samples_per_channel_; }

 private:
  float* const* const signal_;
  const size_t num_channels_;
  const size_t samples_per_channel_;
};
}  // namespace webrtc

#endif  // WEBRTC_MODULES_AUDIO_PROCESSING_AGC2_FLOAT_FRAME_H_
