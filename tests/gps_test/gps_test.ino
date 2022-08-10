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

#include <Arduino.h>
#include <SoftwareSerial.h>

#ifdef GPS_MODULE
#include "gps.h"
SoftwareSerial GPS_Serial(GPS_RX, GPS_TX);
#endif

static int32_t next_data = 0;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pin_write(LED_PIN, LOW);

  Serial.begin(SERIAL_BAUDRATE);

#ifdef GPS_MODULE
  gps_setup();
  GPS_Serial.begin(GPS_BAUDRATE);
#endif
#ifdef RESET_DEBUG
  Serial.println("RESET");
#endif

#ifndef GPS_MODULE
#define LOG_SLOT -1
#endif

  // Do not start until we get a valid time reference
  // for slotted transmissions.
  if (LOG_SLOT >= 0) {
    do {
      while (!GPS_Serial.available()) {
        power_save();
      }
    } while (!gps_decode(GPS_Serial.read()));

    // TODO: use GSM for time if GPS not available

    next_data = millis() + 1000 * (DATA_TIMEOUT - (gps_seconds + DATA_TIMEOUT - LOG_SLOT) % DATA_TIMEOUT);
  } else {
    next_data = millis();
  }


  // TODO: beep while we get a fix, maybe indicating the number of
  // visible satellites by a series of short beeps?
}

void get_pos() {
  // Get a valid position from the GPS
  int valid_pos = 0;
  uint32_t timeout = millis();

#ifdef GPS_DEBUG
  Serial.println("\nget_pos()");
#endif

  gps_reset_parser();

  GPS_Serial.listen();

  do {
    if (GPS_Serial.available()) {
      valid_pos = gps_decode(GPS_Serial.read());
    }
  } while ((millis() - timeout < 2000) && !valid_pos);
}

void loop() {
  // Time for another APRS frame
  if ((int32_t)(millis() - next_data) >= 0) {
    get_pos();
    Serial.println("Done GPS Parse");
    Serial.println(gps_seconds);
    Serial.println(gps_lat);
    Serial.println(gps_lon);
    Serial.println(gps_speed);
    Serial.println(gps_altitude);
    next_data += DATA_TIMEOUT * 1000L;
  } else {
    // Discard GPS data received during sleep window
    while (GPS_Serial.available()) {
      GPS_Serial.read();
    }
  }

  power_save();  // Incoming GPS data or interrupts will wake us up
}