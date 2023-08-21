#pragma once

class Sensors {
    private:
        // last time sensors were updated
        unsigned long last_sensors = 0;
    public:
        float temperature = 0.0;
        float voltage = 0.0;

        Sensors();
        void setup_handler();
        void loop_handler();
};

extern Sensors sensors;