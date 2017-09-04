/*
 *  Copyright (c) 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_MODULES_AUDIO_PROCESSING_INCLUDE_POST_PROCESSING_H_
#define WEBRTC_MODULES_AUDIO_PROCESSING_INCLUDE_POST_PROCESSING_H_

namespace webrtc {

class AudioBuffer;

// Interface for the post processing submodule.
class PostProcessing {
 public:
  // (Re-)Initializes the submodule.
  virtual void Initialize(int sample_rate_hz, int num_channels) = 0;

  // Processes the given capture or render signal.
  virtual void Process(AudioBuffer* audio) = 0;

  virtual ~PostProcessing() {}
};

}  // namespace webrtc

#endif  // WEBRTC_MODULES_AUDIO_PROCESSING_INCLUDE_POST_PROCESSING_H_
