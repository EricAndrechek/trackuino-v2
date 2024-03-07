
#include "Arduino.h"

// include configuration
#include "config.h"
#include "helpers.h"
#include "sensors.h"

// define helper functions

Sensors::Sensors() {
    temperature = 0.0;
    voltage = 0.0;

    last_sensors = 0;
}

void Sensors::setup_handler() {
    // don't need to do anything to initialize analog sensors

    log_init(__FILE__, sizeof(Sensors));
}

// take analog pin and read and convert to voltage
float analog_to_voltage(int analog_pin_number) {
    return analogRead(analog_pin_number) * 5.0 / 1023.0;
}

void Sensors::loop_handler() {
    // read temp analog pin and apply calibration curve
    temperature = (analog_to_voltage(TEMP_PIN) + TEMP_OFFSET) * TEMP_SLOPE;

    // read voltage analog pin and solve for voltage divider
    voltage = analog_to_voltage(VOLTAGE_PIN) * (VOLTAGE_R1 + VOLTAGE_R2) / VOLTAGE_R2;
}

Sensors sensors = Sensors();