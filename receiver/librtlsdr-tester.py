from ctypes import *
from ctypes.util import find_library
import os

print("LD_LIBRARY_PATH:")
print(os.environ['LD_LIBRARY_PATH'])

print()

print("Library locations:")
print(find_library('rtlsdr'))
print(find_library('librtlsdr'))
print(find_library('rtl-sdr'))
print(find_library('rtl_sdr'))
