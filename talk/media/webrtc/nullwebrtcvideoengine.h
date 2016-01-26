/*
 * libjingle
 * Copyright 2016 Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TALK_MEDIA_WEBRTC_NULLWEBRTCVIDEOENGINE_H_
#define TALK_MEDIA_WEBRTC_NULLWEBRTCVIDEOENGINE_H_

#include <vector>

#include "talk/media/base/mediachannel.h"
#include "talk/media/base/mediaengine.h"

namespace webrtc {

class Call;

}  // namespace webrtc


namespace cricket {

class VideoMediaChannel;
class WebRtcVideoDecoderFactory;
class WebRtcVideoEncoderFactory;

// Video engine implementation that does nothing and can be used in
// CompositeMediaEngine.
class NullWebRtcVideoEngine {
 public:
  NullWebRtcVideoEngine() {}
  ~NullWebRtcVideoEngine() {}

  void SetExternalDecoderFactory(WebRtcVideoDecoderFactory* decoder_factory) {}
  void SetExternalEncoderFactory(WebRtcVideoEncoderFactory* encoder_factory) {}

  void Init() {}

  const std::vector<VideoCodec>& codecs() {
    return codecs_;
  }

  RtpCapabilities GetCapabilities() {
    RtpCapabilities capabilities;
    return capabilities;
  }

  VideoMediaChannel* CreateChannel(webrtc::Call* call,
      const VideoOptions& options) {
    return nullptr;
  }

 private:
  std::vector<VideoCodec> codecs_;
};

}  // namespace cricket

#endif  // TALK_MEDIA_WEBRTC_NULLWEBRTCVIDEOENGINE_H_
