#ifndef __APRS_H__
#define __APRS_H__

void aprs_send();

extern const uint32_t VALID_POS_TIMEOUT = 2000;  // ms
extern int32_t next_aprs = 0;

#endif
