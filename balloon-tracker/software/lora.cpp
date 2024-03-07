// We are using SandeepMistry's LoRa library instead of the RadioHead library
// Reasoning: https://github.com/travisgoodspeed/loraham/issues/19#issuecomment-347428458

#include <SoftwareSerial.h>
#include "Arduino.h"
#include <LoRa.h>
#include <TinyGPSMinus.h>

#include "config.h"
#include "helpers.h"
#include "lora.h"
#include "gps.h"
#include "aprs.h"
#include "leds.h"
#include "microsd.h"

Lora_class::Lora_class() {
    last_lora = 0;
}

void Lora_class::setup_handler() {
    chip_select_lora();

    LoRa.setPins(LORA_CS_PIN, LORA_RESET_PIN, LORA_DIO0_PIN);
    
    if (!LoRa.begin(LORA_FREQ)) {
        #ifdef DEBUG
            chip_select_sd();
            micro_sd.current_file.println(F("LoRa module failed to initialize"));
            micro_sd.current_file.flush();
            chip_select_lora();
        #endif
        leds.set_error();

        // kill program
        while (1);
    }

    LoRa.setTxPower(LORA_TX_POWER);
    LoRa.setSpreadingFactor(LORA_SPREADING_FACTOR);
    LoRa.setSignalBandwidth(LORA_BANDWIDTH);
    LoRa.setCodingRate4(LORA_CODING_RATE);
    // LoRa.setSyncWord(LORA_SYNC_WORD);
    // LoRa.enableCrc();
    LoRa.idle();

    chip_select_sd();

    log_init(__FILE__, sizeof(Lora_class));
}

void Lora_class::loop_handler() {
    // check if GPS is stale and if we are above altitude threshold
    // multiply by 100 to convert to centimeters
    if (!gps.stale && gps.tinygps_object.altitude() >= (LORA_ALTITUDE * 100)) {
        // we are above altitude threshold, so we should not broadcast
        // set LoRa into sleep/low power mode
        // TODO: figure out best way of doing this without a reset or 
        // interrupt pin
        return;
    } else if (!gps.stale && gps.tinygps_object.altitude() < (LORA_ALTITUDE * 100)) {
        // TODO: maybe make shutoff altitude higher than startup altitude
        // so that we don't turn on and off too often

        // we are below altitude threshold
        // set LoRa into standby mode
        LoRa.idle();
    }

    // check if it is time to broadcast
    if ((millis() - last_lora) / 1000 >= (LORA_INTERVAL - (SLOT_ERROR / 2))) {
        // check again to make sure LORA_SLOT is good
        // get current seconds, subtract LORA_SLOT,
        // and check if it's divisible by LORA_INTERVAL
        // if it's not divisible, we're not in a good slot

        // need margin of +/- SLOT_ERROR seconds to account for drift
        if (!gps.stale && (gps.second - LORA_SLOT) % LORA_INTERVAL < SLOT_ERROR) {
            // we're in a good slot, broadcast
            broadcast();
        } else if (gps.stale) {
            // GPS is stale but it has been about LORA_INTERVAL seconds
            // we want to broadcast that we are still alive, so broadcast anyway
            broadcast();
            return;
        } else {
            // we're not in a good slot, wait until next loop
            return;
        }
    }
}

void Lora_class::broadcast() {
    // then send it over LoRa
    chip_select_lora();
    LoRa.beginPacket();
    LoRa.print(aprs_object.packet);
    LoRa.endPacket(false);
    chip_select_sd();

    // 

    // TODO: sleep between broadcasts to save power since all are known lengths of time
    // apart. May be possible to put into deep sleep without needing to reset or 
    // reconfigure SPI pins and without needing an interrupt pin due to known sleep time
}

Lora_class lora_object = Lora_class();