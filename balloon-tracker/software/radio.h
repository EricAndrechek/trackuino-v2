#pragma once

#define _1200   1
#define _2400   0

struct packet {
    unsigned char len;
    char* packet;
};

class Radio {
    private:
        // globals
        const unsigned char FLAG = 0x7e;
        const unsigned char PID = 0xf0;
        const unsigned char CF = 0x03;
        const unsigned int tc1200 = (unsigned int)(0.5 * 0.975 * 1000000.0 / 1200.0);
        const unsigned int tc2400 = (unsigned int)(0.5 * 0.975 * 1000000.0 / 2400.0);

        unsigned short crc=0xffff;
        char bit_stuff = 0;
        bool nada = _2400;

        void set_nada_1200();
        void set_nada_2400();
        void set_nada(bool nada);

        void send_packet(packet pack);
        void send_flag(unsigned char flag_len);
        void send_header(char msg_type);
        void send_payload(char type);

        void set_io(void);
        void print_debug(char type);
    public:
        Radio();
        void setup_handler();
        void loop_handler();
};

extern Radio radio;