#include "time_utils.h"
#include "main.h"

volatile uint32_t timer_ms;

uint32_t time_elapsed_ms(uint32_t last_time_ms, uint32_t interval_ms)
{
    return (get_time_ms() - last_time_ms) > interval_ms;
}

uint32_t get_time_ms(void)
{
    return timer_ms;
}

void systick_upd_callback(void)
{
  timer_ms += 1U;
}

uint64_t get_time_us(void)
{
  uint32_t systik_val, ms_val;
  __disable_irq();
  systik_val = SysTick->VAL;
  ms_val = timer_ms;
  __enable_irq();
  return ms_val*1000u+((0xFFFFFFu - systik_val)/72u);
}
