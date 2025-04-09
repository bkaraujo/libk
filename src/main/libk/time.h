#ifndef __LIBK_TIME__
#define __LIBK_TIME__

#include "libk/defines.h"

typedef struct {
    u16 millis;
    u16 year; u8 month; u8 day;
    u8 hour; u8 minute; u8 second;
} KClock;

void k_time_clock(KClock* clock);
u64 k_time_epoch_millis(void);
u64 k_time_epoch_micros(void);

#endif