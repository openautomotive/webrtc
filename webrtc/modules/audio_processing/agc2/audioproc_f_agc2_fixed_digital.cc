#include <array>
#include <numeric>

#include "base/commandlineflags.h"
#include "base/init_google.h"
#include "base/logging.h"
#include "fixed_gain_controller.h"
#include "gain_curve_applier.h"
#include "limiter.h"
#include "vector_float_frame.h"
#include "webrtc/common_audio/wav_file.h"
#include "webrtc/modules/audio_processing/logging/apm_data_dumper.h"
#include "webrtc/rtc_base/checks.h"

namespace {

constexpr float kAudioFrameSizeSec = 0.01f;

void SimulateFixedDigitaGainController(webrtc::WavWriter* wav_writer,
                                       webrtc::WavReader* wav_reader,
                                       webrtc::FixedGainController* fixed_gc) {
  DCHECK_EQ(1, wav_reader->num_channels())
      << "Only mono Wav files are supported";

  const size_t audio_frame_size =
      kAudioFrameSizeSec * wav_reader->sample_rate();
  webrtc::VectorFloatFrame vector_float_frame(1, audio_frame_size, 0.f);
  auto float_frame = *vector_float_frame.float_frame();

  while (true) {
    const size_t num_read_samples = wav_reader->ReadSamples(
        audio_frame_size, float_frame.GetChannel(0).data());
    if (num_read_samples == 0) break;  // EOF.
    fixed_gc->Process(float_frame);
    wav_writer->WriteSamples(float_frame.GetChannel(0).data(),
                             num_read_samples);
  }
}

const char kUsageDescription[] =
    "Command-line tool to mimic the WebRTC audioproc_f tool only using the "
    "AGC2 fixed digital gain controller.";

// "audioproc_f" parameters.
DEFINE_FLAG(string, i, "", "Input wav file path");
DEFINE_FLAG(string, o, "", "Output wav file path");

// Fixed digital GC public parameters.
DEFINE_FLAG(double, agc2_fd_g, 30.f, "Gain to apply (dB)");
DEFINE_FLAG(bool, agc2_fd_with_lm, true, "Use the limiter");

// Level estimator parameters.
DEFINE_FLAG(
    int, agc2_fd_le_n, 10,
    "Number of sub-frames (level estimator), 0 for per-sample operation");
DEFINE_FLAG(double, agc2_fd_le_a, 0.1f, "Level estimator attack (ms)");
DEFINE_FLAG(double, agc2_fd_le_d, 1.f, "Level estimator decay (ms)");

}  // namespace

int main(int argc, char** argv) {
  InitGoogle(kUsageDescription, &argc, &argv, true);

  // TODO(alessiob): Replace 0 with rtc::AtomicOps::Increment() if needed.
  webrtc::WavReader wav_reader(base::GetFlag(FLAGS_i));
  webrtc::WavWriter wav_writer(base::GetFlag(FLAGS_o), wav_reader.sample_rate(),
                               wav_reader.num_channels());

  if (base::GetFlag(FLAGS_agc2_fd_with_lm)) {
    const int num_subframes_flag = base::GetFlag(FLAGS_agc2_fd_le_n);
    const int num_subframes =
        num_subframes_flag > 0
            ? num_subframes_flag
            : rtc::CheckedDivExact(wav_reader.sample_rate(), 100);
    webrtc::FixedGainController fixed_gc(
        static_cast<float>(base::GetFlag(FLAGS_agc2_fd_g)), num_subframes,
        base::GetFlag(FLAGS_agc2_fd_le_a), base::GetFlag(FLAGS_agc2_fd_le_d),
        wav_reader.sample_rate());
    SimulateFixedDigitaGainController(&wav_writer, &wav_reader, &fixed_gc);
  } else {
    webrtc::FixedGainController fixed_gc(static_cast<float>(base::GetFlag(FLAGS_agc2_fd_g)));
    SimulateFixedDigitaGainController(&wav_writer, &wav_reader, &fixed_gc);
  }

  return 0;
}
