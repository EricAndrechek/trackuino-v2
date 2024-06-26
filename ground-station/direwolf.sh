
# rtl_fm:
# -f 144390000: frequency of APRS channel to listen on
# -o 4: unclear, direwolf suggests

# direwolf:
# -c ~/direwolf.conf: configuration file for direwolf
# -T "": enable timestamps for send and received frames (with format)
# -r 24000: audio sample rate, per second
# -n 1: number of audio channels (1 or 2)
# -B 1200: data rate in bits per second for channel 0
# -t 0: text colors: 0 = off, 1 = on, 2,3,4... other

sudo rtl_fm -f 144.39M -o 4 - | direwolf -c direwolf.conf -t 0 -r 24000 -D 1 -dgitm -
