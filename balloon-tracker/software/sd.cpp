// Path: software/sd.cpp
// Description: This module handles the SD card. It can be used to log data to the SD card.

#include <SoftwareSerial.h>
#include "Arduino.h"
#include <SPI.h>
#include <SD.h>

#include "config.h"
#include "helpers.h"
#include "sd.h"
#include "leds.h"
#include "aprs.h"

#define MAIN_FILE "data.txt"

SD_class::SD_class() {
    setup_handler();
}

void SD_class::setup_handler() {
    // initialize SD card
    if (!SD.begin(SD_CS_PIN)) {
        #ifdef DEBUG
            Serial.begin(SERIAL_BAUDRATE);
            Serial.println(F("SD module failed to initialize"));
            Serial.flush();
            Serial.end();
        #endif
        leds.set_error();

        // kill program
        while (1);
    }

    // open main file
    current_file = SD.open(MAIN_FILE, FILE_WRITE);

    // if file not opened successfully
    if (!current_file) {
        // if file failed to open
        #ifdef DEBUG
            Serial.begin(SERIAL_BAUDRATE);
            Serial.println(F("SD module failed to open main file"));
            Serial.flush();
            Serial.end();
        #endif
        leds.set_error();
        
        // lets not kill the program, just try to open the file again later
    }

    log_init(__FILE__, sizeof(SD_class));
}

// grabs data from every module and logs it to the SD card
void SD_class::loop_handler() {
    // if it's time to log
    if (millis() - last_log >= LOG_INTERVAL) {
        // open main file if not opened
        if (!current_file) {
            current_file = SD.open(MAIN_FILE, FILE_WRITE);
        }
        // if that failed, try again later
        if (!current_file) {
            // if file failed to open
            #ifdef DEBUG
                Serial.begin(SERIAL_BAUDRATE);
                Serial.println(F("SD module failed to open main file"));
                Serial.flush();
                Serial.end();
            #endif
            leds.set_error();
            return;
        }

        #ifdef DEBUG
            Serial.begin(SERIAL_BAUDRATE);
            Serial.print(F("Logging to SD card: "));
            Serial.println(aprs_object.packet);
            Serial.flush();
            Serial.end();
        #endif

        current_file.println(aprs_object.packet);
        current_file.flush();

        last_log = millis();
    }
}

SD_class sd_object = SD_class();