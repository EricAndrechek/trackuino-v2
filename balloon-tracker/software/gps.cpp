#include <SoftwareSerial.h>
#include "Arduino.h"
#include <TinyGPSMinus.h>
#include "config.h"
#include "helpers.h"
#include "gps.h"
#include "leds.h"

GPS::GPS() {
    // do nothing
}

void GPS::setup_handler(SoftwareSerial * ss) {
    if (!ss) {
        #ifdef DEBUG
            Serial.begin(SERIAL_BAUDRATE);
            Serial.println(F("GPS serial port is null"));
            Serial.flush();
            Serial.end();
        #endif
        leds.set_error();

        // kill program
        while (1);
    }

    // declare GPS serial port with software serial
    gps_serial = ss;
    // begin GPS serial port
    gps_serial->begin(GPS_BAUDRATE);
    log_init(__FILE__, sizeof(GPS));
}

void GPS::loop_handler() {
    while (gps_serial->available() > 0) {
        tinygps_object.encode(gps_serial->read());
    }

    // if GPS data is valid
    gps.get_datetime();
    if (!stale) {
        // ready for use, stored in gps_object to avoid storing in multiple variables
        // LED library can check for fix and update LED accordingly
        #ifdef DEBUG
            Serial.begin(SERIAL_BAUDRATE);
            Serial.println(F("GPS fix acquired"));
            Serial.flush();
            Serial.end();
        #endif
    } else {
        // no fix yet
        // LED library can check for fix and update LED accordingly
        #ifdef DEBUG
            Serial.begin(SERIAL_BAUDRATE);
            Serial.println(F("No GPS fix yet"));
            Serial.flush();
            Serial.end();
        #endif
    }
}

void GPS::get_datetime() {
    tinygps_object.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
    if (age == TinyGPSMinus::GPS_INVALID_AGE || age > GPS_STALE_AGE) {
        // don't bother overwriting datetime, just mark it as stale
        stale = true;
    } else {
        sprintf(datetime, "%02d/%02d/%02d %02d:%02d:%02d ", month, day, year, hour, minute, second);
        stale = false;
    }
}

GPS gps = GPS();