// Path: software/aprs.cpp
// Description: This file builds the APRS packets that can be used to transmit data via radio or satellite.

// include libraries
#include <SoftwareSerial.h>

// include configuration
#include "config.h"
#include "variables.h"

// define helper functions


// take the destination, source, and digipeating information and build the header
// this header is only for display purposes and is not the one transmitted
char build_header() {
    // S_CALLSIGN   -   6 chars
    // "-"          -   1 char
    // S_SSID       -   1 char

    // ">"          -   1 char

    // D_CALLSIGN   -   6 chars
    // "-"          -   1 char
    // D_SSID       -   1 char

    // ":"          -   1 char

    // total: 18 chars
    // 19th char is null terminator

    char header[19];

    // build header
    sprintf(header, "%s-%d>%s-%d:", S_CALLSIGN, S_SSID, D_CALLSIGN, D_SSID);

    return header;
}


char build_uncompressed_body(char *HMS, char *LAT, char *LON, char *CSE, char *SPD, char *ALT) {
    

    // "="          -   1 char (position without timestamp with APRS messaging)
    // HMS          -   7 chars (HHMMSS"h")
    // LAT          -   8 chars (ddmm.hhN) degrees, decimal minutes (to 2 decimal places), and N/S
    // APRS_TABLE   -   1 char (symbol table)
    // LON          -   9 chars (dddmm.hhW) degrees, decimal minutes (to 2 decimal places), and E/W
    // APRS_SYMBOL  -   1 char (symbol)
    // CSE          -   3 chars (001-360) course over ground, clockwise from due north
    // "/"          -   1 char course/speed separator
    // SPD          -   3 chars (000-999) speed over ground, knots

    // Note: for ambiguity in lat/lon data, replace values with spaces starting from the right
    // ie: 1 space = lat to 1/10 minute, 2 = nearest minute, 3 = nearest 10 minutes, 4 = nearest degree
    // This ambiguity level will automatically apply to longitude as well

    // For no GPS lock, transmit at 0˚0'0"N 0˚0'0"W and transmit \. symbol

    // Note: for course and speed unknown, replace with .../...


    // total: 34 chars

    // 36 left for comment

    // Altitude     -   9 chars - should be in comment text in format /A=aaaaaa (feet)

    // 45 total chars (44 + null terminator)

    char body[44];

    sprintf(body, "=%s%s%c%s%c%s/%s%s", HMS, LAT, APRS_TABLE, LON, APRS_SYMBOL, CSE, SPD, ALT);

    return body;
}

char build_compressed_body() {

    // "/"          -   1 char (position without timestamp with APRS messaging)
    // HMS          -   7 chars (HHMMSS"h")
    // APRS_TABLE   -   1 char (symbol table)
    // YYYY         -   4 chars (compressed latitude)
    // XXXX         -   4 chars (compressed longitude)
    // APRS_SYMBOL  -   1 char (symbol)
    // CS           -   2 chars (course and speed)
    // T            -   1 char (compression type indicator)

    // all bytes except / and $ are base-91 printable ASCII characters
    // they are converted to numeric values by subtracting 33 from their ASCII value
    // ie # = 35 - 33 = 2


