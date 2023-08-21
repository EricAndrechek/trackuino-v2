#pragma once

class Lora_class {
    private:
        // last time LORA was updated
        unsigned long last_lora = 0;
    public:
        Lora_class();
        void setup_handler();
        void loop_handler();
        void broadcast();
};

extern Lora_class lora_object;