#pragma once

#include <TinyGPSMinus.h>
#include <SoftwareSerial.h>

class GPS {
    private:
        
        SoftwareSerial * gps_serial;
    public:
        GPS();
        TinyGPSMinus tinygps_object;
        void setup_handler(SoftwareSerial * ss);
        void loop_handler();

        // storing time data here instead of in pre-cracked format technically increases memory consumption,
        // but is far more efficient than calling crack_datetime() every time we need to access the time
        // and is more convenient than managing variables everywhere else
        int year;
        char month, day, hour, minute, second, hundredths;
        unsigned long age;
        bool stale;
        bool last_stale;
        char datetime[32];

        void get_datetime();
};

extern GPS gps;