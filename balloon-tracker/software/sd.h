#pragma once

#include <SD.h>

class SD_class {
    private:
        File current_file;
        unsigned long last_log = 0;
    public:
        SD_class();
        void setup_handler();
        void loop_handler();
};

extern SD_class sd_object;