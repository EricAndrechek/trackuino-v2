#pragma once

#include <SD.h>

class MicroSD {
    private:
        unsigned long last_log;
    public:
        File current_file;
        MicroSD();
        void setup_handler();
        void loop_handler();
};

extern MicroSD micro_sd;