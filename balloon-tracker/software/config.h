#ifndef __CONFIG_H__
#define __CONFIG_H__

// THIS IS THE TRACKUINO FIRMWARE CONFIGURATION FILE. YOUR CALLSIGN AND
// OTHER SETTINGS GO HERE.
//
// NOTE: all pins are Arduino based, not the Atmega chip. Mapping:
// https://www.arduino.cc/en/Hacking/PinMapping

// Config file variables. Do not touch.
#include "variables.h"
// end do not touch

// --------------------------------------------------------------------------
// GENERAL CONFIGURATION
// --------------------------------------------------------------------------

// Trackuino Config

// How frequently should data be collected, stored, and transmitted?
#define DATA_TIMEOUT 15 // seconds

// The offset in seconds from the top of the minute that messages are
// transmitted/stored. This is primarily for radio messages so that they
// do not step on each other and cause interference.
//
// For multiple Trackuinos all using radios, give them all different offsets
// of equal spacing.
//
// Set to -1 to disable. Not including GPS will also disable this feature.
#define LOG_SLOT -1 // seconds

// Sensors Config (sensors.cpp)
// m and b values for the temperature sensor calibration
#define TEMP_SLOPE 0
#define TEMP_OFFSET 0


// --------------------------------------------------------------------------
// MODULE CONFIGURATION
// --------------------------------------------------------------------------

// Module Config

// Comment out a line to disable that module.

#define BUZZER_MODULE
#define GPS_MODULE
#define SD_MODULE
// specify whether data will be transmitted via RADIO_MODULE or SATELLITE_MODULE
#define TRANSMITTER RADIO_MODULE

// Be sure to fill out the configuration for each module you are using.


// --------------------------------------------------------------------------
// APRS MODULE CONFIGURATION
// --------------------------------------------------------------------------

// APRS Config (aprs.cpp)

// Replace with your callsign
#define S_CALLSIGN "MYCALL"
// Replace with your ssid - useful for tracking multiple balloons with the same
// callsign. 11 is the standard for high altitude balloons, but any number 0-15
// is valid.
#define S_SSID 11

// Destination callsign: controls symbol
#define D_CALLSIGN "GPS"
// GPS allows for symbol, table, and overlay

// Destination callsign SSID: controls digipeating
#define D_SSID 2
// 2: Wide2-2 - good for most situations

// set the symbol you want the aprs.fi map to represent you as:
// http://www.aprs.org/symbols/symbolsX.txt
// default is 'O' for balloon
#define APRS_SYMBOL 'O'
#define APRS_TABLE '/'
#define APRS_OVERLAY ' ' // A-Z, 0-9

// APRS comment: this goes in the comment portion of the APRS message. You
// might want to keep this short. The longer the packet, the more vulnerable
// it is to noise.
#define APRS_COMMENT "Balloon Tracker v0.1"

// --------------------------------------------------------------------------
// BUZZER MODULE CONFIGURATION
// --------------------------------------------------------------------------

// Buzzer Config (buzzer.cpp)

// The buzzer status pulsing follows a simplified pattern from the status LEDs.
// It is outlined below:
// 1 buzz   -   on
// 2 buzzes -   GPS lock
// 3 buzzes -   sending data (doesn't actually buzz during transmission
//              as it would pull too much current, rather right before)
// 4 buzzes -   error - check LED for more info

// Buzz duration in milliseconds
#define BUZZER_DURATION 200

// Time between buzzes in milliseconds
#define BUZZER_DELAY 100

// Time between sets of buzzes in milliseconds
#define BUZZER_INTERVAL 5000

// For example, if you had GPS lock, your buzzer would:
// buzz for BUZZER_DURATION, wait BUZZER_DELAY, buzz for BUZZER_DURATION, 
// wait BUZZER_INTERVAL, and repeat.

// Type of buzzer. An active buzzer is driven by a
// DC voltage. A passive buzzer needs a PWM signal.
#define BUZZER_TYPE ACTIVE

// When using a passive buzzer, specify the PWM frequency here. Choose one
// that maximizes the volume according to the buzzer's datasheet. Not all
// the frequencies are valid, check out the buzzer_*.cpp code. On Arduino,
// it must be between L and 65535, where L = F_CPU / 65535 and F_CPU is the
// clock rate in hertzs. For 16 MHz Arduinos, this gives a lower limit of
// 245 Hz.
#define BUZZER_FREQ 895  // Hz

// This option disables the buzzer above BUZZER_ALTITUDE meters. This is a
// float value, so make it really high (eg. 1000000.0 = 1 million meters)
// if you want it to never stop buzzing.
#define BUZZER_ALTITUDE 3000.0  // meters (1 ft = 0.3048 m)

// --------------------------------------------------------------------------
// GPS MODULE CONFIGURATION
// --------------------------------------------------------------------------

// GPS Config (gps.cpp)

// GPS baud rate (in bits per second).
#define GPS_BAUDRATE 9600 // bps

// --------------------------------------------------------------------------
// RADIO MODULE CONFIGURATION
// --------------------------------------------------------------------------

// Radio Config (radio.cpp)

// set power to HIGH (1W) or LOW (0.5W)
#define POWER HIGH

// Forward error correction (FEC) mode:
// #define FEC_ENABLED // uncomment to enable FEC

// FEC adds additional data to the packet to allow the receiver to correct
// errors in the packet. This is useful for long range transmissions where
// the signal is weak and/or there is a lot of noise. It is not ideal for
// situations where there is significant traffic on the frequency, as it
// will increase the packet length and thus the time the transmitter is
// transmitting. This will increase the chance of stepping on other
// transmissions and causing interference.
// Additionally, it may increase battery draw as the math required is
// computationally expensive.

// --------------------------------------------------------------------------
// SATELLITE MODULE CONFIGURATION
// --------------------------------------------------------------------------

// Satellite Config (satellite.cpp)

// TODO: add Satellite module configuration


// --------------------------------------------------------------------------
// SD MODULE CONFIGURATION
// --------------------------------------------------------------------------

// SD Config (sd.cpp)

#define SD_BAUDRATE 9600 // bps

// --------------------------------------------------------------------------
// LORA CONFIGURATION
// --------------------------------------------------------------------------

// LoRa Config (lora.cpp)

// LoRa frequency in MHz
#define LORA_FREQ 434.0 // MHz

// --------------------------------------------------------------------------
// LOGGING CONFIGURATION
// --------------------------------------------------------------------------

// Whether or not to enable logging
// Disabling logging may significantly improve performance, but can make it
// more difficult to debug issues.
// #define DISABLE_LOGGING // uncomment to disable logging

// Specify the log level:
// * 0 - LOG_LEVEL_SILENT     no output 
// * 1 - LOG_LEVEL_FATAL      fatal errors 
// * 2 - LOG_LEVEL_ERROR      all errors  
// * 3 - LOG_LEVEL_WARNING    errors, and warnings 
// * 4 - LOG_LEVEL_NOTICE     errors, warnings and notices 
// * 5 - LOG_LEVEL_TRACE      errors, warnings, notices & traces 
// * 6 - LOG_LEVEL_VERBOSE    all 
#define LOG_LEVEL 0

// specify whether to write logs to Serial (USB) or SD card
// Note: Do not use Serial when the Molex connector is plugged in
// (ie unplug the transmitter daughterboard)
#define LOG_OUTPUT SD // SERIAL or SD

// --------------------------------------------------------------------------
// PIN CONFIGURATION OVERRIDES
// --------------------------------------------------------------------------

// change the default pin assignments for each module here
// the defaults are what are used in the schematic and PCB

// digital buzzer output pin
// needs to be pwm for passive buzzer
#define BUZZER_PIN 2

// SPI pins
// the below are the defaults for the Arduino Nano

// SCK (clock) pin
#define SCK_PIN 13
// MISO/CIPO pin
#define MISO_PIN 12
// MOSI/COPI pin
#define MOSI_PIN 11

// SD chip select pin
#define SD_CS_PIN 10

// LoRa chip select pin
#define LORA_CS_PIN 9

#if TRANSMITTER == RADIO_MODULE
  #define PTT_PIN 3
  #define PD_PIN 4
  #define MIC_PIN 6
  #define RX_PIN 7
  #define TX_PIN 8
#elif TRANSMITTER == SATELLITE_MODULE
  #define PTT_PIN 3
  #define PD_PIN 4
  #define MIC_PIN 6
  #define RX_PIN 7
  #define TX_PIN 8
#endif

#endif