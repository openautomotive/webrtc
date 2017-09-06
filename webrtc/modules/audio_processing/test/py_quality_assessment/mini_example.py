import quality_assessment.signal_processing as sp

FILE_input = "./pure_tone-1760_10000.wav"
signal_input = sp.SignalProcessingUtils.LoadWav(FILE_input)

f0 = 1760
signal_new = sp.SignalProcessingUtils.GeneratePureTone(signal_input, frequency = f0)

TMP_FILE = "temporary_save.wav"
sp.SignalProcessingUtils.SaveWav(TMP_FILE, signal_new)

signal_new_new_template = sp.SignalProcessingUtils.LoadWav(TMP_FILE)
signal_new_new = sp.SignalProcessingUtils.GeneratePureTone(signal_input, frequency = f0)

TMP_FILE_NEW = "temporary_save_new.wav"
sp.SignalProcessingUtils.SaveWav(TMP_FILE_NEW, signal_new)
