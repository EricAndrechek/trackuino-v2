// import configuration settings
#include "config.h"

// setup debug helpers and macros
#include "debug_helpers.cpp"

// include module code
#include "aprs.cpp"
#include "gps.cpp"
#include "sd.cpp"
#include "buzzer.cpp"
#include "radio.cpp"
#include "satellite.cpp"

void version() {
    info_print(F("Balloon Tracker Starting..."));
    info_print(F("Version: 0.1"));
    info_print(F("Sketch: ") + __FILE__);
    info_print(F("Uploaded: ") + __DATE__);
}

void module_setup() {
    // Initialize modules
    gps_setup();
    sd_setup();
    buzzer_setup();
    aprs_setup();
    #if TRANSMITTER == RADIO_MODULE
        radio_setup();
    #elif TRANSMITTER == SATELLITE_MODULE
        satellite_setup();
    #endif
    debuggers_setup();
}

void setup() {
    // print version info
    version();

    // initialize modules
    module_setup();

    // print memory info
    print_free_mem();
}

void loop() {
    gps_loop();
    buzzer_loop();
}