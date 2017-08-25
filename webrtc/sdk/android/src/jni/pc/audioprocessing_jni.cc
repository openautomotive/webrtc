/*
 *  Copyright 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "webrtc/modules/audio_processing/include/audio_processing.h"
#include "webrtc/sdk/android/src/jni/jni_helpers.h"

namespace webrtc_jni {

JOW(void, AudioProcessing_freeAudioProcessing)(JNIEnv*, jclass, jlong j_p) {
  delete reinterpret_cast<AudioProcessing*>(j_p);
}

JOW(jlong, AudioProcessing_nativeCreateAudioProcessing)(JNIEnv*, jclass) {
  return jlongFromPointer(webrtc::AudioProcessing::Create());
}

}  // namespace webrtc_jni
