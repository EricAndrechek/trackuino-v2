![Banner](https://github.com/trackuino/trackuino/wiki/img/trackuino-banner-narrow.png)

The software and hardware for Trackuino-v2, an open-source APRS tracker based on the Arduino platform. It was designed primarily to track high altitude balloons, so it has other handy features like reading temperature sensors and a buzzer for acoustic location.

Trackuino is intended for use by licensed radio amateurs.

# Features

- Arduino form factor (support for Arduino Nano, others coming soon:tm:)
- Fully modular, choose the daughterboard you want to use and print/buy them and the motherboard that fits your Arduino
- Onboard and external LED headers for module status indications
- Internal/external temperature sensors to read temperature in and outside the payload
- Active/passive buzzer support to ease acoustic payload location
- Support for custom additional analog and digital sensors
- Open source (GPLv2 license), both software and hardware. In other words, do whatever you want with it: modify it, add it to your project, etc. as long as you open-source your modifications as well.

## Built-In Modules

The following modules are included on the motherboard:

### GPS Module

- UBLOX MAX-M10S GPS chip

### MicroSD Card Local Storage

- MicroSD card writing for an on-board copy of recorded data
- Supports CSV format writing of data
- Raw data dumping option
- Custom pre-save formatting option (can convert raw analog input to temperature with given formula, etc.)

### LoRa Radio Module

- Radio: RFM95W
- 915MHz frequency (tuneable to others for different regions)
- 20dBm output power
- Used to send data in addition to daughterboard modules, allows 2-way communication

### Flight Termination Unit (FTU) Transmitter Module

- Modified [315MHz transmitter](https://www.adafruit.com/product/1095) for sending digital FTU signals

### Buzzer

- Active buzzer for acoustic payload location
- Supports GPS readings for variable buzzing based on altitude or satellite lock

### Temperature Sensors

- Internal and external temperature sensors
- TMP36 analog temperature sensor

### LED Indicators

- Onboard and external LED headers for module status indications
- Customizable LED blinking patterns for different module statuses

## Daughterboard Modules

In addition to the main motherboard, you can attach any one of the following daughterboards to the motherboard via the molex connector for additional functionality.

### GSM Cellular Module

**Note**: Work in progress

- Allows sending data to a GSM network (e.g. AT&T, T-Mobile, etc.) via web requests or SMS messages
- Configuration for message format and destination phone number or URL

### HAM Radio APRS Module

- Radio: DORJI DRA818V
- 1200 baud AFSK using 8-bit PWM
- Sends out standard APRS position messages (latitude, longitude, altitude, course, speed and time), and telemetry data
- SMA plug for radio out

### Iridium Satellite Module

- RockBLOCK 9603 Iridium satellite modem

# Setup

Use the `Download ZIP` button or [click here](https://github.com/EricAndrechek/trackuino-v2/archive/refs/heads/main.zip) to get the source code and board schematics. The hardware is designed in KiCad and the software is written in C++ for the Arduino platform.

Navigate to the `balloon-tracker/hardware` directory to find the KiCad files for the motherboard and daughterboard modules. Navigate to the `balloon-tracker/software` directory to find the Arduino sketch for the firmware.

## Choosing Boards

Choose the motherboard you want to use based on what Arduino/MCU form factor you plan to use.

After selecting your motherboard, you can choose the daughterboard you want to use.

Select all motherboards and daughterboards you would like to use, and find somewhere online to get them printed. You will need to open the KiCad files in KiCad and export the gerber files (and drill files) to send to a PCB printing service.

## Soldering

Each module has a readme file that contains instructions for soldering. The boards also include labels on each component detailing what to solder where.

Each board's readme also includes a table of needed components to purchase to be used on your board.

## Software

If you are building for the Arduino platform you need the Arduino IDE. Get it from the [Arduino web site](https://arduino.cc/).

Unzip the software in your sketches directory and load it up by double-clicking on trackuino.ino.

The single most important configuration file is `config.h`. The file is self-documented. Here is where you set up your callsign, module settings, and more.

Some daughterboards, like GSM, allow you to make additional configuration changes in their own custom, self-documented files so that you can do things like specify a JSON format for data to be sent through.

## Flashing

**Important**: When flashing the Arduino, remove the it from the motherboard. After flashing the firmware, you can plug it back in. The GPS, microSD card module, and the host computer share the same serial port on the AVR, so they will conflict when used together.

Within the Arduino IDE, be sure to select the correct board type for your Arduino.

While `software.ino` is opened in the Arduino IDE, select the `Program` tab and click the `Upload` button.

## Debugging

In order to use serial debugging, you need to enable it in the `config.h` file, upload the code to the Arduino again, and then select the `Serial Monitor` tab.
