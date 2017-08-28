/*
 *  Copyright 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "webrtc/sdk/android/src/jni/pc/audio_jni.h"

#include "webrtc/api/audio_codecs/builtin_audio_decoder_factory.h"
#include "webrtc/api/audio_codecs/builtin_audio_encoder_factory.h"
#include "webrtc/modules/audio_processing/include/audio_processing.h"

namespace webrtc_jni {

rtc::scoped_refptr<webrtc::AudioDecoderFactory> CreateAudioDecoderFactory() {
  return webrtc::CreateBuiltinAudioDecoderFactory();
}

rtc::scoped_refptr<webrtc::AudioEncoderFactory> CreateAudioEncoderFactory() {
  return webrtc::CreateBuiltinAudioEncoderFactory();
}

webrtc::AudioProcessing* CreateAudioProcessing() {
  return webrtc::AudioProcessing::Create();
}

}  // namespace webrtc_jni
