#ifndef VBAT_H
#define VBAT_H
#include <stdint.h>
bool vbat_init(void);
float get_battery_level(void);
uint8_t get_battery_byte(void);
#endif