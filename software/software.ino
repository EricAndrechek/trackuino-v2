// Refuse to compile on arduino version 21 or lower. 22 includes an
// optimization of the USART code that is critical for real-time operation
// of the AVR code.
// Plus 0 hack is to detect non-Arduino compilers that define ARDUINO as an empty macro
#if (ARDUINO + 0) < 22
#error "Oops! We need Arduino 22 or 23"
#error "See trackuino.ino for details on this"
#endif


// Trackuino custom libs
#include "config.h"
#include "power.h"
#include "pin.h"
#include "sensors_avr.h"

#include <Arduino.h>
#include <SoftwareSerial.h>

// Module libs
#ifdef APRS_MODULE
#include "aprs.h"
#endif

#ifdef BUZZER_MODULE
#include "buzzer.h"
#endif

#ifdef GPS_MODULE
#include "gps.h"
SoftwareSerial GPS_Serial(GPS_RX, GPS_TX);
#endif

#ifdef GSM_MODULE
#include "gsm.h"
#endif

#ifdef SD_MODULE
#include "sd.h"
SoftwareSerial SD_Serial(SD_RX, SD_TX);
#endif

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pin_write(LED_PIN, LOW);

  Serial.begin(SERIAL_BAUDRATE);
#ifdef GPS_MODULE
  gps_setup();
  GPS_Serial.begin(GPS_BAUDRATE);
#endif
#ifdef SD_MODULE
  sd_setup();
  SD_Serial.begin(SD_BAUDRATE);
#endif
#ifdef GSM_MODULE
  gsm_setup();
#endif
#ifdef BUZZER_MODULE
  buzzer_setup();
#endif
#ifdef APRS_MODULE
  aprs_setup();
#endif
#ifdef RESET_DEBUG
  Serial.listen();
  Serial.println("RESET");
#endif
  sensors_setup();

#ifdef SENSOR_DEBUG
  // do some Serial.prints for each sensor value
  Serial.listen();  
#endif



  // Do not start until we get a valid time reference
  // for slotted transmissions.
  if (LOG_SLOT >= 0) {
    do {
      while (!GPS_Serial.available()) {
        power_save();
      }
    } while (!gps_decode(GPS_Serial.read()));

    next_aprs = millis() + 1000 * (APRS_PERIOD - (gps_seconds + APRS_PERIOD - APRS_SLOT) % APRS_PERIOD);
  } else {
    next_aprs = millis();
  }

  
  // TODO: beep while we get a fix, maybe indicating the number of
  // visible satellites by a series of short beeps?
}

void get_pos() {
  // Get a valid position from the GPS
  int valid_pos = 0;
  uint32_t timeout = millis();

#ifdef GPS_DEBUG
  Serial.listen();
  Serial.println("\nget_pos()");
#endif

  gps_reset_parser();

  GPS_Serial.listen();

  do {
    if (GPS_Serial.available()) {
      valid_pos = gps_decode(GPS_Serial.read());
    }
  } while ((millis() - timeout < VALID_POS_TIMEOUT) && !valid_pos);

  if (valid_pos) {
    if (gps_altitude > BUZZER_ALTITUDE) {
      buzzer_off();  // In space, no one can hear you buzz
    } else {
      buzzer_on();
    }
  }
}

void loop() {
  // Time for another APRS frame
  if ((int32_t)(millis() - next_aprs) >= 0) {
    get_pos();
    aprs_send();
    next_aprs += APRS_PERIOD * 1000L;
    while (afsk_flush()) {
      // TODO: strangely seems to call afsk_flush() twice
      power_save();
    }

#ifdef DEBUG_MODEM
    // Show modem ISR stats from the previous transmission
    afsk_debug();
#endif

  } else {
    // Discard GPS data received during sleep window
    while (Serial.available()) {
      Serial.read();
    }
  }

  power_save();  // Incoming GPS data or interrupts will wake us up
}