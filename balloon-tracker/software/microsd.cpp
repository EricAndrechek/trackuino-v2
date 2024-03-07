// Path: software/sd.cpp
// Description: This module handles the SD card. It can be used to log data to the SD card.

#include <SoftwareSerial.h>
#include "Arduino.h"
#include <SPI.h>
#include <SD.h>

#include "config.h"
#include "helpers.h"
#include "microsd.h"
#include "leds.h"
#include "aprs.h"

#define MAIN_FILE "data.txt"

MicroSD::MicroSD() {
    last_log = 0;
}

void MicroSD::setup_handler() {
    // initialize SD card
    if (!SD.begin(SD_CS_PIN)) {
        #ifdef DEBUG
            Serial.println(F("MicroSD module failed to initialize"));
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
            Serial.println(F("MicroSD module failed to open main file"));
        #endif
        leds.set_error();
        
        // lets not kill the program, just try to open the file again later
    }

    log_init(__FILE__, sizeof(MicroSD));
}

// grabs data from every module and logs it to the SD card
void MicroSD::loop_handler() {
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
                current_file.println(F("MicroSD module failed to open main file"));
                current_file.flush();
            #endif
            leds.set_error();
            return;
        }

        current_file.println(aprs_object.packet);
        current_file.flush();

        last_log = millis();
    }
}

MicroSD micro_sd = MicroSD();