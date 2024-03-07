#include <SoftwareSerial.h>
#include "Arduino.h"
#include <TinyGPSMinus.h>

#include "config.h"
#include "helpers.h"
#include "gps.h"
#include "leds.h"
#include "microsd.h"

GPS::GPS() {
    stale = true;
    last_stale = false;
}

void GPS::setup_handler(SoftwareSerial * ss) {
    if (!ss) {
        #ifdef DEBUG
            micro_sd.current_file.println(F("GPS serial port is null"));
            micro_sd.current_file.flush();
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
        char temp = gps_serial->read();
        tinygps_object.encode(temp);
        Serial.print(temp);
    }

    // if GPS data is valid
    gps.get_datetime();
    
    #ifdef DEBUG
        // ready for use, stored in gps_object to avoid storing in multiple variables
        // LED library can check for fix and update LED accordingly
        if (!stale && last_stale) {
            micro_sd.current_file.println(F("GPS fix acquired"));
            micro_sd.current_file.flush();
        } else if (stale && !last_stale) {
            // no fix yet
            // LED library can check for fix and update LED accordingly
            micro_sd.current_file.println(F("No GPS fix yet"));
            micro_sd.current_file.flush();
        }
    #endif
}

void GPS::get_datetime() {
    tinygps_object.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
    if (age == TinyGPSMinus::GPS_INVALID_AGE || age > GPS_STALE_AGE) {
        // don't bother overwriting datetime, just mark it as stale
        last_stale = stale;
        stale = true;
    } else {
        sprintf(datetime, "%02d/%02d/%02d %02d:%02d:%02d ", month, day, year, hour, minute, second);
        last_stale = stale;
        stale = false;
    }
}

GPS gps = GPS();