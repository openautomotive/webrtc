/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <stdio.h>

#include <memory>

#include "gflags/gflags.h"
#include "webrtc/common_types.h"
#include "webrtc/modules/audio_coding/codecs/pcm16b/pcm16b.h"
#include "webrtc/modules/audio_coding/include/audio_coding_module.h"
#include "webrtc/modules/include/module_common_types.h"
#include "webrtc/system_wrappers/include/clock.h"
#include "webrtc/test/gtest.h"
#include "webrtc/test/testsupport/fileutils.h"

// Delay logging
DEFINE_string(delay, "", "Log for delay.");

// Delay logging
DEFINE_int32(simulate_time_ms, 1000, "Simulate time (ms).");

const int32_t kAudioPlayedOut = 0x00000001;
const int32_t kPacketPushedIn = 0x00000001 << 1;
const int kPlayoutPeriodMs = 10;
constexpr int kSampleRateHz = 48000;
constexpr int kPayloadType = 108;

namespace webrtc {

class InsertPacketWithTiming {
 public:
  InsertPacketWithTiming()
      : receiver_clock_(new SimulatedClock(0)),
        receive_acm_(AudioCodingModule::Create(0, receiver_clock_)),
        time_to_insert_packet_ms_(3),  // An arbitrary offset on pushing packet.
        time_to_playout_audio_ms_(kPlayoutPeriodMs) {
    SetNum10msPerFrame(2);
  }

  void SetNum10msPerFrame(size_t num_10ms_per_frame) {
    num_10ms_per_frame_ = num_10ms_per_frame;
    frame_size_samples_ = kSampleRateHz * num_10ms_per_frame_ / 100;
    payload_size_bytes_ = frame_size_samples_ * 2;
    payload_.reset(new uint8_t[payload_size_bytes_]);

    int16_t audio[frame_size_samples_];
    const int kRange = 0x7FF;  // 2047, easy for masking.
    for (size_t n = 0; n < frame_size_samples_; ++n)
      audio[n] = (rand() & kRange) - kRange / 2;
    WebRtcPcm16b_Encode(audio, frame_size_samples_, payload_.get());
  }

  void SetUp() {
    ASSERT_TRUE(receiver_clock_ != NULL);

    ASSERT_TRUE(receive_acm_.get() != NULL);

    ASSERT_EQ(0, receive_acm_->InitializeReceiver());
    ASSERT_EQ(true, receive_acm_->RegisterReceiveCodec(
                        kPayloadType, {"L16", kSampleRateHz, 1}));
    rtp_info_.header.payloadType = kPayloadType;
    rtp_info_.header.timestamp = 0;
    rtp_info_.header.ssrc = 0x12345678;
    rtp_info_.header.markerBit = false;
    rtp_info_.header.sequenceNumber = 0;
    rtp_info_.type.Audio.channel = 1;
    rtp_info_.type.Audio.isCNG = false;
    rtp_info_.frameType = kAudioFrameSpeech;
  }

  void TickOneMillisecond(uint32_t* action) {
    // One millisecond passed.
    time_to_insert_packet_ms_--;
    time_to_playout_audio_ms_--;
    receiver_clock_->AdvanceTimeMilliseconds(1);

    // Reset action.
    *action = 0;

    // Is it time to pull audio?
    if (time_to_playout_audio_ms_ == 0) {
      time_to_playout_audio_ms_ = kPlayoutPeriodMs;
      bool muted;
      receive_acm_->PlayoutData10Ms(kSampleRateHz, &frame_, &muted);
      ASSERT_FALSE(muted);
      *action |= kAudioPlayedOut;
    }

    // Is it time to push in next packet?
    if (time_to_insert_packet_ms_ <= .5) {
      *action |= kPacketPushedIn;
      time_to_insert_packet_ms_ += num_10ms_per_frame_ * 10;

      rtp_info_.header.timestamp += frame_size_samples_;
      rtp_info_.header.sequenceNumber++;
      // Push in just enough audio.
      ASSERT_EQ(0, receive_acm_->IncomingPacket(
                       payload_.get(), payload_size_bytes_, rtp_info_));
    }
  }

  ~InsertPacketWithTiming() {
    delete receiver_clock_;
  }

  // Jitter buffer delay.
  void Delay(int* optimal_delay, int* current_delay) {
    NetworkStatistics statistics;
    receive_acm_->GetNetworkStatistics(&statistics);
    *optimal_delay = statistics.preferredBufferSize;
    *current_delay = statistics.currentBufferSize;
  }

 private:
  WebRtcRTPHeader rtp_info_;
  AudioFrame frame_;

  // This class just creates these pointers, not deleting them. They are deleted
  // by the associated ACM.
  SimulatedClock* receiver_clock_;

  std::unique_ptr<AudioCodingModule> receive_acm_;

  float time_to_insert_packet_ms_;
  uint32_t time_to_playout_audio_ms_;

  size_t num_10ms_per_frame_;
  size_t frame_size_samples_;
  size_t payload_size_bytes_;
  std::unique_ptr<uint8_t[]> payload_;
};

}  // webrtc

int main(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  webrtc::InsertPacketWithTiming test;
  test.SetUp();

  FILE* delay_log = NULL;
  if (FLAGS_delay.size() > 0) {
    delay_log = fopen(FLAGS_delay.c_str(), "wt");
    if (delay_log == NULL) {
      std::cout << "Cannot open the file to log delay values." << std::endl;
      exit(1);
    }
  }

  uint32_t action_taken;
  int optimal_delay_ms;
  int current_delay_ms;
  for (int i = 0; i < FLAGS_simulate_time_ms; ++i) {
    test.TickOneMillisecond(&action_taken);

    if (action_taken != 0) {
      test.Delay(&optimal_delay_ms, &current_delay_ms);
      if (delay_log != NULL) {
        fprintf(delay_log, "%3d %3d\n", optimal_delay_ms, current_delay_ms);
      }
    }
  }
  std::cout << std::endl;
  if (delay_log != NULL)
    fclose(delay_log);
}
