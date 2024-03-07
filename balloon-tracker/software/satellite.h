#pragma once

class Satellite {
    private:
        unsigned long last_transmission;
        unsigned char last_MO_status;
        unsigned char packet_write_errors;

        void get_MO_status();
        void write_packet();
        void send_packet();
    public:
        Satellite();
        void setup_handler();
        void loop_handler();
};

extern Satellite satellite;