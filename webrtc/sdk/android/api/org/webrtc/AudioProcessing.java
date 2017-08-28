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
  // Package protected for PeerConnectionFactory.
  final long nativeAudioProcessing;

  public AudioProcessing() {
    nativeAudioProcessing = nativeCreateAudioProcessing();
  }

  public void dispose() {
    freeAudioProcessing(nativeAudioProcessing);
  }

  private static native void freeAudioProcessing(long nativeAudioProcessing);

  private static native long nativeCreateAudioProcessing();
}
