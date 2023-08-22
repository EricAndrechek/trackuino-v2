// Path: software/aprs.cpp
// Description: This file builds the APRS packets that can be used to transmit data via radio or satellite.

#include <SoftwareSerial.h>
#include "Arduino.h"
#include <TinyGPSMinus.h>
#include <math.h>

// include configuration
#include "config.h"
#include "helpers.h"
#include "aprs.h"
#include "gps.h"
#include "sensors.h"

// define helper functions

APRS::APRS() {

    build_header();

    log_init(__FILE__, sizeof(APRS));
}

// take the destination, source, and digipeating information and build the header
// this header is only for display purposes and is not the one transmitted
void APRS::build_header() {
    // S_CALLSIGN   -   6 chars
    // "-"          -   1 char
    // S_SSID       -   1 char

    // ">"          -   1 char

    // D_CALLSIGN   -   6 chars
    // "-"          -   1 char
    // D_SSID       -   1 char

    // ":"          -   1 char

    // total: 18 chars

    // build header
    sprintf(header, "%s-%d>%s-%d:", S_CALLSIGN, S_SSID, D_CALLSIGN, D_SSID);
}


void APRS::build_uncompressed_body(char* body, char* hms, char* telemetry) {

    // "/"          -   1 char (position with timestamp without APRS messaging)
    // HMS          -   7 chars (HHMMSS"h")
    // LAT          -   8 chars (ddmm.hhN) degrees, decimal minutes (to 2 decimal places), and N/S
    // APRS_TABLE   -   1 char (symbol table)
    // LON          -   9 chars (dddmm.hhW) degrees, decimal minutes (to 2 decimal places), and E/W
    // APRS_SYMBOL  -   1 char (symbol)
    // CSE          -   3 chars (001-360) course over ground, clockwise from due north
    // "/"          -   1 char course/speed separator
    // SPD          -   3 chars (000-999) speed over ground, knots
    // total:       -   34 chars - 36 left for comment

    // APRS_COMMENT -   19 chars max
    // Altitude     -   9 chars - ("/A="aaaaaa) (feet)
    // Telemetry    -   8 chars - ("|aaaaaa|") (base-91 encoded)
    // total:       -   36 chars

    // total:       -   70 chars

    // Note: for ambiguity in lat/lon data, replace values with spaces starting from the right
    // ie: 1 space = lat to 1/10 minute, 2 = nearest minute, 3 = nearest 10 minutes, 4 = nearest degree
    // This ambiguity level will automatically apply to longitude as well

    // For no GPS lock, transmit at 0˚0'0"N 0˚0'0"W and transmit \. symbol

    // Note: for course and speed unknown, replace with .../...

    char lat[8], lon[9];
    char course[3], speed[3], altitude[6];
    char aprs_table_char = APRS_TABLE;
    char aprs_symbol_char = APRS_SYMBOL;

    // if GPS is not stale
    if (!gps.stale) {
        // update lat and lon
        strcpy(lat, gps.tinygps_object.get_latitude());
        strcpy(lon, gps.tinygps_object.get_longitude());

        // build course, speed, and altitude
        sprintf(course, "%03.0d", gps.tinygps_object.f_course());
        sprintf(speed, "%03.0d", gps.tinygps_object.f_speed_knots());
        sprintf(altitude, "%06.0lf", gps.tinygps_object.altitude() / 30.48);
    } else {
        // GPS is stale, so we should transmit 0˚0'0"N 0˚0'0"W and transmit \. symbol
        strcpy(lat, "0000.00N");
        strcpy(lon, "00000.00W");
        strcpy(course, "...");
        strcpy(speed, "...");
        strcpy(altitude, "000000");
        aprs_table_char = '\\';
        aprs_symbol_char = '.';
    }

    sprintf(body, "/%s%s%c%s%c%s/%s/%s/A=%s%s", hms, lat, aprs_table_char, lon, aprs_symbol_char, course, speed, APRS_COMMENT, altitude, telemetry);
}

void APRS::build_compressed_body(char* body, char* hms, char* telemetry) {
    // the inputted values are in the uncompressed format, so they must be converted to the compressed format using base-91 encoding

    // "/"          -   1 char (position with timestamp without APRS messaging)
    // HMS          -   7 chars (HHMMSS"h")
    // APRS_TABLE   -   1 char (symbol table)
    // YYYY         -   4 chars (compressed latitude)
    // XXXX         -   4 chars (compressed longitude)
    // APRS_SYMBOL  -   1 char (symbol)
    // CS           -   2 chars (course and speed)
    // T            -   1 char (compression type indicator)
    // total:       -   21 chars - 40 left for comment

    // APRS_COMMENT -   19 chars max
    // Altitude     -   9 chars - ("/A="aaaaaa) (feet)
    // Telemetry    -   8 chars - ("|aaaaaa|") (base-91 encoded)
    // total:       -   36 chars

    // total:       -   57 chars


    char T = 0b00100110;
    char aprs_table_char = APRS_TABLE;
    char aprs_symbol_char = APRS_SYMBOL;
    char y[4], x[4];
    char c;
    char s;
    char altitude[6];

    // if GPS is not stale
    if (!gps.stale) {
        // get uncompressed type lat and lon
        char lat[8], lon[9];
        strcpy(lat, gps.tinygps_object.get_latitude());
        strcpy(lon, gps.tinygps_object.get_longitude());

        // turn into decimal degrees negative or positive
        char lat_degrees[2], lon_degrees[3];
        char lat_minutes[5], lon_minutes[5];
        char lat_direction, lon_direction;

        // get lat degrees
        lat_degrees[0] = lat[0];
        lat_degrees[1] = lat[1];

        // get lon degrees
        lon_degrees[0] = lon[0];
        lon_degrees[1] = lon[1];
        lon_degrees[2] = lon[2];

        // get lat minutes
        lat_minutes[0] = lat[2];
        lat_minutes[1] = lat[3];
        lat_minutes[2] = lat[4];
        lat_minutes[3] = lat[5];
        lat_minutes[4] = lat[6];

        // get lon minutes
        lon_minutes[0] = lon[3];
        lon_minutes[1] = lon[4];
        lon_minutes[2] = lon[5];
        lon_minutes[3] = lon[6];
        lon_minutes[4] = lon[7];

        // get lat direction
        lat_direction = lat[7];

        // get lon direction
        lon_direction = lon[8];

        // convert to decimal degrees
        double lat_decimal = atof(lat_minutes) / 60.0;
        lat_decimal += atof(lat_degrees);
        if (lat_direction == 'S') {
            lat_decimal *= -1;
        }

        double lon_decimal = atof(lon_minutes) / 60.0;
        lon_decimal += atof(lon_degrees);
        if (lon_direction == 'W') {
            lon_decimal *= -1;
        }

        unsigned long lat_pre_compression = 380926 * (90.0 - lat_decimal);
        unsigned long lon_pre_compression = 190463 * (180.0 + lon_decimal);

        // compress lat and lon
        base91_encode(lat_pre_compression, y, 4);
        base91_encode(lon_pre_compression, x, 4);

        // build course, speed, and altitude
        c = round(gps.tinygps_object.f_course() / 4.0) + 33;
        s = round(log(gps.tinygps_object.f_speed_knots() + 1) / log(1.08)) + 33.0;
        sprintf(altitude, "%06.0lf", gps.tinygps_object.altitude() / 30.48);
    } else {
        // GPS is stale, so we should transmit 0˚0'0"N 0˚0'0"W and transmit \. symbol
        strcpy(y, "0000");
        strcpy(x, "0000");

        c = ' ';
        s = ' ';
        strcpy(altitude, "000000");
        aprs_table_char = '\\';
        aprs_symbol_char = '.';
    }

    sprintf(body, "/%s%c%s%s%c%c%c%c/%s/A=%s%s", hms, aprs_table_char, y, x, aprs_symbol_char, c, s, T, APRS_COMMENT, altitude, telemetry);
}

// this assumes latitude and longitude inputs have been adjusted according to APRS spec already
void APRS::base91_encode(unsigned long &input, char* output, int length) {
    for (char i = length - 1; i > 0; i--) {
        output[i] = input % 91 + 33;
        input /= 91;
    }
    output[0] = input % 91 + 33;
    output[length] = '\0';
}
void APRS::base91_encode(unsigned char &input, char* output, int length) {
    for (char i = length - 1; i > 0; i--) {
        output[i] = input % 91 + 33;
        input /= 91;
    }
    output[0] = input % 91 + 33;
    output[length] = '\0';
}
void APRS::base91_encode(float &input, char* output, int length) {
    unsigned long input_long = round(input);
    for (char i = length - 1; i > 0; i--) {
        output[i] = input_long % 91 + 33;
        input_long /= 91;
    }
    output[0] = input_long % 91 + 33;
    output[length] = '\0';
}

// build base91 telemetry data and increment sequence number
void APRS::build_telemetry(char* telemetry) {
    char encoded_sequence[2];
    char encoded_temperature[2];
    char encoded_voltage[2];

    // encode values
    base91_encode(sequence, encoded_sequence, 2);
    base91_encode(sensors.temperature, encoded_temperature, 2);
    base91_encode(sensors.voltage, encoded_voltage, 2);

    // build telemetry
    sprintf(telemetry, "|%s%s%s|", encoded_sequence, encoded_temperature, encoded_voltage);

    // keep sequence number between 0 and 255, wrapping around if necessary
    sequence = (sequence + 1) & 255;
}


// build uncompressed and compressed packets to pass for any modules that may need it 
// (ie: LoRa, radio, satellite) using the gps class
void APRS::loop_handler() {

    // build variables locally so they are freed after this function
    char hms[7];
    char telemetry[8];
    
    // build hms
    sprintf(hms, "%02d%02d%02dh", gps.hour, gps.minute, gps.second);

    // build telemetry
    build_telemetry(telemetry);

    #if COMPRESSED == true
        char body[57];
        // build compressed body
        build_compressed_body(body, hms, telemetry);
        // build full packet
        sprintf(packet, "%s%s", header, body);
        // null terminate packet
        packet[75] = '\0';
        info_len = 75 - 18;
    #else
        char body[70];
        // build uncompressed body
        build_uncompressed_body(body, hms, telemetry);
        // build full packet
        sprintf(packet, "%s%s", header, body);
        // null terminate packet
        packet[88] = '\0';
        info_len = 88 - 18;
    #endif
}

APRS aprs_object = APRS();