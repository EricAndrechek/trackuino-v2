// FEC proved to be too memory and compute heavy to run on a HAB after preliminary testing.
// This is the code that was used if anyone can get it to work efficiently in the future.

// It was written prior to the radio.cpp and radio.h files being turned into streamlined classes
// and will need to be updated to work with the new code.


// FX.25 CODE

// given ax.25 processed information, find the optimal FEC tag
FEC_Tag opt_pack_tag(unsigned char data_size) {

    #ifdef DEBUG
        Serial.print(F("AX.25 Info Length: "));
        Serial.println(data_size - 20);
        Serial.print(F("Data Size: "));
        Serial.println(data_size);
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
        Serial.print(F("Optimal Tag: "));
        Serial.println(tag.tag);
        Serial.print(F("Tag Data: "));
        Serial.println(tag.data);
        Serial.print(F("Tag Check: "));
        Serial.println(tag.check);
        Serial.print(F("Tag Value: "));
        print_unsigned_long_long(tag.tag_value);
        Serial.println();
    #endif

    return tag;
}

packet fx25_padded(FEC_Tag tag, packet hdlc) {
    #ifdef DEBUG
        Serial.print(F("FX.25 Data Packet Length: "));
        Serial.println(tag.data);
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
        Serial.println(F("FX.25 Data Packet: "));
        print_hex(p.packet, p.len);
        Serial.println();
    #endif

    return p;
}

// TODO: implement this correctly for RS FEC
char* fec_check_symbols(FEC_Tag tag, packet fx_padded) {
    #ifdef DEBUG
        Serial.print(F("FX.25 Check Packet Length: "));
        Serial.println(tag.check);
    #endif

    const int message_length = 64;
    const int ecc_length = 16;

    RS::ReedSolomon<message_length, ecc_length> rs;

    char encoded[message_length + ecc_length];
    rs.Encode(fx_padded.packet, encoded);

    char* fec_symbols = new char[tag.check];
    for (int i = 0; i < tag.check; i++) {
        fec_symbols[i] = encoded[fx_padded.len + i];
    }

    #ifdef DEBUG
        Serial.println(F("FX.25 Check Packet: "));
        print_hex(fec_symbols, tag.check);
        Serial.println();
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
        // flip MSB to LSB
        char temp = 0;
        for (int k = 0; k < 8; k++) {
            temp |= ((fx_p[tag.data + j] >> k) & 0x01) << (7 - k);
        }
        fx_p[tag.data + j] = temp;
    }

    p.len = tag.data + tag.check;
    p.packet = fx_p;

    #ifdef DEBUG
        Serial.print(F("FX.25 Codeblock Length: "));
        Serial.println(tag.data + tag.check);
        Serial.println(F("FX.25 Codeblock: "));
        print_hex(p.packet, p.len);
        Serial.println();
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
        fx_p[i] = (tag.tag_value >> (8 * (i - 2))) & 0xff;
        char temp = 0;
        for (int j = 0; j < 8; j++) {
            temp |= ((fx_p[i] >> j) & 0x01) << (7 - j);
        }
        fx_p[i] = temp;
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
        Serial.print(F("FX.25 Packet Length: "));
        Serial.println(tag.data + tag.check + 12);
        Serial.println(F("FX.25 Packet: "));
        print_hex(fx_p, tag.data + tag.check + 12);
        Serial.println();
    #endif

    p.len = tag.data + tag.check + 12;
    p.packet = fx_p;
    
    return p;
}
