from __future__ import division
import array
import quality_assessment.signal_processing as sp
import matplotlib.pyplot as plt
import pydub
import numpy as np
import math
import itertools

import wav_read_writer

FILE_output = "output/apmcfg-attack-0__decay-0__frames-0__gain--5/capture-pure_tone-1760_1000/render-(none)/echosim-noecho/datagen-identity/datagen_params-default/output.wav"

FILE_input = "pure_tone-1760_1000.wav"
signal_input = sp.SignalProcessingUtils.LoadWav(FILE_input)
signal_output = sp.SignalProcessingUtils.LoadWav(FILE_output)

duration = len(signal_output) / 1000.0
samples_input = np.array(signal_input.get_array_of_samples(), dtype = 'int16')
samples_output = np.array(signal_output.get_array_of_samples(), dtype = 'int16')
num_samples = len(samples_output)
t = np.linspace(0, duration, num_samples)
f0 = 1760
scaling = 2 / num_samples

print("Num_samples in input = ", len(samples_input))
print("Num_samples in output = ", num_samples)


def checkTHD(samples):
  thd, noise, b_terms, xy_terms, _, output_without_fundamental, _, _ = sp.SignalProcessingUtils.LowLevelTHD(samples, f0, 48000)
  print("""
THD = {}
noise = {}
b_terms = {}
xy_terms = {}
""".format(thd, noise, b_terms, xy_terms))
  return output_without_fundamental

def scalar_prod(s1, s2):
  return np.sum(s1 * s2) * scaling

def save(samples, filename):
  wrw = wav_read_writer.WavReadWriter(filename)
  for sample in samples:
    wrw.write_sample(sample)

# def parse_write(samples):
#   save(samples, "samples.wav")
#   signal = sp.SignalProcessingUtils.LoadWav("samples.wav")
#   return signal.get_array_of_samples()

def parse_writeS(signal):
  sp.SignalProcessingUtils.SaveWav("samples.wav", signal)
  # save(samples, "samples.wav")
  signal = sp.SignalProcessingUtils.LoadWav("samples.wav")
  return np.array(signal.get_array_of_samples(), dtype = 'int16')


def plot(inp, outp, difference):
  I0, I1 = 0, len(inp)
  T = np.arange(I0, I1) / float(48000)

  # Compute spectra.
  S_inp = np.abs(np.fft.rfft(inp[I0:I1]))
  S_outp = np.abs(np.fft.rfft(outp[I0:I1]))
  S_diff = np.abs(np.fft.rfft(difference[I0:I1]))

  S_f = np.fft.rfftfreq(T.shape[-1])
  plt.figure(figsize=(12, 9))

  # ax = plt.subplot(3, 1, 3)
  plt.title('input (blue), output (red) and \'noise\' (green)')
  plt.plot(S_f, S_inp, 'b')
  plt.plot(S_f, S_outp, 'r')
  plt.plot(S_f, S_diff, 'g')
  plt.yscale('log')
  plt.axis([0.0, 0.5, 0.0, max(S_inp)])

  plt.show()

def plot_one(x):
  T = np.arange(0, len(x)) / float(48000)

  # Compute spectra.
  S = np.abs(np.fft.rfft(x))

  S_f = np.fft.rfftfreq(T.shape[-1])
  plt.figure(figsize=(12, 9))

  plt.plot(S_f, S)

  plt.yscale('log')
  plt.axis([0.0, 0.5, 0.0, max(S)])

  plt.show()


def check_some_diffs():

  generated = sp.SignalProcessingUtils.GeneratePureToneArray(duration*1000, 48000, frequency = 1760)

  real_pure_input = sp.SignalProcessingUtils.GeneratePureToneArray(duration*1000, 48000, frequency = 1760)

  # diff = generated - real_pure_input
  # print("Diff (should be 0): ", diff)

  # diff = real_pure_input - parse_write(generated)
  # print("Diff (should be 0): ", diff)

  opened = parse_writeS(sp.SignalProcessingUtils.GeneratePureTone(duration*1000, 48000, frequency = 1760))
  print("Opened has length: ", len(opened))
  print("Real pure has length: ", len(real_pure_input))
  print("samples_input has length: ", len(samples_input))

  diff = real_pure_input - opened
  print("Diff (should be 0): ", diff)

  diff = samples_input - opened
  print("Diff (should be 0): ", diff)

  real_pure_output = real_pure_input * np.power(10, -5 / 20.0)


  # plot_one(real_pure_input)
  # plot_one(real_pure_output)
  owf = checkTHD(real_pure_output)

  # plot_one(owf)
  # plot(real_pure_input, real_pure_output, owf)
  owf = checkTHD(samples_output)

  diff_segment = pydub.AudioSegment(
      data=np.array(owf, dtype='int16').tobytes(),
      metadata = {
          'sample_width': 2,
          'frame_rate': 48000,
          'frame_width': 2,
          'channels': 1,
      }
  )

  diff_segment.export("output_minus_fundamental.wav", format='wav')

  # plot(samples_input, samples_output, owf)

check_some_diffs()


# plt.plot(samples_input - real_pure_input)

# plot_one(samples_input)
# plot_one(real_pure_input)
