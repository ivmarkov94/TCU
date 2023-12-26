#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <stdint.h>
#include "cmsis_gcc.h"

uint32_t get_time_ms(void);
uint64_t get_time_us(void);
uint32_t time_elapsed_ms(uint32_t last_time_ms, uint32_t interval_ms);

#endif /* TIME_UTILS_H */