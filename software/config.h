#ifndef __CONFIG_H__
#define __CONFIG_H__

// THIS IS THE TRACKUINO FIRMWARE CONFIGURATION FILE. YOUR CALLSIGN AND
// OTHER SETTINGS GO HERE.
//
// NOTE: all pins are Arduino based, not the Atmega chip. Mapping:
// https://www.arduino.cc/en/Hacking/PinMapping


// --------------------------------------------------------------------------
// GENERAL CONFIGURATION
// --------------------------------------------------------------------------

// Sensors Config (sensors.cpp)

// Most of the sensors.cpp functions use internal reference voltages (either
// AVCC or 1.1V). If you want to use an external reference, you should
// uncomment the following line:
//
// #define USE_AREF
//
// BEWARE! If you hook up an external voltage to the AREF pin and 
// accidentally set the ADC to any of the internal references, YOU WILL
// FRY YOUR AVR.
//
// It is always advised to connect the AREF pin through a pull-up resistor,
// whose value is defined here in ohms (set to 0 if no pull-up):
//
#define AREF_PULLUP           4700
//
// Since there is already a 32K resistor at the ADC pin, the actual
// voltage read will be VREF * 32 / (32 + AREF_PULLUP)
//
// Read more in the Arduino reference docs:
// http://arduino.cc/en/Reference/AnalogReference?from=Reference.AREF

// Pin mappings for the internal / external temperature sensors. VS refers
// to (arduino) digital pins, whereas VOUT refers to (arduino) analog pins.
#define INTERNAL_LM60_VS_PIN     6
#define INTERNAL_LM60_VOUT_PIN   0
#define EXTERNAL_LM60_VS_PIN     7
#define EXTERNAL_LM60_VOUT_PIN   1

// Units for temperature sensors (Added by: Kyle Crockett)
// 1 = Celsius, 2 = Kelvin, 3 = Fahrenheit
#define TEMP_UNIT 1

// Calibration value in the units selected. Use integer only.
#define CALIBRATION_VAL 0

// Resistors divider for the voltage meter (ohms)
#define VMETER_R1       10000
#define VMETER_R2       3300

// Voltage meter analog pin
#define VMETER_PIN      2


// --------------------------------------------------------------------------
// MODULE CONFIGURATION
// --------------------------------------------------------------------------

// Module Config

// Uncomment the lines for each module you would like to enable
// #define APRS_MODULE
// #define BUZZER_MODULE
// #define GPS_MODULE
// #define GSM_MODULE
// #define SD_MODULE

// Be sure to fill out the configuration for each module you are using.


// --------------------------------------------------------------------------
// APRS MODULE CONFIGURATION
// --------------------------------------------------------------------------

// APRS Config (aprs.cpp)

// Set your callsign and SSID here. Common values for the SSID are
// (from http://www.aprs.org/aprs11/SSIDs.txt):
// 0  Your primary station usually fixed and message capable
// 1  generic additional station, digi, mobile, wx, etc
// 2  generic additional station, digi, mobile, wx, etc
// 3  generic additional station, digi, mobile, wx, etc
// 4  generic additional station, digi, mobile, wx, etc
// 5  Other networks(Dstar, Iphones, Androids, Blackberry's etc)
// 6  Special activity, Satellite ops, camping or 6 meters, etc
// 7  walkie talkies, HT's or other human portable
// 8  boats, sailboats, RV's or second main mobile
// 9  Primary Mobile(usually message capable)
// 10 internet, Igates, echolink, winlink, AVRS, APRN, etc
// 11 balloons, aircraft, spacecraft, etc
// 12 APRStt, DTMF, RFID, devices, one - way trackers*, etc
// 13 Weather stations
// 14 Truckers or generally full time drivers
// 15 generic additional station, digi, mobile, wx, etc

#define S_CALLSIGN      "MYCALL"
#define S_CALLSIGN_ID   11   // default is 11 for balloon

// set the symbol you want the aprs.fi map to represent you as:
// http://www.aprs.net/vm/DOS/SYMBOLS.HTM#:~:text=PRIMARY%20SYMBOL%20TABLE%20(/)
#define APRS_SYMBOL     'O'  // default is 'O' for balloon

// Destination callsign: APRS (with SSID=0) is usually okay.
#define D_CALLSIGN      "APRS"
#define D_CALLSIGN_ID   0

// Digipeating paths:
// (read more about digipeating paths here: http://wa8lmf.net/DigiPaths/ )
// The recommended digi path for a balloon is WIDE2-1
#define DIGI_PATH1      "WIDE2"
#define DIGI_PATH1_TTL  1

// APRS comment: this goes in the comment portion of the APRS message. You
// might want to keep this short. The longer the packet, the more vulnerable
// it is to noise. 
#define APRS_COMMENT    "Trackuino reminder: replace callsign with your own"

// AX.25 Config (ax25.cpp)

// TX delay in milliseconds
#define TX_DELAY      300
// TODO figure out why 300ms default and what this setting changes

// Tracker Config (trackuino.ino)

// APRS packets are slotted so that multiple trackers can be used without
// them stepping on one another. The transmission times are governed by
// the formula:
//
//         APRS_SLOT (seconds) + n * APRS_PERIOD (seconds)
//
// When launching multiple balloons, use the same APRS_PERIOD in all balloons
// and set APRS_SLOT so that the packets are spaced equally in time.
// Eg. for two balloons and APRS_PERIOD = 60, set APRS_SLOT to 0 and 30, 
// respectively. The first balloon will transmit at 00:00:00, 00:01:00, 
// 00:02:00, etc. and the second balloon will transmit at 00:00:30, 00:01:30,
// 00:02:30, etc.
#define APRS_SLOT     0     // seconds. -1 disables slotted transmissions
#define APRS_PERIOD   60    // seconds

// Modem Config (afsk.cpp)

// AUDIO_PIN is the audio-out pin. The audio is generated by timer 2 using
// PWM, so the only two options are pins 3 and 11.
// Pin 11 doubles as MOSI, so I suggest using pin 3 for PWM and leave 11 free
// in case you ever want to interface with an SPI device.
#define AUDIO_PIN       3

// Pre-emphasize the 2200 tone by 6 dB. This is actually done by 
// de-emphasizing the 1200 tone by 6 dB and it might greatly improve
// reception at the expense of poorer FM deviation, which translates
// into an overall lower amplitude of the received signal. 1 = yes, 0 = no.
#define PRE_EMPHASIS    1

// Radio Config (radio_hx1.cpp)

// This is the PTT pin
#define PTT_PIN           4

// Debug Config

// Turn on this module's debugging mode by uncommenting the following lines.
// Each sub-module has its own debug mode.
// #define APRS_DEBUG_AX25      // AX.25 frame dump
// #define APRS_DEBUG_MODEM     // Modem ISR overrun and profiling
// #define APRS_DEBUG_AFSK      // AFSK (modulation) output
// #define APRS_DEBUG           // APRS packet dump


// --------------------------------------------------------------------------
// BUZZER MODULE CONFIGURATION
// --------------------------------------------------------------------------

// Buzzer Config (buzzer.cpp)

// Type of buzzer (0=active, 1=passive). An active buzzer is driven by a
// DC voltage. A passive buzzer needs a PWM signal.
#define BUZZER_TYPE             0

// When using a passive buzzer, specify the PWM frequency here. Choose one
// that maximizes the volume according to the buzzer's datasheet. Not all
// the frequencies are valid, check out the buzzer_*.cpp code. On Arduino,
// it must be between L and 65535, where L = F_CPU / 65535 and F_CPU is the
// clock rate in hertzs. For 16 MHz Arduinos, this gives a lower limit of 
// 245 Hz.
#define BUZZER_FREQ             895     // Hz

// These are the number of seconds the buzzer will stay on/off alternately
#define BUZZER_ON_TIME          1       // secs
#define BUZZER_OFF_TIME         2       // secs

// This option disables the buzzer above BUZZER_ALTITUDE meters. This is a
// float value, so make it really high (eg. 1000000.0 = 1 million meters)
// if you want it to never stop buzzing.
#define BUZZER_ALTITUDE         3000.0  // meters (1 ft = 0.3048 m)

// The options here are pin 9 or 10
#define BUZZER_PIN              9

// Turn on this module's debugging mode by uncommenting the following line.
// #define BUZZER_DEBUG             // Buzzer debug


// --------------------------------------------------------------------------
// GPS MODULE CONFIGURATION
// --------------------------------------------------------------------------

// GPS Config (gps.cpp)

// GPS baud rate (in bits per second). This is also the baud rate at which
// debug data will be printed out the serial port.
#define GPS_BAUDRATE  9600

// Turn on this module's debugging mode by uncommenting the following line.
// #define GPS_DEBUG                // GPS sentence dump and checksum validation


// --------------------------------------------------------------------------
// GSM MODULE CONFIGURATION
// --------------------------------------------------------------------------

// GSM Config (gsm.cpp)

// Turn on this module's debugging mode by uncommenting the following line.
// #define GSM_DEBUG                // Signal strength and out/in-bound data dump


// --------------------------------------------------------------------------
// SD MODULE CONFIGURATION
// --------------------------------------------------------------------------

// SD Config (sd.cpp)

// Turn on this module's debugging mode by uncommenting the following line.
// #define SD_DEBUG                 // Raw SD card data dump


// --------------------------------------------------------------------------
// DEVELOPMENT CONFIGURATION
// --------------------------------------------------------------------------

// Power Config (power_.cpp)

// This is the LED pin (13 on Arduinos). The LED will be on while the AVR is
// running and off while it's sleeping, so its brightness gives an indication
// of the CPU activity.
#define LED_PIN                 13

// Debug info includes printouts from different modules to aid in testing and
// debugging.
//
// Some of the DEBUG modes will cause invalid modulation, so do NOT forget
// to turn them off when you put this to real use.
//
// Particularly the APRS_DEBUG_AFSK will print every PWM sample out the
// serial port, causing extreme delays in the actual AFSK transmission.
// 
// 1. To properly receive debug information, only connect the Arduino RX pin 
//    to the GPS TX pin, and leave the Arduino TX pin disconnected. 
//
// 2. On the serial monitor, set the baudrate to GPS_BAUDRATE (above),
//    usually 9600.
//
// 3. When flashing the firmware, disconnect the GPS from the RX pin or you
//    will get errors.

// Tracker Config (trackuino.ino)

// #define RESET_DEBUG          // AVR reset
// #define SENSOR_DEBUG         // Sensors


#endif

