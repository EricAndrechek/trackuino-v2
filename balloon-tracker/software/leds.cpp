// Path: software/leds.cpp
// Description: This module handles the on-board status LEDS.

// LED purposes:
// FIX: display status of GPS fix
// ERROR: indicate if there is an error
// STATUS: indicate status of the system (ie: transmitting, etc)

#include <SoftwareSerial.h>
#include "Arduino.h"

#include "config.h"
#include "helpers.h"
#include "gps.h"
#include "leds.h"

// how long to keep the ERROR LED on for
#define ERROR_LED_INTERVAL 5000 // 5 seconds

// status LED just copies buzzer LED time constants

LEDS::LEDS() {
    setup_handler();
}

void LEDS::setup_handler() {
    pinMode(GPS_LED_PIN, OUTPUT);
    pinMode(ERROR_LED_PIN, OUTPUT);
    pinMode(STATUS_LED_PIN, OUTPUT);

    log_init(__FILE__, sizeof(LEDS));
}

void LEDS::loop_handler() {

    // GPS LED is effectively stateless, but we keep track of it's state to avoid unnecessary digitalWrite() calls
    // if GPS is stale and GPS LED is on
    if (gps.stale && GPS_LED_state) {
        digitalWrite(GPS_LED_PIN, LOW);
    } else if (!gps.stale && !GPS_LED_state) {
        // if GPS is not stale and GPS LED is off
        digitalWrite(GPS_LED_PIN, HIGH);
    }

    // errors should only be displayed for a short period of time
    // if the ERROR LED is on and it's time to turn it off
    if (ERROR_LED_state && millis() - last_ERROR_LED >= ERROR_LED_INTERVAL) {
        // turn off ERROR LED
        digitalWrite(ERROR_LED_PIN, LOW);
    }

    // if status LED is on
    if (STATUS_LED_state) {
        // if status LED has been on for at least BUZZER_DURATION
        if (millis() - last_STATUS_LED >= BUZZER_DURATION) {
            // turn off LED
            digitalWrite(STATUS_LED_PIN, LOW);
            // mark status LED as off
            STATUS_LED_state = false;
            // update last status LED update time
            last_STATUS_LED = millis();
            // remove one from the number of flashes remaining
            STATUS_LED_flashes_remaining--;
        }
    }
    // if status LED is off
    else {
        // check if there is another status led flash to display and if it's time to flash
        if (STATUS_LED_flashes_remaining > 0 && millis() - last_STATUS_LED >= BUZZER_DELAY) {
            // turn on LED
            digitalWrite(STATUS_LED_PIN, HIGH);
            STATUS_LED_state = true;
            // update last status LED update time
            last_STATUS_LED = millis();

            // if there are no more status flashes to display and it's time for the next interval
        } else if (STATUS_LED_flashes_remaining == 0 && millis() - last_STATUS_LED >= BUZZER_INTERVAL) {
            // move status queue to current status
            STATUS_LED_status = STATUS_LED_queued_status;
            // set next status back to default (on)
            STATUS_LED_queued_status = STATUS::ON;
            // set flashes remaining
            switch (STATUS_LED_status) {
                case STATUS::ON:
                    STATUS_LED_flashes_remaining = 1;
                    break;
                case STATUS::SAVING:
                    STATUS_LED_flashes_remaining = 2;
                    break;
                case STATUS::TRANSMITTING:
                    STATUS_LED_flashes_remaining = 3;
                    break;
                case STATUS::ERROR:
                    STATUS_LED_flashes_remaining = 4;
                    break;
            }
            // turn on LED
            digitalWrite(STATUS_LED_PIN, HIGH);
            STATUS_LED_state = true;
            // update last status LED update time
            last_STATUS_LED = millis();
        }
    }
}

// queue a status to be displayed
void LEDS::set_status(STATUS status) {
    // overwrite current queued status
    STATUS_LED_queued_status = status;
}

void LEDS::set_error() {
    // turn on ERROR LED
    digitalWrite(ERROR_LED_PIN, HIGH);
    // update last ERROR LED update time
    last_ERROR_LED = millis();
}

LEDS leds = LEDS();