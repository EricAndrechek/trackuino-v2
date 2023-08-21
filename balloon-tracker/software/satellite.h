#pragma once

class Satellite {
    private:
        unsigned long last_transmission = 0;
        unsigned char last_MO_status = 5; // 5 is reserved, we will init to it.
        unsigned char packet_write_errors = 0;

        void get_MO_status();
        void write_packet();
        void send_packet();
    public:
        Satellite();
        void setup_handler();
        void loop_handler();
};

extern Satellite satellite;