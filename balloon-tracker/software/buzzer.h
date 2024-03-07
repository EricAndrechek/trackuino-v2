#pragma once

enum BuzzerStatus {
    On,
    Lock,
    Transmitting,
    Error
};

class Buzzer {
    private:
        // last time buzzer was on
        unsigned long last_buzz;
        // last state of buzzer
        bool last_buzz_state;
        // number of buzzes remaining
        char buzzes_remaining;
        // status of buzzer
        BuzzerStatus buzzer_status;
        // status to be transmitted
        BuzzerStatus queued_status;

        void _start_buzz();
        void _stop_buzz();
    public:
        Buzzer();
        void setup_handler();
        void loop_handler();

        void set_status(BuzzerStatus status);
};

extern Buzzer buzzer;