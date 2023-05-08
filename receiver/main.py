from rtlsdr import RtlSdr
import utilities
from config_reader import config

sdr = RtlSdr()

# configure device
sdr.sample_rate = config['sample_rate']
sdr.center_freq = config['freq']
sdr.freq_correction = config['ppm']
sdr.gain = config['gain']

print(sdr.read_samples(512))
