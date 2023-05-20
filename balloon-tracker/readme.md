![Banner](https://github.com/trackuino/trackuino/wiki/img/trackuino-banner-narrow.png)

The software and hardware for Trackuino, an open-source APRS tracker based on the Arduino platform. It was designed primarily to track high altitude balloons, so it has other handy features like reading temperature sensors and a buzzer for acoustic location.

Trackuino is intended for use by licensed radio amateurs.

# Features

- Arduino form factor (support for Arduino Nano, Uno, and Mega)
- Fully modular, choose the daughterboards you want to use and print/buy them and the motherboard that fits your Arduino
- Onboard and external LED headers for module status indications
- Internal/external temperature sensors to read temperature in and outside the payload
- Active/passive buzzer support to ease acoustic payload location
- Support for custom additional analog and digital sensors
- Open source (GPLv2 license), both software and hardware. In other words, do whatever you want with it: modify it, add it to your project, etc. as long as you open-source your modifications as well.

## Modules

Choose whatever modules you want to use and add them onto your motherboard.

### GPS Module

- Any GPS module that outputs NMEA strings (e.g. SiRF, UBLOX, etc.) is supported
- GPS used needs 5V, GND, RX, and TX pins

### GSM Cellular Module

- Allows sending data to a GSM network (e.g. AT&T, T-Mobile, etc.) via web requests or SMS messages
- Configuration for message format and destination phone number or URL

### Radio Module

- Radio: Radiometrix's HX1 (300 mW).
- 1200 bauds AFSK using 8-bit PWM
- Sends out standard APRS position messages (latitude, longitude, altitude, course, speed and time).
- SMA female plug for radio out

### SD Card Local Storage Module

- OpenLog SD Card writing for an on-board copy of recorded data
- Supports CSV format writing of data
- Raw data dumping option
- Custom pre-save formatting option (can convert raw analog input to temperature with given formula, etc.)

### Buzzer Module

- Active buzzer for acoustic payload location
- Supports GPS readings for variable buzzing based on altitude or satellite lock

# Setup

Use the `Download ZIP` button or [click here](https://github.com/EricAndrechek/trackuino-v2/archive/refs/heads/main.zip) to get the source code and board schematics.

## Choosing Boards

Choose the motherboard you want to use based on what Arduino/MCU form factor you plan to use. The larger the Arduino/MCU and therefore motherboard, the more modules you will be able to add.

After selecting your motherboard, you can choose the modules you want to use. You can add as many modules as you want so long as they all fit on the motherboard. One of these modules needs to be the power daughterboard.

**Important**: Pay attention to the voltage your Arduino is using. If you are unsure, check [this page](https://learn.sparkfun.com/tutorials/arduino-comparison-guide/totally-tabular) to find out. If your MCU is 3.3V, you will need to print the 3.3V power daughterboard, if it is 5V, you will need to print the 5V power daughterboard.

Select all motherboards and module daughter boards you would like to use, and find somewhere online to get them printed.

## Soldering

Each module has a readme file that contains instructions for soldering. The boards also include labels on each component detailing what to solder where.

Each board's readme also includes a table of needed components to purchase to be used on your board.

## Software

If you are building for the Arduino platform you need the Arduino IDE. Get it from the [Arduino web site](https://arduino.cc/).

Unzip the software in your sketches directory and load it up by double-clicking on trackuino.ino.

The single most important configuration file is `config.h`. The file is self-documented. Here is where you set up your callsign, module settings, and more.

Some modules, like GSM, allow you to make additional configuration changes in their own custom, self-documented files so that you can do things like specify a JSON format for data to be sent through.

## Flashing

**Important**: When flashing the Arduino, remove the it from the motherboard. After flashing the firmware, you can plug it back in. The GPS, OpenLog, and the host computer share the same serial port on the AVR, so they will conflict when used together.

Within the Arduino IDE, be sure to select the correct board type for your Arduino.

While trackuino.ino is opened in the Arduino IDE, select the `Program` tab and click the `Upload` button.

## Debugging

In order to use serial debugging, you need to enable it in the `config.h` file, upload the code to the Arduino again, and then select the `Serial Monitor` tab.
