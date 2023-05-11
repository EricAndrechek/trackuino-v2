# Software Walkthrough

**Important:** Please read the general README file for general information about how to setup your Trackuino.

This file exists as a technical walk-through and documentation of the software.

## Quick Info

### APRS

APRS, or Automatic Packet Reporting System, is an amateur radio-based system to send live data over radio to receiver stations, that can then upload the data to the internet.

APRS sends data out in a broadcast fashion, with no indication on a successful transmission from receivers.

Digipeaters, or packet receivers, receive your APRS broadcast and forward them on to more Digipeaters, modifying the package to indicate they have been forwarded an additional time.

The number of repetitions is set by the path settings of the APRS packet. Read more about what these settings should be [here](http://aprs.org/fix14439.html).

Eventually these packages reach an IGate, an APRS receiver connected to the internet.

### AFSK

AFSK, or Audio Frequency-Shift Keying, is a method of converting binary digital data into frequency or pitch in an audio tone. This allows it to be transmitted over a radio or telephone.

Bell 202 AFSK uses a 1200 Hz tone for mark (typically a binary 1) and 2200 Hz for space (typically a binary 0) at 1200bps. This is what is most commonly used in APRS.

### AX.25

AX.25 is a data link layer protocol used to send data over a radio. It specifies specifically how the APRS data is bundled before being sent over the radio with AFSK.

## Suggested Readings

These are some basic articles to help you understand the core concepts of these protocols.

- [Audio Frequency-Shift Keying (AFSK)](https://en.wikipedia.org/wiki/Frequency-shift_keying#Audio_frequency-shift_keying)
- [Bell 202 Modem - AFSK at 1200bps](https://en.wikipedia.org/wiki/Bell_202_modem)
- [Automatic Packet Reporting System (APRS)](https://en.wikipedia.org/wiki/Automatic_Packet_Reporting_System)
- [SparkFun HX1 Radio Tutorial](https://learn.sparkfun.com/tutorials/hx1-aprs-transmitter-hookup-guide/all#hardware-overview)

## In-Depth Readings

To fully understand the implementation of these protocols, you should read the following specs:

- [AX.25 Specifications](https://www.tapr.org/pdf/AX25.2.2.pdf)
- [APRS Specifications](http://www.aprs.org/doc/APRS101.PDF)

## Additional Notes

The current implementation of the telemetry data within the APRS protocol is non-standard. The telemetry data should be included as outlined [here](https://github.com/PhirePhly/aprs_notes/blob/master/telemetry_format.md).

The "official" telemetry implementation, however, has some restrictions, and so the popular website [aprs.fi](https://aprs.fi/)'s [own implementation](https://aprs.fi/doc/guide/aprsfi-telemetry.html) is used. Read about it and how it adapts the APRS specification's chapter 13: Telemetry Data [here](http://he.fi/doc/aprs-base91-comment-telemetry.txt).

This means that our current implementation is not universally supported, however we found that [aprs.fi](https://aprs.fi/) has enough support that this custom implementation still works well enough.

## Files

AFSK:

`afsk_avr.cpp` - AFSK implementation for AVR microcontrollers
`afsk_pic32.cpp` - AFSK implementation for PIC32 microcontrollers
Their respective header files will only export their functions (which accomplish the same things in both files, just are architecture/compiler specific) if the compiler detects that that is its target architecture.

`ax25.cpp` - AX.25 implementation
