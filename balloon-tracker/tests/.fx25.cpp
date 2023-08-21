// Our implementation of the FX.25 protocol
// Takes telemetry data and config parameters
// Turns it into APRS format, then AX.25 packet
// Then adds HDLC framing and FEC tags to create FX.25 packet

// This implementation does not support digipeating
// and is only for sending packets
// It does not support MIC-E encoding
// It does not support sending multiple packets in one FX.25 packet

#include <iostream>
#include <string>

using namespace std;

// globals
unsigned char FLAG = 0x7e;
unsigned char PID = 0xf0;
unsigned char CF = 0x03;

// will be defined in config.h
#define check_value 32
#define d_address "TOCALL"
#define d_ssid 1
#define s_address "MYCALL"
#define s_ssid 0

#define DEBUG

// packet buffers
char ax25_header_packet[16];

struct packet {
    unsigned char len;
    char* packet;
};


#ifdef DEBUG
    // DEBUG HELPER FUNCTIONS
    void print_hex(char* packet, unsigned char length) {
        for (int i = 0; i < length; i++) {
            printf("%02x ", packet[i] & 0xff);
        }
        printf("\n");
    }
 #endif

// FX.25 CODE

class FEC_Tag {
    public:
        FEC_Tag(const char tag) {
            this->tag = tag;
            switch (tag)
            {
            case 1:
                this->data = 239;
                this->check = 16;
                this->tag_value = 0xB74DB7DF8A532F3E;
                break;
            case 2:
                this->data = 128;
                this->check = 16;
                this->tag_value = 0x26FF60A600CC8FDE;
                break;
            case 3:
                this->data = 64;
                this->check = 16;
                this->tag_value = 0xC7DC0508F3D9B09E;
                break;
            case 4:
                this->data = 32;
                this->check = 16;
                this->tag_value = 0x8F056EB4369660EE;
                break;
            case 5:
                this->data = 223;
                this->check = 32;
                this->tag_value = 0x6E260B1AC5835FAE;
                break;
            case 6:
                this->data = 128;
                this->check = 32;
                this->tag_value = 0xFF94DC634F1CFF4E;
                break;
            case 7:
                this->data = 64;
                this->check = 32;
                this->tag_value = 0x1EB7B9CDBC09C00E;
                break;
            case 8:
                this->data = 32;
                this->check = 32;
                this->tag_value = 0xDBF869BD2DBB1776;
                break;
            case 9:
                this->data = 191;
                this->check = 64;
                this->tag_value = 0x3ADB0C13DEAE2836;
                break;
            case 0xA:
                this->data = 128;
                this->check = 64;
                this->tag_value = 0xAB69DB6A543188D6;
                break;
            case 0xB:
                this->data = 64;
                this->check = 64;
                this->tag_value = 0x4A4ABEC4A724B796;
                break;
            default:
                this->data = 0;
                this->check = 0;
                this->tag_value = 0;
                break;
            }
        }

        unsigned char tag;
        unsigned char data;
        unsigned char check;
        unsigned long long tag_value;
};

// given ax.25 processed information, find the optimal FEC tag
FEC_Tag opt_pack_tag(unsigned char data_size) {

    #ifdef DEBUG
        printf("AX.25 Info Length: %d\n", data_size - 20);
        printf("Data Size: %d\n", data_size);
    #endif

    // find the smallest tag.data with the matching check value
    // if no matching check value, find the smallest tag.data
    FEC_Tag tag = FEC_Tag(0);
    for (int i = 1; i <= 0xB; i++) {
        FEC_Tag temp = FEC_Tag(i);
        if (temp.check == check_value) {
            if (temp.data >= data_size) {
                if (tag.data == 0 || tag.check != check_value || (tag.check == check_value && temp.data < tag.data)) {
                    tag = temp;
                }
            }
        } else {
            if (temp.data >= data_size) {
                if (tag.data == 0 || (tag.check != check_value && temp.data < tag.data)) {
                    tag = temp;
                }
            }
        }
    }

    #ifdef DEBUG
        printf("Optimal Tag: %d\n", tag.tag);
        printf("Tag Data: %d\n", tag.data);
        printf("Tag Check: %d\n", tag.check);
        printf("Tag Value: %llx\n", tag.tag_value);
        printf("\n");
    #endif

    return tag;
}

packet fx25_padded(FEC_Tag tag, packet hdlc) {
    #ifdef DEBUG
        printf("FX.25 Data Packet Length: %d\n", tag.data);
    #endif

    packet p;

    char* fx_p = new char[tag.data];

    int i = 0;

    // HDLC Packet
    for (; i < hdlc.len; i++) {
        fx_p[i] = hdlc.packet[i];
    }

    // potentially need to fix misaligned bytes

    // PAD
    for (; i < tag.data; i++) {
        fx_p[i] = FLAG;
    }

    p.len = tag.data;
    p.packet = fx_p;

    #ifdef DEBUG
        printf("FX.25 Data Packet: ");
        print_hex(p.packet, p.len);
        printf("\n");
    #endif

    return p;
}

// TODO: implement this correctly for RS FEC
char* fec_check_symbols(FEC_Tag tag, packet fx_padded) {
    #ifdef DEBUG
        printf("FX.25 Check Packet Length: %d\n", tag.check);
    #endif

    char* fec_symbols = new char[tag.check];

    // FEC Check Symbols
    for (int i = 0; i < tag.check; i++) {
        fec_symbols[i] = 0;
    }

    // FEC Check Symbols
    for (int i = 0; i < fx_padded.len; i++) {
        for (int j = 0; j < tag.check; j++) {
            fec_symbols[j] ^= fx_padded.packet[i] & (tag.tag_value >> (8 * (7 - j)));
        }
    }

    #ifdef DEBUG
        printf("FX.25 Check Packet: ");
        print_hex(fec_symbols, tag.check);
        printf("\n");
    #endif

    return fec_symbols;
}

packet fec_codeblock(FEC_Tag tag, packet hdlc) {
    packet p;

    char* fx_p = new char[tag.data + tag.check];

    packet fx_padded = fx25_padded(tag, hdlc);
    char* fec_symbols = fec_check_symbols(tag, fx_padded);

    // HDLC Packet
    for (int j = 0; j < fx_padded.len; j++) {
        fx_p[j] = fx_padded.packet[j];
    }

    // FEC Check Symbols
    for (int j = 0; j < tag.check; j++) {
        fx_p[tag.data + j] = fec_symbols[j];
    }

    p.len = tag.data + tag.check;
    p.packet = fx_p;

    #ifdef DEBUG
        printf("FX.25 Codeblock Length: %d\n", tag.data + tag.check);
        printf("FX.25 Codeblock: ");
        print_hex(p.packet, p.len);
        printf("\n");
    #endif

    return p;
}

packet fx25_packet(FEC_Tag tag, packet hdlc) {
    // two preambles (2 bytes)
    // correlation tag value (8 bytes)
    // FEC codeblock (data + check bytes)
    // two postambles (2 bytes)
    // total: data + check + 12 bytes

    packet p;

    char* fx_p = new char[tag.data + tag.check + 12];

    // Preamble
    fx_p[0] = FLAG;
    fx_p[1] = FLAG;

    // Correlation Tag Value
    int i = 2;
    for (; i < 10; i++) {
        fx_p[i] = (tag.tag_value >> (8 * (9 - i))) & 0xff;
    }

    // FEC Codeblock
    packet fx_codeblock = fec_codeblock(tag, hdlc);
    for (int j = 0; j < fx_codeblock.len; j++) {
        fx_p[i + j] = fx_codeblock.packet[j];
    }

    // Postamble
    fx_p[i + fx_codeblock.len] = FLAG;
    fx_p[i + fx_codeblock.len + 1] = FLAG;

    #ifdef DEBUG
        printf("FX.25 Packet Length: %d\n", tag.data + tag.check + 12);
        printf("FX.25 Packet: ");
        print_hex(fx_p, tag.data + tag.check + 12);
        printf("\n");
    #endif

    p.len = tag.data + tag.check + 12;
    p.packet = fx_p;
    
    return p;
}

// HDLC CODE

char* flipped_order(unsigned char len, char* ax25_packet) {
    #ifdef DEBUG
        printf("Flipped Packet Length: %d\n", len - 4);
    #endif

    // HDLC Packet
    char* packet = new char[len - 4];

    // AX.25 Packet
    int i = 0;
    for (; i < len - 3; i++) {
        // swap bit order (MSB -> LSB)
        packet[i] = 0;
        for (int j = 0; j < 8; j++) {
            packet[i] |= ((ax25_packet[i] >> j) & 0x1) << (7 - j);
        }
    }

    #ifdef DEBUG
        printf("Flipped Packet: ");
        print_hex(packet, len - 4);
        printf("\n");
    #endif

    return packet;
}

unsigned short int crc16_ccitt(const char* packet, const int length) {
    // TODO: this is broken
    // should be 0x13 0x12
    // this gets 0x2e 0x5d
    // arduino gets 0xab 0x75

    unsigned short int crc = 0xffff;

    for (int i = 0; i < length; i++) {
        for (int j = 0; j < 8; j++) {
            unsigned char bit = (packet[i] >> (7 - j)) & 0x1;
            unsigned char c15 = (crc >> 15) & 0x1;
            crc <<= 1;
            if (c15 ^ bit) {
                crc ^= 0x1021;
            }
        }
    }

    crc = crc ^ 0xffff & 0xffff;

    #ifdef DEBUG
        printf("CRC: %04x\n", crc);
    #endif

    return crc;
}

char* raw_hdlc(unsigned char len, char* ax25_packet) {
    ax25_packet = flipped_order(len, ax25_packet);

    #ifdef DEBUG
        printf("Raw HDLC Packet Length: %d\n", len);
    #endif

    // Un-Stuffed HDLC Packet
    char* packet = new char[len];

    // Flag
    packet[0] = FLAG;

    int i = 1;

    // AX.25 Packet
    for (; i < len - 3; i++) {
        packet[i] = ax25_packet[i - 1];
    }

    // CRC
    unsigned short int crc = crc16_ccitt(ax25_packet, len - 3);
    packet[i] = (crc >> 8) & 0xff;
    packet[i + 1] = crc & 0xff;

    // Flag
    packet[i + 2] = FLAG;

    #ifdef DEBUG
        printf("Raw HDLC Packet: ");
        print_hex(packet, len);
        printf("\n");
    #endif

    return packet;
}

packet stuffed_hdlc_packet(unsigned char len, char* raw_hdlc_packet) {
    packet p;

    p.len = len;
    p.packet = new char[len];

    // add flag
    p.packet[0] = FLAG;

    unsigned short int bit_pos = 0;
    unsigned char stuffing = 0;
    for (int i = 1; i < len; i++) {
        // printf("i: %d v: %02x\n", i, raw_hdlc_packet[i] & 0xff);
        for (int j = 0; j < 8; j++) {
            unsigned char bit = (raw_hdlc_packet[i] >> (7 - j)) & 0x1;
            if (bit == 0) {
                stuffing = 0;
            } else if (i != len - 1) {
                stuffing++;
            }

            // printf("index: %d bit_pos: %d\n", i + ((j + bit_pos) / 8), 7 - ((j + bit_pos) % 8));
            p.packet[i + ((j + bit_pos) / 8)] |= bit << (7 - ((j + bit_pos) % 8));

            if (stuffing == 5) {
                bit_pos++;
                stuffing = 0;
                // printf("[STUFF] index: %d bit_pos: %d\n", i + ((j + bit_pos) / 8), 7 - ((j + bit_pos) % 8));
                p.packet[i + ((j + bit_pos) / 8)] |= 0x0 << (7 - ((j + bit_pos) % 8));
            }
        }
    }

    // add to length the number of bytes added by stuffing
    p.len += bit_pos / 8 + (bit_pos % 8 != 0);

    #ifdef DEBUG
        printf("Stuffed HDLC Packet Length: %d\n", p.len);
        printf("Stuffed HDLC Packet: ");
        print_hex(p.packet, p.len);
        printf("\n");
    #endif

    return p;
}

packet hdlc_packet(unsigned char len, char* ax25_packet) {
    char* raw_packet = raw_hdlc(len, ax25_packet);
    packet p = stuffed_hdlc_packet(len, raw_packet);
    delete[] raw_packet;
    return p;
}

// AX.25 HEADER W/ HLDC ADDRESS FIELD CODE

// takes a callsign and returns it in AX.25 format
// (uppercase, padded with spaces, shifted left by 1 bit)
char* ax25_call_sign(string call_sign, bool is_source) {
    char* ax25_call_sign = new char[7];

    // ensure we only get the first 6 characters
    // add spaces if less than 6
    if (call_sign.length() > 6) {
        call_sign = call_sign.substr(0, 6);
    } else if (call_sign.length() < 6) {
        while (call_sign.length() < 6) {
            call_sign += " ";
        }
    }

    // uppercase
    for (int i = 0; i < call_sign.length(); i++) {
        call_sign[i] = toupper(call_sign[i]);
    }

    // shift left by 1 bit
    for (int i = 0; i < call_sign.length(); i++) {
        ax25_call_sign[i] = call_sign[i] << 1;
        // set the last bit to 0 if we're a source
        // and 1 if we're a destination
        ax25_call_sign[i] |= is_source ? 0x0 : 0x0;
    }

    // add null terminator
    ax25_call_sign[6] = '\0';

    return ax25_call_sign;
}

// takes an ssid and returns in AX.25 format
unsigned char ax25_ssid(unsigned char ssid, bool is_source) {
    // ax.25 ssid format:
    // CRRSSID0
    // C = Command/Response
    // R = Reserved (should be 1)
    // SSID = 4-bit SSID
    // whether or not last octet of HDLC address field

    // we always have a command packet
    // so set the C bit to 0 when we're a source
    // and 1 when we're a destination

    // ensure we only get the first 4 bits
    // ssid can be 0-15
    ssid &= 0xf;
    // now should look like xxxxSSID

    // shift left by 1 bit
    ssid <<= 1;
    // now should look like xxxSSIDx

    // set the R bits to 1
    ssid |= 0x60;
    // now should look like x11SSIDx

    // set the C bit
    ssid |= (is_source ? 0x0 : 0x80);
    // now should look like C11SSIDx

    // set the last bit to 1 if source, 0 if destination
    ssid |= (is_source ? 0x1 : 0x0);

    return ssid;
}

// builds global ax25_header_packet
void ax25_header() {
    #ifdef DEBUG
        printf("AX.25 Header Packet Length: 16\n");
    #endif

    char* dest_addy = ax25_call_sign(d_address, false);
    char* sour_addy = ax25_call_sign(s_address, true);
    for (int i = 0; i < 6; i++) {
        // ensure we only get the first 7 bits of each character
        ax25_header_packet[i] = dest_addy[i] & 0xff;
    }
    ax25_header_packet[6] = ax25_ssid(d_ssid, false);
    for (int i = 0; i < 6; i++) {
        // ensure we only get the first 7 bits of each character
        ax25_header_packet[i + 7] = sour_addy[i] & 0xff;
    }
    ax25_header_packet[13] = ax25_ssid(s_ssid, true);
    // TODO: AX.25 4.2.1 bit lengths
    ax25_header_packet[14] = CF;
    ax25_header_packet[15] = PID;

    #ifdef DEBUG
        printf("AX.25 Header: ");
        print_hex(ax25_header_packet, 16);
        printf("\n");
    #endif

    // TODO: CRC for AX.25 header

    delete[] dest_addy;
    delete[] sour_addy;
}

// AX.25 CODE

char* ax25_packet(unsigned char len, string info) {
    #ifdef DEBUG
        printf("AX.25 Packet Length: %d\n", len - 4);
    #endif

    char* packet = new char[len - 4];

    // AX.25 Header
    int i = 0;
    for (; i < 16; i++) {
        packet[i] = ax25_header_packet[i];
    }

    // Info Field
    int j = 0;
    for (; j < info.length(); j++) {
        packet[i + j] = info[j];
    }

    #ifdef DEBUG
        printf("AX.25 Packet: ");
        print_hex(packet, len - 4);
        printf("\n");
    #endif

    return packet;
}

// APRS CODE

// take a total length and pre-packeted info
// and return a FX.25 packet
packet aprs_packet(string info) {
    // flag = 1 byte
    // header = 16 bytes
    // info = info.length() bytes
    // fcs = 2 bytes
    // flag = 1 byte
    // total = info.length() + 20 bytes
    
    unsigned char len = info.length() + 20;
    char* ax25 = ax25_packet(len, info);
    packet hdlc = hdlc_packet(len, ax25);
    // FEC_Tag tag = opt_pack_tag(hdlc.len);
    // packet fx25 = fx25_packet(tag, hdlc);
    return hdlc;
}

// takes telemetry data and comments
// and returns a formatted APRS message
string format_info(string telemetry, string comment) {
    string info = "T" + telemetry + " " + comment;

    #ifdef DEBUG
        printf("Info Field: %s\n", info.c_str());
        printf("\n");
    #endif

    return info;
}

// TODO: something to turn telemetry data into a string

// MAIN

void init() {
    // addresses and ssid's will never change
    ax25_header();
}

int main() {
    init();

    string telemetry = "12345678";
    string comment = "Hello World!";

    string info = format_info(telemetry, comment);

    packet pack = aprs_packet(info);

    printf("Packet: ");
    for (int i = 0; i < pack.len; i++) {
        printf("%02x ", pack.packet[i] & 0xff);
    }

    printf("Packet: \n");
    for (int i = 0; i < pack.len; i++) {
        printf("%02x\n", pack.packet[i] & 0xff);
    }
    printf("\n");
}