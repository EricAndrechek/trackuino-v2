#include <SoftwareSerial.h>
#include "Arduino.h"

#include "config.h"
#include "helpers.h"
#include "radio.h"

// take an APRS packet and turns it into an AX.25 packet
// Then adds HDLC framing

// Note: this code reverses the bit order as per protocol, 
// however it does it in a less efficient way that doing so
// during transmission. This is intentional as it is needed
// for adding in forward error correction. FEC has been disabled
// (see .fec.cpp) so it is not really needed, but is left in in
// the event we can get FEC to run more efficiently.

// This implementation does not support digipeating
// and is only for sending packets
// It does not support MIC-E encoding
// It does not support sending multiple packets in one FX.25 packet


// DEBUG HELPER FUNCTIONS
#ifdef DEBUG

    // print a packet in hex
    void print_hex(char* packet, unsigned char length) {
        for (int i = 0; i < length; i++) {
            Serial.print("0x");
            Serial.print((packet[i] & 0xff) < 16 ? "0" : "");
            Serial.print(packet[i] & 0xff, HEX);
            Serial.print(" ");
        }
        Serial.println();
    }

    // print a long that is too big for Serial.print
    // this is used only really for FEC stuff
    void print_unsigned_long_long(unsigned long long val) {
        if (val < 10) {
            Serial.print((unsigned int) val);
        } else {
            unsigned long long subproblem = val / 10;
            int remainder = val % 10;
            print_unsigned_long_long(subproblem);
            Serial.print(remainder);
        }
    }

 #endif


// RADIO CODE

// send 1200hz tone
void Radio::set_nada_1200() {
    digitalWrite(MIC_PIN, HIGH);
    delayMicroseconds(tc1200);
    digitalWrite(MIC_PIN, LOW);
    delayMicroseconds(tc1200);
}

// send 2400hz tone
void Radio::set_nada_2400() {
    digitalWrite(MIC_PIN, HIGH);
    delayMicroseconds(tc2400);
    digitalWrite(MIC_PIN, LOW);
    delayMicroseconds(tc2400);
    
    digitalWrite(MIC_PIN, HIGH);
    delayMicroseconds(tc2400);
    digitalWrite(MIC_PIN, LOW);
    delayMicroseconds(tc2400);
}

// send 1200hz or 2400hz tone
void Radio::set_nada(bool &nada) {
    if (nada) {
        set_nada_1200();
    } else {
        set_nada_2400();
    }
}


// PACKET HELPERS

// send the flag a bunch of times to initialize
void Radio::send_flag(int flags) {
    for (; flags >= 0; flags--) {
        for (char j = 0; j < 8; j++) {
            if (FLAG & (1 << j)) {
                set_nada(nada);
            } else {
                nada ^= 1;
                set_nada(nada);
            }
        }
    }
}

void Radio::send_packet(packet pack) {
    digitalWrite(PTT_PIN, LOW);
    delay(300);
    send_flag(100);
    for (unsigned int i = 0; i < pack.len; i++) {
        for (char j = 7; j >= 0; j--) {
            if (((pack.packet[i] & 0xff) >> j) & 0x01) {
                set_nada(nada);
            } else {
                nada ^= 1;
                set_nada(nada);
            }
        }
    }
    send_flag(3);
    #ifndef NO_HAM
        digitalWrite(_PTT, HIGH);
    #endif
    digitalWrite(LED_BUILTIN, LOW);
}

// HDLC CODE

// calculate the CRC-16-CCITT of a packet
unsigned short int Radio::crc16_ccitt(const char* packet, const int length) {
    // TODO: ESP32 ROM built in and faster
    // check other implementations for speed on other boards

    unsigned short int crc = 0xffff;

    unsigned short int crc16_table[] = {
        0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
        0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6, 0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
        0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485, 0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
        0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4, 0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
        0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823, 0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
        0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12, 0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
        0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41, 0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
        0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70, 0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
        0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F, 0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
        0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E, 0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
        0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D, 0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
        0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C, 0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
        0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB, 0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
        0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A, 0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
        0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9, 0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
        0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8, 0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
    };

    for (int i = 0; i < length; i++) {
        unsigned short int j = (packet[i] ^ (crc >> 8)) & 0xff;
        crc = crc16_table[j] ^ (crc << 8);
    }

    crc = ((crc ^ 0xffff) & 0xffff);

    #ifdef DEBUG
        Serial.print(F("CRC: 0x"));
        Serial.print(crc < 16 ? "0" : "");
        Serial.println(crc, HEX);
    #endif

    return crc;
}

// takes a packet and returns it with the bit order flipped
char* Radio::flipped_order(unsigned char len, char* ax25_packet) {
    #ifdef DEBUG
        Serial.print(F("Flipped Packet Length: "));
        Serial.println(len - 4);
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
        Serial.println(F("Flipped Packet: "));
        print_hex(packet, len - 4);
        Serial.println();
    #endif

    return packet;
}

// takes an AX.25 packet and returns it with HDLC framing
char* Radio::raw_hdlc(unsigned char len, char* ax25_packet) {
    ax25_packet = flipped_order(len, ax25_packet);

    #ifdef DEBUG
        Serial.print(F("Raw HDLC Packet Length: "));
        Serial.println(len);
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
    unsigned short int crc = crc16_ccitt(ax25_packet, len - 4);
    packet[i] = (crc >> 8) & 0xff;
    packet[i + 1] = crc & 0xff;

    // TODO: might need more flags?

    // Flag
    packet[i + 2] = FLAG;

    #ifdef DEBUG
        Serial.println("Raw HDLC Packet: ");
        print_hex(packet, len);
        Serial.println();
    #endif

    return packet;
}

// takes a raw HDLC packet and returns it with bit stuffing
packet Radio::stuffed_hdlc_packet(unsigned char len, char* raw_hdlc_packet) {
    packet p;

    p.len = len;
    char* packet = new char[len];
    // set all bits to 0
    for (int i = 0; i < len; i++) {
        packet[i] = 0;
    }

    // add flag
    packet[0] = FLAG;

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

            // printf("index: %d bit_pos: %d bit: %d\n", i + ((j + bit_pos) / 8), 7 - ((j + bit_pos) % 8), bit);
            packet[i + ((j + bit_pos) / 8)] |= bit << (7 - ((j + bit_pos) % 8));

            if (stuffing == 5) {
                bit_pos++;
                stuffing = 0;
                // printf("[STUFF] index: %d bit_pos: %d bit: %d\n", i + ((j + bit_pos) / 8), 7 - ((j + bit_pos) % 8), bit);
                packet[i + ((j + bit_pos) / 8)] |= 0x0 << (7 - ((j + bit_pos) % 8));
            }
        }
    }

    // TODO: unclear what stuffing offset is supposed to do to final flag transmission

    // add to length the number of bytes added by stuffing
    p.len += bit_pos / 8 + (bit_pos % 8 != 0);
    p.packet = packet;

    #ifdef DEBUG
        Serial.print(F("Stuffed HDLC Packet Length: "));
        Serial.println(p.len);
        Serial.println(F("Stuffed HDLC Packet: "));
        print_hex(p.packet, p.len);
        Serial.println();
    #endif

    return p;
}

// takes an AX.25 packet and returns it with HDLC framing and bit stuffing
packet Radio::hdlc_packet(unsigned char len, char* ax25_packet) {
    char* raw_packet = raw_hdlc(len, ax25_packet);
    print_free_mem();
    packet p = stuffed_hdlc_packet(len, raw_packet);
    print_free_mem();
    delete[] raw_packet;
    return p;
}

// AX.25 HEADER W/ HLDC ADDRESS FIELD CODE

// takes a callsign and returns it in AX.25 format
// (uppercase, padded with spaces, shifted left by 1 bit)
char* Radio::ax25_call_sign(String call_sign, bool is_source) {
    char* ax25_call_sign = new char[7];

    // ensure we only get the first 6 characters
    // add spaces if less than 6
    if (call_sign.length() > 6) {
        call_sign = call_sign.substring(0, 6);
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
unsigned char Radio::ax25_ssid(unsigned char ssid, bool is_source) {
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
void Radio::ax25_header() {
    #ifdef DEBUG
        Serial.println(F("AX.25 Header Packet Length: 16"));
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
        Serial.println(F("AX.25 Header: "));
        print_hex(ax25_header_packet, 16);
        Serial.println();
    #endif

    // TODO: can CRC be partially built for AX.25 header?

    delete[] dest_addy;
    delete[] sour_addy;
}

// AX.25 CODE

// takes a length and info field and returns an AX.25 packet
char* Radio::ax25_packet(unsigned char len, String info) {
    #ifdef DEBUG
        Serial.print(F("AX.25 Packet Length: "));
        Serial.println(len - 4);
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
        Serial.println(F("AX.25 Packet: "));
        print_hex(packet, len - 4);
        Serial.println();
    #endif

    return packet;
}

// APRS CODE

// take a total length and pre-packeted info
// and return a FX.25 packet
packet Radio::aprs_packet(String info) {
    // flag = 1 byte
    // header = 16 bytes
    // info = info.length() bytes
    // fcs = 2 bytes
    // flag = 1 byte
    // total = info.length() + 20 bytes
    
    unsigned char len = info.length() + 20;
    char* ax25 = ax25_packet(len, info);
    print_free_mem();
    packet hdlc = hdlc_packet(len, ax25);
    print_free_mem();
    return hdlc;

    // this forward error correction (FEC) code
    // to turn things into FX.25 for improved range
    // has been removed. See .fec.cpp for more.

    // FEC_Tag tag = opt_pack_tag(hdlc.len);
    // packet fx25 = fx25_packet(tag, hdlc);
    // return fx25;
}

// takes telemetry data and comments
// and returns a formatted APRS message
String Radio::format_info(String telemetry, String comment) {
    String info = ">T" + telemetry + " " + comment;

    #ifdef DEBUG
        Serial.print(F("Info Field: "));
        Serial.println(info.c_str());
        Serial.println();
    #endif

    return info;
}

// TODO: something to turn telemetry data into a String

// MAIN

void Radio::dorji_init(SoftwareSerial &ser) {
    ser.println(F("AT+DMOCONNECT"));
}

void Radio::dorji_reset(SoftwareSerial &ser) {
    for(char i=0;i<1;i++) {
        Serial.println("sending connect command...");
        ser.println(F("AT+DMOCONNECT"));
    }
}

void Radio::dorji_setfreq(float txf, float rxf, SoftwareSerial &ser) {
    ser.print(F("AT+DMOSETGROUP=0,"));
    ser.print(txf, 4);
    ser.print(',');
    ser.print(rxf, 4);
    ser.println(F(",0000,0,0000"));
}

void Radio::dorji_setfilter(bool emph, bool hpf, bool lpf, SoftwareSerial &ser) {
    ser.print(F("AT+SETFILTER="));
    ser.print(emph);
    ser.print(',');
    ser.print(hpf);
    ser.print(',');
    ser.println(lpf);
}

void Radio::dorji_readback(SoftwareSerial &ser) {
    String d;
    
    unsigned long current_time  = millis();
    while(ser.available() < 1) {
        if ((millis() - current_time) / 100 > 5) {
            Serial.println("Connection to DRA818V failed...");
            break;
        }
    }
    if(ser.available() > 0) {
        d = ser.readString();
        Serial.print(d);
    }
}

void Radio::dorji_close(SoftwareSerial &ser) {
    ser.end();
}

void Radio::setup_handler() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(OUT_PIN, OUTPUT);

    #ifndef NO_HAM
        pinMode(DRJ_RXD, INPUT);
        pinMode(DRJ_TXD, OUTPUT);
        pinMode(_PTT, OUTPUT);
        pinMode(_PD, OUTPUT);
        pinMode(_POW, OUTPUT);

        digitalWrite(_PTT, HIGH);
        digitalWrite(_PD, HIGH);
        digitalWrite(_POW, LOW);
    #endif

    Serial.begin(9600);

    #ifndef NO_HAM
        dorji.begin(9600);
    #endif

    delay(250); // may need delay after pin setup before interfacing
    Serial.println(F("\n\n"));
    Serial.println(F("Pins intialized..."));

    print_free_mem();

    Serial.println(F("Building header..."));
    ax25_header();
    Serial.println(F("Header built."));

    print_free_mem();
    
    #ifndef NO_HAM
        Serial.println(F("Setting up DRA818V module..."));
        dorji_reset(dorji);
        dorji_readback(dorji);
        Serial.println(F("DRA818V reset"));
        // delay(1000); // - may need potential delay after restart
        dorji_setfreq(146.390, 146.390, dorji);
        dorji_readback(dorji);
        Serial.println(F("DRA818V configured"));
        // delay(1000); // - may need potential delay before setting filter
        dorji_setfilter(0,0,0,dorji);
        dorji_readback(dorji);
        Serial.println(F("DRA818V filter on"));
        Serial.println(F("DRA818V setup."));

        Serial.println(' ');

        dorji_close(dorji);
    #endif

    Serial.println(F("Building packet..."));
    String telemetry = "12345678";
    String comment = "Hello World!";

    String info = format_info(telemetry, comment);

    print_free_mem();

    packet pack = aprs_packet(info);

    print_free_mem();

    Serial.println(F("Done Building."));

    delay(500);

    Serial.println(F("Sending...\n"));

    send_packet(pack);

    Serial.println(F("\nSent."));
}

void Radio::loop_handler() {
    
}