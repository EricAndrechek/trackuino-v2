#pragma once

enum STATUS {
    ON,
    SAVING,
    TRANSMITTING,
    ERROR
};

class LEDS {
    private:
        // false = off, true = on
        bool GPS_LED_state = false;
        bool ERROR_LED_state = false;
        bool STATUS_LED_state = false;

        // last time LEDs were updated
        unsigned long last_ERROR_LED = 0;
        unsigned long last_STATUS_LED = 0;

        char STATUS_LED_flashes_remaining = 0;
        STATUS STATUS_LED_status = STATUS::ON;
        STATUS STATUS_LED_queued_status = STATUS::ON;
    public:
        LEDS();
        void setup_handler();
        void loop_handler();

        void set_error();
        void set_status(STATUS status);
};

extern LEDS leds;