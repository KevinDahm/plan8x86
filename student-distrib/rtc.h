#ifndef _RTC_H
#define _RTC_H

#include "types.h"

#define RTC_PORT 0x70
#define CHOOSE_RTC_A 0x8A
#define CHOOSE_RTC_B 0x8B
#define CHOOSE_RTC_C 0x0C
#define BASE_RTC_FREQ 0x06
#define RTC_CMD_A 0x20
#define RTC_CMD_B 0x40

extern void rtc_init();
#endif
