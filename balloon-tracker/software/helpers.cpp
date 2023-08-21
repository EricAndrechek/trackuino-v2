#include <SoftwareSerial.h>
#include "Arduino.h"
#include "config.h"

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
        Serial.begin(SERIAL_BAUDRATE);
        Serial.print(F("Free memory: "));
        Serial.print(freeMemory());
        Serial.println(F(" bytes"));
        Serial.flush();
        Serial.end();
    #endif
}

void log_init(const char file[], unsigned long size) {
    #ifdef DEBUG
        Serial.begin(SERIAL_BAUDRATE);
        Serial.print(F("Initialized "));
        Serial.print(file);
        Serial.print(F(" with "));
        Serial.print(size);
        Serial.println(F(" bytes"));
        Serial.flush();
        Serial.end();
    #endif
}