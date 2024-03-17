#include "Arduino.h"

#include "config.h"
#include "helpers.h"
#include "radio.h"
#include "leds.h"
#include "aprs.h"
#include "gps.h"
#include "microsd.h"

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

void Radio::send_packet() {
    digitalWrite(PTT_PIN, LOW);
    delay(300);
    send_flag(100);
    for (unsigned int i = 0; i < packet_length; i++) {
        for (char j = 7; j >= 0; j--) {
            if (((packet[i] & 0xff) >> j) & 0x01) {
                set_nada(nada);
            } else {
                nada ^= 1;
                set_nada(nada);
            }
        }
    }
    send_flag(3);
    digitalWrite(PTT_PIN, HIGH);
}

// HDLC CODE

// calculate the CRC-16-CCITT of a packet
unsigned short int Radio::crc16_ccitt() {
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

    for (unsigned char i = 0; i < aprs_object.info_len; i++) {
        unsigned short int j = (packet[17 + i] ^ (crc >> 8)) & 0xff;
        crc = crc16_table[j] ^ (crc << 8);
    }

    crc = ((crc ^ 0xffff) & 0xffff);

    return crc;
}

// takes a packet and re-builds it with the bit order flipped
void Radio::flip_order() {

    // first byte is empty for flag, ignore
    unsigned char i = 1;
    for (; i < max_packet_len; i++) {
        // swap bit order (MSB -> LSB)
        char raw_byte = packet[i];
        packet[i] = 0;
        for (char j = 0; j < 8; j++) {
            packet[i] |= ((raw_byte >> j) & 0x1) << (7 - j);
        }
    }
}

// takes an AX.25 packet and re-builds it with HDLC framing
void Radio::raw_hdlc() {
    flip_order();

    // Flag
    packet[0] = FLAG;

    // CRC
    unsigned short int crc = crc16_ccitt();
    packet[aprs_object.info_len + 17] = (crc >> 8) & 0xff;
    packet[aprs_object.info_len + 17 + 1] = crc & 0xff;

    // TODO: might need more flags?

    // Flag
    packet[aprs_object.info_len + 17 + 2] = FLAG;
}

// takes a raw HDLC packet and returns it with bit stuffing
void Radio::stuff_hdlc_packet() {

    // TODO: very bad way of doing this...
    // wastes lots of memory and makes unsafe assumptions
    char stuffed_packet[max_packet_len];

    // set all bits to 0
    // cannot assume they already are
    for (unsigned char i = 0; i < max_packet_len; i++) {
        stuffed_packet[i] = 0;
    }

    // add flag
    stuffed_packet[0] = FLAG;

    unsigned short int bit_pos = 0;
    unsigned char stuffing = 0;
    for (unsigned char i = 1; i < aprs_object.info_len + 16; i++) {
        for (unsigned char j = 0; j < 8; j++) {
            unsigned char bit = (packet[i] >> (7 - j)) & 0x1;
            if (bit == 0) {
                stuffing = 0;
            } else if (i != aprs_object.info_len + 16 - 1) {
                stuffing++;
            }

            // printf("index: %d bit_pos: %d bit: %d\n", i + ((j + bit_pos) / 8), 7 - ((j + bit_pos) % 8), bit);
            stuffed_packet[i + ((j + bit_pos) / 8)] |= bit << (7 - ((j + bit_pos) % 8));

            if (stuffing == 5) {
                bit_pos++;
                stuffing = 0;
                // printf("[STUFF] index: %d bit_pos: %d bit: %d\n", i + ((j + bit_pos) / 8), 7 - ((j + bit_pos) % 8), bit);
                stuffed_packet[i + ((j + bit_pos) / 8)] |= 0x0 << (7 - ((j + bit_pos) % 8));
            }
        }
    }

    // TODO: unclear what stuffing offset is supposed to do to final flag transmission

    // add to length the number of bytes added by stuffing
    packet_length = aprs_object.info_len + 20 + (bit_pos / 8) + (bit_pos % 8 != 0);

    strcpy(packet, stuffed_packet);
}

// takes an AX.25 packet and formats it with HDLC framing and bit stuffing
void Radio::build_hdlc_packet() {
    raw_hdlc();
    stuff_hdlc_packet();
}

// AX.25 HEADER W/ HLDC ADDRESS FIELD CODE

// takes a callsign and modifies it in AX.25 format
// (uppercase, padded with spaces, shifted left by 1 bit)
void Radio::ax25_call_sign(char *output, bool is_source) {
    char i = 0;

    if (!is_source) {
        // set output to D_CALLSIGN
        for (; i < 6; i++) {
            if (i > sizeof(D_CALLSIGN) - 2) {
                break;
            }
            output[i] = D_CALLSIGN[i];
        }

        if (i == 3) {
            // we have a 3 character callsign,
            // room to add symbol and table and overlay
            output[i++] = APRS_SYMBOL;
            output[i++] = APRS_TABLE;
            output[i++] = APRS_OVERLAY;
        }

        if (i < 6) {
            // add padding
            while (i < 6) {
                output[i++] = ' ';
            }
        }
    } else {
        // set output to S_CALLSIGN
        for (; i < 6; i++) {
            if (i > sizeof(S_CALLSIGN) - 2) {
                break;
            }
            output[i] = S_CALLSIGN[i];
        }

        if (i < 6) {
            // add padding
            while (i < 6) {
                output[i++] = ' ';
            }
        }
    }

    // uppercase
    for (i = 0; i < 6; i++) {
        output[i] = toupper(output[i]);
    }

    // shift left by 1 bit
    for (i = 0; i < 6; i++) {
        output[i] = output[i] << 1;
        // set the last bit to 0 if we're a source
        // and 1 if we're a destination
        output[i] |= is_source ? 0x0 : 0x0;
    }
}

// takes an ssid and returns in AX.25 format
unsigned char Radio::ax25_ssid(bool is_source) {
    // ssid is S_SSID if source, D_SSID if destination
    unsigned char ssid = is_source ? S_SSID : D_SSID;

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
    // build callsigns
    char d_address[6];
    char s_address[6];
    ax25_call_sign(d_address, false);
    ax25_call_sign(s_address, true);

    // add callsigns and ssids to header
    for (int i = 0; i < 6; i++) {
        // ensure we only get the first 7 bits of each character
        ax25_header_packet[i] = d_address[i] & 0xff;
    }
    ax25_header_packet[6] = ax25_ssid(false);

    for (int i = 0; i < 6; i++) {
        // ensure we only get the first 7 bits of each character
        ax25_header_packet[i + 7] = s_address[i] & 0xff;
    }
    ax25_header_packet[13] = ax25_ssid(true);

    // TODO: AX.25 4.2.1 bit lengths
    ax25_header_packet[14] = CF;
    ax25_header_packet[15] = PID;

    // TODO: can CRC be partially built for AX.25 header?
}

// AX.25 CODE

// takes an info field and builds an AX.25 packet
void Radio::build_ax25_packet() {
    char empty = 0x0;
    sprintf(packet, "%c%s%s%c%c%c", empty, ax25_header_packet, (aprs_object.packet + 18), empty, empty, empty);
}

// APRS CODE

// take a total length and pre-packeted info
// and return a FX.25 packet
void Radio::build_aprs_packet() {
    // flag = 1 byte
    // header = 16 bytes
    // info = info.length() bytes
    // fcs = 2 bytes
    // flag = 1 byte
    // total = info.length() + 20 bytes
    
    build_ax25_packet();
    build_hdlc_packet();

    // this forward error correction (FEC) code
    // to turn things into FX.25 for improved range
    // has been removed. See .fec.cpp for more.

    // FEC_Tag tag = opt_pack_tag(hdlc.len);
    // packet fx25 = fx25_packet(tag, hdlc);
    // return fx25;
}

// RADIO CODE

// Send connection AT command to DRA818V
void Radio::init_radio() {
    Serial1.println(F("AT+DMOCONNECT"));
}

// Send reset AT command to DRA818V
// This is just 3 inits without reading back
void Radio::reset_radio() {
    for (char i = 0; i < 1; i++) {
        Serial1.println(F("AT+DMOCONNECT"));
    }
}

// Send AT command to DRA818V to set frequency
void Radio::set_frequency() {
    Serial1.print(F("AT+DMOSETGROUP=0,"));
    Serial1.print(RADIO_FREQ, 4);
    Serial1.print(',');
    Serial1.print(RADIO_FREQ, 4);
    Serial1.println(F(",0000,0,0000"));
}

// Send AT command to DRA818V to set filter
void Radio::set_filter() {
    // TODO: make configurable and explain settings
    Serial1.print(F("AT+SETFILTER="));
    Serial1.print(0);
    Serial1.print(',');
    Serial1.print(0);
    Serial1.print(',');
    Serial1.println(0);
}

// Read incoming data from DRA818V
// Needed between AT commands
void Radio::read_radio() {
    // technically this variable isn't needed
    // as we aren't doing anything with the data atm
    String d;
    
    unsigned long current_time  = millis();
    while (Serial1.available() < 1) {
        if ((millis() - current_time) / 1000 > 5) {
            // TODO: connection failed, handle error
            #ifdef DEBUG
                micro_sd.current_file.println(F("Radio connection failed"));
                micro_sd.current_file.flush();
            #endif
            // leds.set_error();
            // TODO: maybe we kill the program instead?
            break;
        }
    }
    if (Serial1.available() > 0) {
        d = Serial1.readString();
        // handle data if we want more advanced error handling and logic here
    }
}

// MAIN

// Send AT commands over Serial1 to setup the DRA818V
void Radio::setup_handler() {
    pinMode(MIC_PIN, OUTPUT);
    pinMode(PTT_PIN, OUTPUT);
    pinMode(PD_PIN, OUTPUT);


    digitalWrite(PTT_PIN, HIGH); // low -> tx, high -> rx
    digitalWrite(PD_PIN, HIGH); // low -> sleep mode, high -> normal mode

    Serial1.begin(RADIO_BAUDRATE);

    delay(250); // may need delay after pin setup before interfacing

    ax25_header();
    
    reset_radio();
    read_radio();
    delay(1000); // - may need potential delay after reset

    set_frequency();
    read_radio();
    delay(1000); // - may need potential delay before setting filter

    set_filter();
    read_radio();

    Serial1.end();
}

void Radio::loop_handler() {
    // check if it is time to broadcast
    if ((millis() - last_transmission) / 1000 >= (DATA_TIMEOUT - (SLOT_ERROR / 2))) {
        // check again to make sure slot is good
        // get current seconds, subtract slot,
        // and check if it's divisible by DATA_TIMEOUT
        // if it's not divisible, we're not in a good slot

        // need margin of +/- SLOT_ERROR seconds to account for drift
        if (!gps.stale && (gps.second - DATA_SLOT) % DATA_TIMEOUT < SLOT_ERROR) {
            // we're in a good slot, write data
            build_aprs_packet();
            send_packet();
            return;
        } else if (gps.stale) {
            // GPS is stale but it has been about DATA_TIMEOUT seconds
            // we want to broadcast that we are still alive, so broadcast anyway
            build_aprs_packet();
            send_packet();
            return;
        }
    }
}

Radio::Radio() {
    crc = 0xffff;
    bit_stuff = 0;
    nada = _2400;

    packet_length = 0;

    last_transmission = 0;
}

Radio radio = Radio();
