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