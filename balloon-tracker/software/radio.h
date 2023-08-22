#pragma once

#define _1200   1
#define _2400   0

// packet length is 90 bytes max for our use
// this is not the best way to do this but it works
// make 100 to roughly account for bit stuffing
// bad way of doing it
#define max_packet_len  100

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

        // packet buffers
        char ax25_header_packet[16]; // always 16 bytes
        unsigned char packet_length = 0;
        char packet[max_packet_len]; // max 100 bytes for simplicity

        // timing
        unsigned long last_transmission = 0;

        // bit banging mic out functions
        void set_nada_1200();
        void set_nada_2400();
        void set_nada(bool &nada);

        // send functions
        void send_flag(int flag_len);
        void send_packet();

        // build crc
        unsigned short int crc16_ccitt();

        // hdlc generation functions
        void flip_order();
        void raw_hdlc();
        void stuff_hdlc_packet();
        void build_hdlc_packet();

        // ax25 generation functions
        void ax25_call_sign(char *output, bool is_source);
        unsigned char ax25_ssid(bool is_source);
        void ax25_header();
        void build_ax25_packet();

        // main packet builder
        void build_aprs_packet();

        // radio functions
        void init_radio();
        void reset_radio();
        void set_frequency();
        void set_filter();
        void read_radio();
    public:
        Radio();
        void setup_handler();
        void loop_handler();
};

extern Radio radio;