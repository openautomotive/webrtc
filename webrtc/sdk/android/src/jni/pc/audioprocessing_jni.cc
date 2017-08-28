/*
 *  Copyright 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "webrtc/sdk/android/src/jni/jni_helpers.h"
#include "webrtc/sdk/android/src/jni/pc/audio_jni.h"

namespace webrtc_jni {

JNI_FUNCTION_DECLARATION(void,
                         AudioProcessing_freeAudioProcessing,
                         JNIEnv*,
                         jclass,
                         jlong j_p) {
  delete reinterpret_cast<webrtc::AudioProcessing*>(j_p);
}

JNI_FUNCTION_DECLARATION(jlong,
                         AudioProcessing_nativeCreateAudioProcessing,
                         JNIEnv*,
                         jclass) {
  return jlongFromPointer(CreateAudioProcessing());
}

}  // namespace webrtc_jni
