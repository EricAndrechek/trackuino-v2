#include "Arduino.h"
#include <SoftwareSerial.h>

// import configuration settings
#include "config.h"

#include "helpers.h"

// include module code
#include "gps.h"
#include "sensors.h"
#include "aprs.h"
#include "microsd.h"
#include "buzzer.h"
#include "leds.h"
#if TRANSMITTER == 1
    #include "satellite.h"
#else
    #include "radio.h"
#endif
#include "lora.h"

SoftwareSerial gps_ss(GPS_RX_PIN, GPS_TX_PIN);

void version() {
    #ifdef DEBUG
        micro_sd.current_file.println(F("Balloon Tracker Starting..."));
        micro_sd.current_file.println(F("Version: 0.1.2"));
        micro_sd.current_file.print(F("Sketch: "));
        micro_sd.current_file.println(__FILE__);
        micro_sd.current_file.print(F("Uploaded: "));
        micro_sd.current_file.println(__DATE__);
        micro_sd.current_file.println();
        micro_sd.current_file.flush();
    #endif
}

void module_setup() {
    micro_sd.setup_handler();

    gps.setup_handler(&gps_ss);
    sensors.setup_handler();
    aprs_object.setup_handler();
    buzzer.setup_handler();
    leds.setup_handler();
    #if TRANSMITTER == 1
        satellite.setup_handler();
    #else
        radio.setup_handler();
    #endif
    lora_object.setup_handler();
}

void setup() {
    // initialize modules
    module_setup();

    // print version info
    version();

    // setup SPI CS Pins
    pinMode(LORA_CS_PIN, OUTPUT);
    pinMode(SD_CS_PIN, OUTPUT);
    chip_select_sd();

    // print memory info
    print_free_mem();
}

void loop() {
    // get new gps data
    micro_sd.current_file.println("Grabbing GPS...");
    gps.loop_handler();

    // get new sensor data
    micro_sd.current_file.println("Grabbing sensor data...");
    sensors.loop_handler();

    // process gps data into packets
    micro_sd.current_file.println("Making APRS packet...");
    aprs_object.loop_handler();

    // log data to sd card
    micro_sd.current_file.println("Saving to SD card...");
    micro_sd.loop_handler();

    // play buzzer for statuses
    micro_sd.current_file.println("Playing buzzer...");
    buzzer.loop_handler();

    // update leds for statuses
    micro_sd.current_file.println("Updating LEDs...");
    leds.loop_handler();

    // transmit data over lora
    micro_sd.current_file.println("Transmitting LoRa...");
    lora_object.loop_handler();

    #if TRANSMITTER == 1
        // transmit data over satellite
        satellite.loop_handler();
    #else
        // transmit data over radio
        radio.loop_handler();
    #endif

    micro_sd.current_file.println("Time to repeat!");
    micro_sd.current_file.flush();
}