// Path: software/buzzer.cpp
// Description: This module handles the buzzer. It can be used to transmit status information via buzz pulses.

#include <SoftwareSerial.h>
#include "Arduino.h"

// include configuration
#include "config.h"
#include "helpers.h"
#include "buzzer.h"
#include "gps.h"
#include "microsd.h"

Buzzer::Buzzer() {
    last_buzz = 0;
    last_buzz_state = false;
    buzzes_remaining = 0;
    buzzer_status = BuzzerStatus::On;
    queued_status = BuzzerStatus::On;
}

// handle turning buzzer on/off to transmit status beeps
void Buzzer::loop_handler() {
    // if GPS is valid and if we are above altitude threshold
    // multiply by 100 to convert to centimeters
    if (!gps.stale && gps.tinygps_object.altitude() >= (BUZZER_ALTITUDE * 100)) {
        // we are above altitude threshold, so we should not buzz
        // set buzzer into sleep/low power mode
        _stop_buzz();

        // states are the same so it will resume when below altitude threshold
        return;
    }

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
}

// allow other modules to set the status of the buzzer
// this will modify the next status to be transmitted
void Buzzer::set_status(BuzzerStatus status) {
    queued_status = status;
}

// initialize buzzer module and pins
void Buzzer::setup_handler() {
    pinMode(BUZZER_PIN, OUTPUT);

    log_init(__FILE__, sizeof(Buzzer));
}

void Buzzer::_start_buzz() {
    #if BUZZER_TYPE == 0
        digitalWrite(BUZZER_PIN, HIGH);
    #elif BUZZER_TYPE == 1
        tone(BUZZER_PIN, BUZZER_FREQUENCY);
    #endif
    // mark buzzer as on
    last_buzz_state = true;
    // update last_buzz time
    last_buzz = millis();
}

void Buzzer::_stop_buzz() {
    #if BUZZER_TYPE == 0
        digitalWrite(BUZZER_PIN, LOW);
    #elif BUZZER_TYPE == 1
        noTone(BUZZER_PIN);
    #endif
    // mark buzzer as off
    last_buzz_state = false;
    // update last_buzz time
    last_buzz = millis();
    // remove one from the number of buzzes remaining
    buzzes_remaining--;
}

Buzzer buzzer = Buzzer();