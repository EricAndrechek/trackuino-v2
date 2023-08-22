#include "Arduino.h"
#include <SoftwareSerial.h>

// import configuration settings
#include "config.h"

#include "helpers.h"

// include module code
#include "gps.h"
#include "sensors.h"
#include "aprs.h"
#include "sd.h"
#include "buzzer.h"
#include "leds.h"
#if TRANSMITTER == 1
    #include "satellite.h"
#else
    #include "radio.h"
#endif
#include "lora.h"

SoftwareSerial gps_ss(GPS_RX_PIN, GPS_TX_PIN);

void version() {
    #ifdef DEBUG
        Serial.begin(SERIAL_BAUDRATE);
        Serial.println(F("Balloon Tracker Starting..."));
        Serial.println(F("Version: 0.1"));
        Serial.print(F("Sketch: "));
        Serial.println(__FILE__);
        Serial.print(F("Uploaded: "));
        Serial.println(__DATE__);
        Serial.println();
        Serial.flush();
        Serial.end();
    #endif
}

void module_setup() {
    // Initialize modules that use the SoftwareSerial library
    // the rest are initialized in their respective classes upon construction
    gps.setup_handler(&gps_ss);
}

void setup() {
    // print version info
    version();

    // initialize modules
    module_setup();

    // setup SPI CS Pins
    pinMode(LORA_CS_PIN, OUTPUT);
    digitalWrite(LORA_CS_PIN, HIGH);
    pinMode(SD_CS_PIN, OUTPUT);
    digitalWrite(SD_CS_PIN, HIGH);

    // print memory info
    print_free_mem();
}

void loop() {
    // get new gps data
    gps.loop_handler();
    // get new sensor data
    sensors.loop_handler();
    // process gps data into packets
    aprs_object.loop_handler();
    // log data to sd card
    digitalWrite(LORA_CS_PIN, HIGH);
    digitalWrite(SD_CS_PIN, LOW);
    sd_object.loop_handler();
    digitalWrite(SD_CS_PIN, HIGH);
    digitalWrite(LORA_CS_PIN, LOW);
    // play buzzer for statuses
    buzzer.loop_handler();
    // update leds for statuses
    leds.loop_handler();
    // transmit data over lora
    lora_object.loop_handler();
    #if TRANSMITTER == 1
        // transmit data over satellite
        satellite.loop_handler();
    #else
        // transmit data over radio
        radio.loop_handler();
    #endif
}