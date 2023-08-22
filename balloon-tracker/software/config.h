#ifndef __CONFIG_H__
#define __CONFIG_H__

// THIS IS THE HAB TRACKER FIRMWARE CONFIGURATION FILE. YOUR CALLSIGN AND
// OTHER SETTINGS GO HERE.

// --------------------------------------------------------------------------
// GENERAL CONFIGURATION
// --------------------------------------------------------------------------

// Tracker Config

// How frequently should data be transmitted via radio/satellite?
// SD data logging and LoRa transmissions happen at their own intervals.
#define DATA_TIMEOUT 60 // seconds

// The offset in seconds from the top of the minute that messages are
// transmitted. This is primarily for radio messages so that they
// do not step on each other and cause interference.
// For multiple trackers all using radios, give them all different offsets
// of equal spacing.
#define DATA_SLOT 0 // seconds (0-59) offset from top of minute
// If you do not plan on using this, set it to a random number between 0 and 59.

// number of seconds of error it can be off from slot to broadcast
// should be slightly longer than the time it takes for a full loop
#define SLOT_ERROR 5 // seconds

// Whether to broadcast/save data in compressed APRS format or uncompressed.
// Compressed will use more CPU time and memory, but will save bandwidth.
// Uncompressed will use less CPU time and memory, but will use more bandwidth.
#define COMPRESSED true // true or false
// a raw APRS message body is max 70 characters uncompressed and 57 compressed

// --------------------------------------------------------------------------
// MODULE CONFIGURATION
// --------------------------------------------------------------------------

// Module Config

// specify whether data will be transmitted via RADIO_MODULE (0) or SATELLITE_MODULE (1)
#define TRANSMITTER 0

// Be sure to fill out the configuration for each module you are using.

// --------------------------------------------------------------------------
// SENSORS CONFIGURATION
// --------------------------------------------------------------------------

// Sensor Config (sensors.cpp)
// YOU NEED TO CALIBRATE THESE VALUES, this is the default for the TMP36
#define TEMP_SLOPE 100
#define TEMP_OFFSET -0.5
// this does (temp - 0.5) * 100

// resistor values for the voltage divider calibration
#define VOLTAGE_R1 0
#define VOLTAGE_R2 0
// R1 and R2 labelled as shown in this calculator's diagram: 
// https://ohmslawcalculator.com/voltage-divider-calculator

// --------------------------------------------------------------------------
// APRS MODULE CONFIGURATION
// --------------------------------------------------------------------------

// APRS Config (aprs.cpp)

// Replace with your callsign
#define S_CALLSIGN "KD8CJT"
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

// APRS comment: this goes in the comment portion of the APRS message
// this must be 19 characters or less due to APRS message length limits
// this limit is NOT checked in code - you must check it yourself or
// bad and unpredictable things will happen
#define APRS_COMMENT "UM HAB Tracker v0.1" // 19 characters or less

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
#define BUZZER_TYPE 0 // ACTIVE_BUZZER (0) or PASSIVE_BUZZER (1)

// When using a passive buzzer, specify the PWM frequency here. Choose one
// that maximizes the volume according to the buzzer's datasheet. Not all
// the frequencies are valid, check out the buzzer_*.cpp code. On Arduino,
// it must be between L and 65535, where L = F_CPU / 65535 and F_CPU is the
// clock rate in hertzs. For 16 MHz Arduinos, this gives a lower limit of
// 245 Hz.
#define BUZZER_FREQ 895  // Hz

// This option disables the buzzer above BUZZER_ALTITUDE meters. This is a
// float value, so make it really high (eg. 1000000 = 1 million meters)
// if you want it to never stop buzzing.
#define BUZZER_ALTITUDE 3000  // meters (1 ft = 0.3048 m)

// --------------------------------------------------------------------------
// GPS MODULE CONFIGURATION
// --------------------------------------------------------------------------

// GPS Config (gps.cpp)

// GPS baud rate (in bits per second).
#define GPS_BAUDRATE 9600 // bps

// GPS invalidation time (in milliseconds).
// how long to wait before marking old GPS fix data as stale
#define GPS_STALE_AGE 5000 // milliseconds
// ie if no new data is received in this GPS_STALE_AGE since the last good
// fix, the data becomes stale and will be invalidated and not used

// --------------------------------------------------------------------------
// RADIO MODULE CONFIGURATION
// --------------------------------------------------------------------------

// Radio Config (radio.cpp)

// Radio baud rate (in bits per second).
#define RADIO_BAUDRATE 9600 // bps

// Radio transmit power in dBm
// This is configured with the switch on the radio daughterboard
// Off = 0.5 W
// On = 1 W

// Radio frequency in MHz
#define RADIO_FREQ 144.390 // MHz

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

// Satellite baud rate (in bits per second).
#define SATELLITE_BAUDRATE 19200 // bps


// --------------------------------------------------------------------------
// SD MODULE CONFIGURATION
// --------------------------------------------------------------------------

// SD Config (sd.cpp)

#define SD_BAUDRATE 9600 // bps

// how often to log data to the SD card
#define LOG_INTERVAL 5000 // milliseconds

// --------------------------------------------------------------------------
// LORA CONFIGURATION
// --------------------------------------------------------------------------

// LoRa Config (lora.cpp)

// LoRa frequency in Hz
#define LORA_FREQ 915E6 // Hz (E6 = 10^6)

// Seconds between transmissions
#define LORA_INTERVAL 30 // seconds

// Offset in seconds from the top of the minute
// Set to -1 to disable. Not including GPS will also disable this feature.
#define LORA_SLOT 0 // seconds (0-59) offset from top of minute
// for example, a LORA_INTERVAL of 15 and LORA_SLOT of 7 would mean
// transmission would occur at 0:07, 0:22, 0:37, 0:52, 1:07, etc.

// LoRa altitude cutoff in meters
// transmissions will stop above this altitude
#define LORA_ALTITUDE 3000.0 // meters (1 ft = 0.3048 m)
// set to a really high number to (effectively) disable

// LoRa transmit power in dBm
#define LORA_TX_POWER 17 // dBm (5-20)

// While setting the following parameters (if changing them from the defaults),
// it is advised that you test them in this calculator:
// https://www.thethingsnetwork.org/airtime-calculator
// to ensure that your transmissions will not exceed the duty cycle:
// https://www.thethingsnetwork.org/docs/lorawan/duty-cycle/
// and to ensure your configuration is legal in your country:
// https://www.thethingsnetwork.org/docs/lorawan/frequency-plans/

// LoRa spreading factor
#define LORA_SPREADING_FACTOR 7 // 7-12
// https://development.libelium.com/lora_networking_guide/transmission-modes#spreading-factor

// LoRa bandwidth
#define LORA_BANDWIDTH 125E3 // Hz
// Supported values are 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3,
// 41.7E3, 62.5E3, 125E3, 250E3, and 500E3
// https://development.libelium.com/lora_networking_guide/transmission-modes#bandwidth

// LoRa coding rate denominator
#define LORA_CODING_RATE 5 // 5-8 (4/5 - 4/8)
// https://development.libelium.com/lora_networking_guide/transmission-modes#coding-rate


// --------------------------------------------------------------------------
// DEBUG CONFIGURATION
// --------------------------------------------------------------------------

// Whether or not to enable debug logging
// Disabling logging may significantly improve performance but can be
// useful for debugging.

// To use debug logging, you MUST have the transmitter daughterboard
// disconnected. Otherwise, the debug messages will interfere with the
// radio/satellite transmissions and cause errors.
#define DEBUG // uncomment to enable debug logging

#define SERIAL_BAUDRATE 9600 // bps

// --------------------------------------------------------------------------
// PIN CONFIGURATION OVERRIDES
// --------------------------------------------------------------------------

// change the default pin assignments for each module here
// the defaults are what are used in the schematic and PCB
// our current setup uses the Arduino Nano Every
// https://content.arduino.cc/assets/Pinout-NANOevery_latest.png

// digital buzzer output pin
// needs to be PWM compatible for passive buzzer
#define BUZZER_PIN 2

// SPI pins
// the below are the defaults for the Arduino Nano Every
// SCK (clock) pin
#define CLOCK_PIN 13
// MISO/CIPO pin
#define CIPO_PIN 12
// MOSI/COPI pin
#define COPI_PIN 11

// SD chip select pin
#define SD_CS_PIN 10

// LoRa chip select pin
#define LORA_CS_PIN 19
// LoRa reset pin
#define LORA_RESET_PIN -1 // -1 to disable (tied to arduino reset pin)
// LoRa interrupt pin
#define LORA_DIO0_PIN -1 // interrupt pin (not used in this project's code)

// GPS pins
#define GPS_RX_PIN 7
#define GPS_TX_PIN -1 // -1 to disable

// Status LED pins
#define GPS_LED_PIN 15 // A1
#define ERROR_LED_PIN 16 // A2
#define STATUS_LED_PIN 20 // A6

// Temperature sensor pin
#define TEMP_PIN A7
// Voltage divider pin (for battery voltage)
#define VOLTAGE_PIN A4

// Transmitter daughterboard pins
#if TRANSMITTER == RADIO_MODULE
  #define PTT_PIN 3
  #define PD_PIN 4
  #define MIC_PIN 6
#elif TRANSMITTER == SATELLITE_MODULE
  #define SLP_PIN 4
#endif

#endif