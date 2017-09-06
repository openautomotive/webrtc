from __future__ import division
import array
import quality_assessment.signal_processing as sp
import matplotlib.pyplot as plt
import pydub
import numpy as np
import math
import itertools

DURATION_S = 20
SAMPLE_RATE = 48000
FREQ = 1760

def sp_sine():
  return sp.SignalProcessingUtils.GeneratePureToneArray(DURATION_S*1000, SAMPLE_RATE, frequency=FREQ)

def sp_sine_pydub():
  return sp.SignalProcessingUtils.GeneratePureTone(
      DURATION_S*1000, SAMPLE_RATE, frequency=FREQ
  ).get_array_of_samples()

def math_sine():
  return [(2**15-1)*math.sin(2*math.pi * x / SAMPLE_RATE * FREQ) for x in range(DURATION_S * SAMPLE_RATE)]

def numpy_sine():
  return (2**15-1)*np.sin(
      np.linspace(0, 2*np.pi * DURATION_S * FREQ, DURATION_S * SAMPLE_RATE)
  )

def numpy_mod_sine():
  # Reduce large exact numbers mod 2*pi before multiplying.
  args = FREQ*np.linspace(0, DURATION_S * SAMPLE_RATE, DURATION_S * SAMPLE_RATE, endpoint=False, dtype='int32')
  args_mod = np.mod(args, SAMPLE_RATE)
  arg_times = args_mod * 2*np.pi / SAMPLE_RATE

  return (2**15-1)*np.sin(arg_times)


x = np.array((2**15-1)*(np.array(numpy_mod_sine(), dtype='int16') - numpy_mod_sine()), dtype='int16')
pydub.AudioSegment(data=x.tobytes(),
                   metadata={
                       'sample_width': 2,
                       'frame_rate': 48000,
                       'frame_width': 2,
                       'channels': 1,
                   }
).export("quantization_noise.wav", format='wav')

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

if True:
  plot_one(sp_sine())
  plot_one(sp_sine_pydub())
  plot_one(math_sine())
  plot_one(numpy_sine())
  plot_one(numpy_mod_sine())
  plot_one(np.array(numpy_mod_sine(), dtype='int16'))
