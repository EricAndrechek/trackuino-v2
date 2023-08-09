// Path: software/sd.cpp
// Description: This module handles the SD card. It can be used to log data to the SD card.

// include libraries
#include <SPI.h>
#include <SD.h>

// include configuration
#include "config.h"
#include "variables.h"

// helper functions
void sd_setup();
Serial& _sd_serial();

// SD VARIABLES

// only allocate memory if SD module is enabled
#ifdef SD_MODULE

    // SD card chip select pin
    const int SD_CHIP_SELECT = 10;

    // SD card serial port
    SoftwareSerial sd_serial(SD_RX, SD_TX);

    // SD card file
    File sd_file;

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

// 
void sd_loop() {
}