#pragma once

class APRS {
    private:
        char header[18];
        unsigned char sequence = 0;

        void build_header();
        void build_uncompressed_body(char* body, char* hms, char* telemetry);
        void build_compressed_body(char* body, char* hms, char* telemetry);
        void build_telemetry(char* telemetry);
        void base91_encode(unsigned long& input, char* output, int length);
        void base91_encode(unsigned char& input, char* output, int length);
        void base91_encode(float& input, char* output, int length);
    public:
        char *packet;
        APRS();
        ~APRS();
        void loop_handler();
};

extern APRS aprs_object;