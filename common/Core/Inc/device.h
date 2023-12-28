
#ifndef DEVICE_H
#define DEVICE_H
#include "main.h"
#include <stdio.h>
#include "stdbool.h"
#include <stdlib.h>
#include <string.h>
#include "stm32f103xb.h"
#include "time_utils.h"

#define UNKNOWN_ERROR 0xA0
#define STACK_OVERFLOW_ERROR 0xA1
#define REBOOT_INFO_AREA    (FLASH_BASE + 0xFC00)
#define ARRAY_SIZE(x)  (sizeof(x) / sizeof((x) [0]))

#define WAIT(time_ms) \
    {static uint32_t ms_tim = 0;\
    ms_tim = get_time_ms();\
    while(1)\
    {\
      wdgs_refresh(); \
      if(time_elapsed_ms(ms_tim, time_ms))\
      {\
        break;\
      }\
    }}

#define LED_TONGLE_MS(time_ms) \
    {static uint32_t ms_tim =0;\
    if(time_elapsed_ms(ms_tim, time_ms))\
    {\
      ms_tim = get_time_ms();\
      LL_GPIO_TogglePin(led_GPIO_Port,led_Pin);\
    }}

#define LED_TONGLE_CNT(time_stps) \
    {\
        static uint32_t cnt = 0;\
        cnt++;\
        if(cnt >= time_stps)\
        {\
        cnt = 0;\
        LL_GPIO_TogglePin(led_GPIO_Port,led_Pin);\
        }\
    }

#define JITTER_MS(jitter_var)   \
    {                           \
        static uint32_t us_last;\
        uint32_t us_now;        \
        us_now = get_time_ms(); \
        jitter_var = us_now - us_last;\
        us_last = us_now;       \
    }

#define JITTER_US(jitter_var)   \
    {                           \
        static uint64_t us_last;\
        uint64_t us_now;        \
        us_now = get_time_us(); \
        jitter_var = us_now - us_last;\
        us_last = us_now;       \
    }

#define TASK(task_name, repeat_time_ms)                 \
  {                                                     \
    static uint32_t t_##task_name;                      \
    if((t_##task_name + repeat_time_ms) < get_time_ms())\
    {                                                   \
      t_##task_name = get_time_ms();                    \
      task_name();                                      \
    }                                                   \
  }

#define TASK_ARG(task, repeat_time_ms)                  \
  {                                                     \
    static uint32_t t_save;                      \
    if((t_save + repeat_time_ms) < get_time_ms())\
    {                                                   \
      t_save = get_time_ms();                           \
      task;                                             \
    }                                                   \
  }
    
#endif /* DEVICE_H */
