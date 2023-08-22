#include <SoftwareSerial.h>
#include "Arduino.h"

#include "config.h"
#include "helpers.h"
#include "satellite.h"
#include "aprs.h"
#include "leds.h"
#include "gps.h"

Satellite::Satellite() {
    setup_handler();
}

void Satellite::setup_handler() {
    // this uses the Arduino Nano Every's hardware serial
    Serial.begin(SATELLITE_BAUDRATE);

    // TODO: need some sort of debug handling for while reading in status info and connecting, etc

    // initial AT command to init
    Serial.println(F("AT\r"));
    delay(1000);
    while (Serial.available()) {
        Serial.read();
    }

    // Get status
    Serial.println(F("AT+SBDSX\r"));
    delay(1000);
    while (Serial.available()) {
        Serial.read();
    }

    // done with setup, ready to transmit

    log_init(__FILE__, sizeof(Satellite));
}

// read in serial and parse +SBDIX response
// setting last_MO_status if available
void Satellite::get_MO_status() {
    // read in serial if available and check if it is a +SBDIX response
    // if it is, parse it
    char response[64];
    while (Serial.available()) {
        response[0] = Serial.read();
        if (response[0] == '+') {
            // read in the rest of the response
            unsigned char i = 1;
            while (Serial.available() && i < 63) {
                response[i] = Serial.read();
                i++;
            }
            response[i] = '\0';

            // check if it is a +SBDIX response
            if (strstr(response, "+SBDIX:") != NULL) {
                // it is, so parse it

                // get MO status
                char* token = strtok(response, ",");
                token = strtok(NULL, ",");
                last_MO_status = atoi(token);
            }
        } else {
            // it is not, so continue
            continue;
        }
    }
}

// check if we have a packet to send
// if we do, send it
void Satellite::send_packet() {
    // +SBDIX:<MO status>,<MOMSN>,<MT status>,<MTMSN>,<MT length>,<MT queued>
    // where MO Status: https://docs.rockblock.rock7.com/reference/sbdix
    // should be 0-4 if successful

    get_MO_status();

    if (last_MO_status < 5) {
        // packet already sent, don't send again
        return;
    }

    if (last_MO_status == 5) {
        // we don't have a packet to send, so don't send
        return;
    }

    // we have a packet to send, so send it
    Serial.print(F("AT+SBDIX\r"));

    get_MO_status();
}

// write packet to satellite module
// and send if no errors occur
void Satellite::write_packet() {

    if (packet_write_errors >= 3) {
        #ifdef DEBUG
            Serial.begin(SERIAL_BAUDRATE);
            Serial.println(F("Satellite module failed to write packet"));
            Serial.flush();
            Serial.end();
        #endif
        leds.set_error();
        packet_write_errors = 0;

        // don't kill, just return. We will try again next time
        return;
    }

    // clear buffer
    while (Serial.available()) {
        Serial.read();
    }
    Serial.print(F("AT+SBDD0\r"));
    // may need to add a delay here
    while (Serial.available()) {
        Serial.read();
    }
    
    // TODO: change to binary mode in the future
    Serial.print(F("AT+SBDWT="));
    Serial.print(aprs_object.packet);
    Serial.print(F("\r"));
    // may need to add a delay here

    // read in response to variable
    // allow it to overwrite so that we don't overflow
    char response[16];
    unsigned char i = 0;
    while (Serial.available()) {
        response[i] = Serial.read();
        i = (i + 1) % 16;
    }

    // check if response includes "OK"
    // TODO: make this happen by sending AT command and data separately
    // so that we can read in the specific status code
    if (strstr(response, "OK") == NULL) {
        // re-send packet
        packet_write_errors++;
        send_packet();
    }

    packet_write_errors = 0;
    last_MO_status = 5;
    send_packet();
}


void Satellite::loop_handler() {

    // check if it is time to broadcast
    if ((millis() - last_transmission) / 1000 >= (DATA_TIMEOUT - (SLOT_ERROR / 2))) {
        // check again to make sure slot is good
        // get current seconds, subtract slot,
        // and check if it's divisible by DATA_TIMEOUT
        // if it's not divisible, we're not in a good slot

        // need margin of +/- SLOT_ERROR seconds to account for drift
        if (!gps.stale && (gps.second - DATA_SLOT) % DATA_TIMEOUT < SLOT_ERROR) {
            // we're in a good slot, write data
            write_packet();
            return;
        } else if (gps.stale) {
            // GPS is stale but it has been about DATA_TIMEOUT seconds
            // we want to broadcast that we are still alive, so broadcast anyway
            write_packet();
            return;
        }
    }

    // if we didn't just send a new message, check that the last message was sent
    // if it wasn't, try again
    send_packet();
}

Satellite satellite = Satellite();