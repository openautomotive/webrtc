import struct
import wave

class WavReadWriter:

  def __init__(self, filename_out):
    """Function docstring."""

    file_out = wave.open(filename_out, "wb")
    file_out.setnchannels(1)
    file_out.setsampwidth(2)
    file_out.setframerate(48000)

    self.file_out = file_out

  def write_sample(self, sample):
    assert -32768 <= int(sample) <= 32767
    self.file_out.writeframes(struct.pack("@h", int(sample)))
