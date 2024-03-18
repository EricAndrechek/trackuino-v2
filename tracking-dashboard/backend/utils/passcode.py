# Returns APRS-IS passcode for given callsign
# Based on https://github.com/magicbug/PHP-APRS-Passcode/blob/master/aprs_func.php

def get_passcode(callsign):
    callsign = callsign.strip().split('-')[0].upper()
    hash = 0x73e2
    for i in range(0, len(callsign), 2):
        hash ^= ord(callsign[i]) << 8
        hash ^= ord(callsign[i+1])
    return hash & 0x7fff

if __name__ == '__main__':
    import sys
    if len(sys.argv) != 2:
        print('Usage: passcode.py <callsign>')
        sys.exit(1)
    print(get_passcode(sys.argv[1]))
