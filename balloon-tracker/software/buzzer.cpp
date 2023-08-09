// Path: software/buzzer.cpp
// Description: This module handles the buzzer. It can be used to transmit status information via buzz pulses.

// include libraries
#include <SoftwareSerial.h>

// include configuration
#include "config.h"
#include "variables.h"

// helper functions
void buzzer_setup();
void _start_buzz();
void _stop_buzz();

// needs to be defined for other functions to reference
// error-free regardless of buzzer module being enabled
enum BuzzerStatus {
    On,
    Lock,
    Transmitting,
    Error
};

// BUZZER VARIABLES

// only allocate memory if buzzer module is enabled
#ifdef BUZZER_MODULE

    // last time buzzer was started/stopped
    unsigned long last_buzz = 0;

    // last state of buzzer; true = on, false = off
    bool last_buzz_state = false;

    // number of remaining buzzes to transmit status
    unsigned char buzzes_remaining = 1;

    // latest status to transmit via buzzer
    BuzzerStatus buzzer_status = BuzzerStatus::On;

    BuzzerStatus queued_status = BuzzerStatus::On;

#endif

// handle turning buzzer on/off to transmit status beeps
void buzzer_loop() {
    #ifdef BUZZER_MODULE
        // if buzzer is on
        if (last_buzz_state) {
            // if buzzer has been on for at least BUZZER_DURATION
            if (millis() - last_buzz >= BUZZER_DURATION) {
                // stop buzzer
                _stop_buzz();
            }
        }
        // if buzzer is off
        else {
            // check if there is another buzz to transmit and if it's time to buzz
            if (buzzes_remaining > 0 && millis() - last_buzz >= BUZZER_DELAY) {
                // start buzzer
                _start_buzz();

                // if there are no more buzzes to transmit and it's time for the next interval
            } else if (buzzes_remaining == 0 && millis() - last_buzz >= BUZZER_INTERVAL) {
                // move buzz queue to current status
                buzzer_status = queued_status;
                // set next status back to default (on)
                queued_status = BuzzerStatus::On;
                // set buzzes remaining
                switch (buzzer_status) {
                    case BuzzerStatus::On:
                        buzzes_remaining = 1;
                        break;
                    case BuzzerStatus::Lock:
                        buzzes_remaining = 2;
                        break;
                    case BuzzerStatus::Transmitting:
                        buzzes_remaining = 3;
                        break;
                    case BuzzerStatus::Error:
                        buzzes_remaining = 4;
                        break;
                }
                // start buzzer
                _start_buzz();
            }
        }
    #endif
}

// allow other modules to set the status of the buzzer
// this will modify the next status to be transmitted
void buzzer_status(BuzzerStatus status) {
    #ifdef BUZZER_MODULE
        queued_status = status;
    #endif
}

// initialize buzzer module and pins
void buzzer_setup() {
    #ifdef BUZZER_MODULE
        info_print(F("Buzzer Module Enabled"));
        pinMode(BUZZER_PIN, OUTPUT);
    #endif
}

void _start_buzz() {
    #ifdef BUZZER_MODULE
        #if BUZZER_TYPE == ACTIVE
            digitalWrite(BUZZER_PIN, HIGH);
        #elif BUZZER_TYPE == PASSIVE
            tone(BUZZER_PIN, BUZZER_FREQUENCY);
        #endif
        // mark buzzer as on
        last_buzz_state = true;
        // update last_buzz time
        last_buzz = millis();
    #endif
}

void _stop_buzz() {
    #ifdef BUZZER_MODULE
        #if BUZZER_TYPE == ACTIVE
            digitalWrite(BUZZER_PIN, LOW);
        #elif BUZZER_TYPE == PASSIVE
            noTone(BUZZER_PIN);
        #endif
        // mark buzzer as off
        last_buzz_state = false;
        // update last_buzz time
        last_buzz = millis();
        // remove one from the number of buzzes remaining
        buzzes_remaining--;
    #endif
}