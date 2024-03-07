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
        bool GPS_LED_state;
        bool ERROR_LED_state;
        bool STATUS_LED_state;

        // last time LEDs were updated
        unsigned long last_ERROR_LED;
        unsigned long last_STATUS_LED;

        char STATUS_LED_flashes_remaining;
        STATUS STATUS_LED_status;
        STATUS STATUS_LED_queued_status;
    public:
        LEDS();
        void setup_handler();
        void loop_handler();

        void set_error();
        void set_status(STATUS status);
};

extern LEDS leds;