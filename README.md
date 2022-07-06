![Banner](https://github.com/trackuino/trackuino/wiki/img/trackuino-banner-narrow.png)

This is the firmware for Trackuino, an open-source APRS tracker based on the Arduino platform. It was designed primarily to track high altitude balloons, so it has other handy features like reading temperature sensors and a buzzer for acoustic location.

Trackuino is intended for use by licensed radio amateurs.

# Features

-   Arduino shield form factor (you can stack more shields on it)
-   GPS: Any GPS module that outputs NMEA strings (e.g. SiRF, UBLOX, etc.), just needs 5V, GND, RX, and TX pins
-   Radio: Radiometrix's HX1 (300 mW).
-   1200 bauds AFSK using 8-bit PWM
-   Sends out standard APRS position messages (latitude, longitude, altitude, course, speed and time).
-   Internal/external temperature sensors to read temperature in and outside the payload
-   Active/passive buzzer support to ease acoustic payload location.
-   2 x SMA female plugs (1 x GPS in + 1 x radio out)
-   Onboard and external pin headers for status LEDs
-   Support for custom additional analog and digital sensors
-   Hardware schematics for a variety of Arduino footprints
-   OpenLog SD Card writing for an on-board copy of recorded data
-   Open source (GPLv2 license), both software and hardware. In other words, do whatever you want with it: modify it, add it to your project, etc. as long as you open-source your modifications as well.

# Download

Use the `Download ZIP` button or [click here](https://github.com/EricAndrechek/trackduino-v2/zipball/master) to get the source code.

# Building

If you are building for the Arduino platform you need the Arduino IDE. Get it from the [Arduino web site](http://arduino.cc/).

Unzip the software in your sketches directory and load it up by double-clicking on trackuino.ino.

The single most important configuration file is "config.h". The file is self-documented. Here is where you set up your callsign, among other things.

# Flashing

**Important**: When flashing the Arduino, remove the Trackuino shield. After flashing the firmware, you can plug it back in. The GPS, OpenLog, and the host computer share the same serial port on the AVR, so they will conflict when used together.

# Hardware

The `hardware` folder contains the Eagle schematic / pcb files of a shield you can build as-is (gerber files are included) or modify to suit your needs. Check its README for details.
