# Copyright (c) 2017 The WebRTC project authors. All Rights Reserved.
#
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file in the root of the source
# tree. An additional intellectual property rights grant can be found
# in the file PATENTS.  All contributing project authors may
# be found in the AUTHORS file in the root of the source tree.

"""Signal processing utility module.
"""
from __future__ import division

import array
import logging
import math
import os
import sys

try:
  import numpy as np
except ImportError:
  logging.critical('Cannot import the third-party Python package numpy')
  sys.exit(1)

try:
  import pydub
  import pydub.generators
except ImportError:
  logging.critical('Cannot import the third-party Python package pydub')
  sys.exit(1)

try:
  import scipy.signal
except ImportError:
  logging.critical('Cannot import the third-party Python package scipy')
  sys.exit(1)

from . import exceptions


class SignalProcessingUtils(object):
  """Collection of signal processing utilities.
  """

  def __init__(self):
    pass

  @classmethod
  def LoadWav(cls, filepath, channels=1):
    """Loads wav file.

    Args:
      filepath: path to the wav audio track file to load.
      channels: number of channels (downmixing to mono by default).

    Returns:
      AudioSegment instance.
    """
    if not os.path.exists(filepath):
      logging.error('cannot find the <%s> audio track file', filepath)
      raise exceptions.FileNotFoundError()
    return pydub.AudioSegment.from_file(
        filepath, format='wav', channels=channels)

  @classmethod
  def SaveWav(cls, output_filepath, signal):
    """Saves wav file.

    Args:
      output_filepath: path to the wav audio track file to save.
      signal: AudioSegment instance.
    """
    return signal.export(output_filepath, format='wav')

  @classmethod
  def CountSamples(cls, signal):
    """Number of samples per channel.

    Args:
      signal: AudioSegment instance.

    Returns:
      An integer.
    """
    number_of_samples = len(signal.get_array_of_samples())
    assert signal.channels > 0
    assert number_of_samples % signal.channels == 0
    return number_of_samples / signal.channels

  @classmethod
  def GenerateSilence(cls, duration=1000, sample_rate=48000):
    """Generates silence.

    This method can also be used to create a template AudioSegment instance.
    A template can then be used with other Generate*() methods accepting an
    AudioSegment instance as argument.

    Args:
      duration: duration in ms.
      sample_rate: sample rate.

    Returns:
      AudioSegment instance.
    """
    return pydub.AudioSegment.silent(duration, sample_rate)

  @classmethod
  def GeneratePureTone(cls, duration, sample_rate, frequency=440.0):
    """Generates a pure tone.

    The pure tone is generated with the same duration and in the same format of
    the given template signal.

    Args:
      template: AudioSegment instance.
      frequency: Frequency of the pure tone in Hz.

    Return:
      AudioSegment instance.
    """

    if frequency > sample_rate >> 1:
      raise exceptions.SignalProcessingException('Invalid frequency')

    template = pydub.AudioSegment.silent(duration, sample_rate)
    assert len(template) == duration
    assert template.frame_rate == sample_rate
    assert template.channels == 1
    duration_s = duration / 1000

    num_samples = duration_s * template.frame_rate
    max_value = 2 * np.pi * frequency * duration_s
    print("max value: {}, num_samples: {}".format(max_value, num_samples))
    sin_args = np.linspace(0, max_value, num_samples)

    sin_args = np.mod(sin_args, 2*np.pi)

    sin_vals = (2**15 - 1) * np.sin(sin_args)
    sin_vals_in_array = np.array(sin_vals, dtype = 'int16')

    x = pydub.AudioSegment(
        data = sin_vals_in_array.tobytes(),
        metadata = {
            'sample_width': template.sample_width,
            'frame_rate': template.frame_rate,
            'frame_width': template.frame_width,
            'channels': template.channels,
        }
    )

    ttt = sin_vals_in_array - x.get_array_of_samples()
    print(max(ttt))
    assert(max(ttt) == 0)

    return x

    # generator = pydub.generators.Sine(
    #     sample_rate=template.frame_rate,
    #     bit_depth=template.sample_width * 8,
    #     freq=frequency)

    # return generator.to_audio_segment(
    #     duration=len(template),
    #     volume=0.0)

  @classmethod
  def GenerateWhiteNoise(cls, template):
    """Generates white noise.

    The white noise is generated with the same duration and in the same format
    of the given template signal.

    Args:
      template: AudioSegment instance.

    Return:
      AudioSegment instance.
    """
    generator = pydub.generators.WhiteNoise(
        sample_rate=template.frame_rate,
        bit_depth=template.sample_width * 8)
    return generator.to_audio_segment(
        duration=len(template),
        volume=0.0)

  @classmethod
  def AudioSegmentToRawData(cls, signal):
    samples = signal.get_array_of_samples()
    if samples.typecode != 'h':
      raise exceptions.SignalProcessingException('Unsupported samples type')
    return np.array(signal.get_array_of_samples(), np.int16)

  @classmethod
  def DetectHardClipping(cls, signal, threshold=2):
    """Detects hard clipping.

    Hard clipping is simply detected by counting samples that touch either the
    lower or upper bound too many times in a row (according to |threshold|).
    The presence of a single sequence of samples meeting such property is enough
    to label the signal as hard clipped.

    Args:
      signal: AudioSegment instance.
      threshold: minimum number of samples at full-scale in a row.

    Returns:
      True if hard clipping is detect, False otherwise.
    """
    if signal.channels != 1:
      raise NotImplementedError('mutliple-channel clipping not implemented')
    if signal.sample_width != 2:  # Note that signal.sample_width is in bytes.
      raise exceptions.SignalProcessingException(
          'hard-clipping detection only supported for 16 bit samples')
    samples = cls.AudioSegmentToRawData(signal)

    # Detect adjacent clipped samples.
    samples_type_info = np.iinfo(samples.dtype)
    mask_min = samples == samples_type_info.min
    mask_max = samples == samples_type_info.max

    def HasLongSequence(vector, min_legth=threshold):
      """Returns True if there are one or more long sequences of True flags."""
      seq_length = 0
      for b in vector:
        seq_length = seq_length + 1 if b else 0
        if seq_length >= min_legth:
          return True
      return False

    return HasLongSequence(mask_min) or HasLongSequence(mask_max)

  @classmethod
  def ApplyImpulseResponse(cls, signal, impulse_response):
    """Applies an impulse response to a signal.

    Args:
      signal: AudioSegment instance.
      impulse_response: list or numpy vector of float values.

    Returns:
      AudioSegment instance.
    """
    # Get samples.
    assert signal.channels == 1, (
        'multiple-channel recordings not supported')
    samples = signal.get_array_of_samples()

    # Convolve.
    logging.info('applying %d order impulse response to a signal lasting %d ms',
                 len(impulse_response), len(signal))
    convolved_samples = scipy.signal.fftconvolve(
        in1=samples,
        in2=impulse_response,
        mode='full').astype(np.int16)
    logging.info('convolution computed')

    # Cast.
    convolved_samples = array.array(signal.array_type, convolved_samples)

    # Verify.
    logging.debug('signal length: %d samples', len(samples))
    logging.debug('convolved signal length: %d samples', len(convolved_samples))
    assert len(convolved_samples) > len(samples)

    # Generate convolved signal AudioSegment instance.
    convolved_signal = pydub.AudioSegment(
        data=convolved_samples,
        metadata={
            'sample_width': signal.sample_width,
            'frame_rate': signal.frame_rate,
            'frame_width': signal.frame_width,
            'channels': signal.channels,
        })
    assert len(convolved_signal) > len(signal)

    return convolved_signal

  @classmethod
  def Normalize(cls, signal):
    """Normalizes a signal.

    Args:
      signal: AudioSegment instance.

    Returns:
      An AudioSegment instance.
    """
    return signal.apply_gain(-signal.max_dBFS)

  @classmethod
  def Copy(cls, signal):
    """Makes a copy os a signal.

    Args:
      signal: AudioSegment instance.

    Returns:
      An AudioSegment instance.
    """
    return pydub.AudioSegment(
        data=signal.get_array_of_samples(),
        metadata={
            'sample_width': signal.sample_width,
            'frame_rate': signal.frame_rate,
            'frame_width': signal.frame_width,
            'channels': signal.channels,
        })

  @classmethod
  def MixSignals(cls, signal, noise, target_snr=0.0, bln_pad_shortest=False):
    """Mixes two signals with a target SNR.

    Mix two signals with a desired SNR by scaling noise (noise).
    If the target SNR is +/- infinite, a copy of signal/noise is returned.

    Args:
      signal: AudioSegment instance (signal).
      noise: AudioSegment instance (noise).
      target_snr: float, numpy.Inf or -numpy.Inf (dB).
      bln_pad_shortest: if True, it pads the shortest signal with silence at the
                        end.

    Returns:
      An AudioSegment instance.
    """
    # Handle infinite target SNR.
    if target_snr == -np.Inf:
      # Return a copy of noise.
      logging.warning('SNR = -Inf, returning noise')
      return cls.Copy(noise)
    elif target_snr == np.Inf:
      # Return a copy of signal.
      logging.warning('SNR = +Inf, returning signal')
      return cls.Copy(signal)

    # Check signal and noise power.
    signal_power = float(signal.dBFS)
    noise_power = float(noise.dBFS)
    if signal_power == -np.Inf:
      logging.error('signal has -Inf power, cannot mix')
      raise exceptions.SignalProcessingException(
          'cannot mix a signal with -Inf power')
    if noise_power == -np.Inf:
      logging.error('noise has -Inf power, cannot mix')
      raise exceptions.SignalProcessingException(
          'cannot mix a signal with -Inf power')

    # Pad signal (if necessary). If noise is the shortest, the AudioSegment
    # overlay() method implictly pads noise. Hence, the only case to handle
    # is signal shorter than noise and bln_pad_shortest True.
    if bln_pad_shortest:
      signal_duration = len(signal)
      noise_duration = len(noise)
      logging.warning('mix signals with padding')
      logging.warning('  signal: %d ms', signal_duration)
      logging.warning('  noise: %d ms', noise_duration)
      padding_duration = noise_duration - signal_duration
      if padding_duration > 0:  # That is signal_duration < noise_duration.
        logging.debug('  padding: %d ms', padding_duration)
        padding = pydub.AudioSegment.silent(
            duration=padding_duration,
            frame_rate=signal.frame_rate)
        logging.debug('  signal (pre): %d ms', len(signal))
        signal = signal + padding
        logging.debug('  signal (post): %d ms', len(signal))

        # Update power.
        signal_power = float(signal.dBFS)

    # Mix signals using the target SNR.
    gain_db = signal_power - noise_power - target_snr
    return cls.Normalize(signal.overlay(noise.apply_gain(gain_db)))


  @classmethod
  def LowLevelTHD(cls, samples, f0_harmonic, rate):
    #samples = np.array(samples, dtype='float128')
    num_samples = len(samples)
    duration_seconds = num_samples / rate

    scaling = 2.0 / num_samples
    max_freq = rate >> 1

    t = np.linspace(0, duration_seconds, num_samples)

    def scalar_prod(s1, s2):
      return np.sum(s1 * s2) * scaling

    # Analyze harmonics.
    n = 1
    basis_vectors = []
    while f0_harmonic * n < max_freq:
      basis_vectors.append((np.array(
          np.sin(2.0 * math.pi * n * f0_harmonic * t)),
                            np.array(
          np.cos(2.0 * math.pi * n * f0_harmonic * t))))
      n += 1

    # Normalize:
    for i, (v_sin, v_cos) in enumerate(basis_vectors):
      v_sin =  v_sin / np.sqrt(scalar_prod(v_sin, v_sin))
      v_cos =  v_cos / np.sqrt(scalar_prod(v_cos, v_cos))
      basis_vectors[i] = (v_sin, v_cos)

    xy_terms = [(scalar_prod(samples, v_sin), scalar_prod(samples, v_cos))
                for (v_sin, v_cos) in basis_vectors
    ]
    b_terms = [np.sqrt(x_n**2 + y_n**2) for (x_n, y_n) in xy_terms]
      # x_n = scalar_prod(samples, np.array(
      #     np.sin(2.0 * math.pi * n * f0_harmonic * t)))
      # y_n = scalar_prod(samples, np.array(
      #     np.cos(2.0 * math.pi * n * f0_harmonic * t) ))
      # xy_terms.append((x_n, y_n))
      # b_terms.append(np.sqrt(x_n**2 + y_n**2))
      # n += 1

    x_1, y_1 = xy_terms[0]
    v_sin_1, v_cos_1 = basis_vectors[0]
    output_without_fundamental = samples - x_1 * v_sin_1 - y_1 * v_cos_1
    #pure_harmonic = v_1 * v_sin_1

    distortion_and_noise = np.sqrt(np.pi * scalar_prod(output_without_fundamental,
                                                  output_without_fundamental))

    # TODO(alessiob): Fix or remove if not needed.
    thd = np.sqrt(np.sum([x**2 for x in b_terms[1:]])) / b_terms[0]

    # TODO(alessiob): Check range.
    thd_plus_noise = distortion_and_noise / b_terms[0]

    noise = thd_plus_noise - thd
    # print("""
# THD = {}
# noise = {}
# b_terms = {}
# xy_terms = {}
# """.format(thd, noise, b_terms, xy_terms))
    return thd, noise, b_terms, xy_terms, basis_vectors, output_without_fundamental, distortion_and_noise, scaling


  @classmethod
  def THDAndNoise(cls, signal, f0_harmonic):
    samples = SignalProcessingUtils.AudioSegmentToRawData(
        signal)
    return SignalProcessingUtils.LowLevelTHD(samples, f0_harmonic, signal.frame_rate)[:2]

  @classmethod
  def ApplyGainWithClipping(cls, signal, gain_db):
    return signal.apply_gain(gain_db)
