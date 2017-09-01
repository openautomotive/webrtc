/*
 *  Copyright 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

package org.webrtc;

/** Java wrapper for a C++ AudioProcessing module. */
public class AudioProcessing {
  final long nativeAudioProcessing; // Package protected for PeerConnectionFactory.

  public AudioProcessing() {
    nativeAudioProcessing = nativeCreateAudioProcessing();
  }

  // Must be called if and only if the AudioProcessing instance is not used to create a
  // PeerConnectionFactory.
  public void release() {
    JniCommon.nativeReleaseRef(nativeAudioProcessing);
  }

  private static native long nativeCreateAudioProcessing();
}
