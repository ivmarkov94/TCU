#include "time_utils.h"

volatile uint32_t timer_ms;

uint32_t time_elapsed_ms(uint32_t last_time_ms, uint32_t interval_ms)
{
    return (get_time_ms() - last_time_ms) > interval_ms;
}

uint32_t get_time_ms(void)
{
    return timer_ms;
}

__attribute__((weak)) uint64_t get_time_us(void)
{
    return 1ULL;
}
