#ifndef __HELPERS_CPP__
#define __HELPERS_CPP__

#include "config.h"
#include "variables.h"

#include <SoftwareSerial.h>

#include "libraries/Arduino-Log/ArduinoLog.h"

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__



void logging_setup() {
    #if LOG_OUTPUT == SD
        // get SD serial
        
}


#ifdef LOG_OUTPUT == SD
    #include <SD.cpp>


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
    #ifdef DEBUG_LEVEL >= 3
        info_print(freeMemory() + F(" bytes free"));
    #endif
}

#endif