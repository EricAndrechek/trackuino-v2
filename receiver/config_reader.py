import json
import utilities

def parseGain(gain):
    if gain == 'auto':
        return gain
    else:
        return float(gain)

raw_config = json.load(open('config.json'))

config = {
    'freq': utilities.MHz_to_KHz(raw_config['APRS Frequency (MHz)']),
    'sample_rate': raw_config['Sample Rate (Msps)'] * 1000000,
    'gain': parseGain(raw_config['Gain (dB)']),
    'ppm': raw_config['Frequency Correction (ppm)'],
}
