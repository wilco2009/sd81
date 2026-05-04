#ifndef RTC_H
#define RTC_H

#include "GLOBALS.h"
#include <STM32RTC.h>

extern STM32RTC& rtc;

void rtc_init(void);

#endif


