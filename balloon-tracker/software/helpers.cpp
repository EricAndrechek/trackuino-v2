#include <SoftwareSerial.h>
#include "Arduino.h"

#include "config.h"
#include "microsd.h"

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

// returns the number of bytes currently free in RAM
int freeMemory() {
    char top;
    #ifdef __arm__
        return &top - reinterpret_cast<char*>(sbrk(0));
    #elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
        return &top - __brkval;
    #else  // __arm__
        return __brkval ? &top - __brkval : &top - __malloc_heap_start;
    #endif  // __arm__
}

// print number of bytes currently free in RAM to info
void print_free_mem() {
    #ifdef DEBUG
        micro_sd.current_file.print(F("Free memory: "));
        micro_sd.current_file.print(freeMemory());
        micro_sd.current_file.println(F(" bytes"));
        micro_sd.current_file.flush();
    #endif
}

void log_init(const char file[], unsigned long size) {
    #ifdef DEBUG
        micro_sd.current_file.print(F("Initialized "));
        micro_sd.current_file.print(file);
        micro_sd.current_file.print(F(" with "));
        micro_sd.current_file.print(size);
        micro_sd.current_file.println(F(" bytes"));
        micro_sd.current_file.flush();
    #endif
}

void chip_select_lora() {
    digitalWrite(SD_CS_PIN, HIGH);
    digitalWrite(LORA_CS_PIN, LOW);
}

void chip_select_sd() {
    digitalWrite(LORA_CS_PIN, HIGH);
    digitalWrite(SD_CS_PIN, LOW);
}