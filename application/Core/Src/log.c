#include "log.h"
#include "adc.h"
#include "ring_buffer.h"
#include "usart_console.h"
#define DATA_IN_LINE 16
uint8_t log_buffer[LOG_BUFFER_SIZE];
ring_buffer_t log_data = {.buffer = log_buffer, .size = LOG_BUFFER_SIZE, .flag_overflow = 0, .flag_error_pop = 0};

bool pop_log_info(uint8_t *data);

bool push_log_info(uint8_t data, uint8_t size)
{
    bool result = true;
    for (uint8_t i = 0; i < size; i++)
    {
        if(ring_get_count(&log_data) < (LOG_BUFFER_SIZE-1u)){
            ring_push(data, &log_data);
        }else{
            result = false;
            break;
        }
    }
    return result;
}

bool pop_log_info(uint8_t *data)
{
    bool result = false;
    if(ring_get_count(&log_data) > 0){
        *data = ring_pop(&log_data);
        result = true;
    }
    return result;
}

void print_log_info(uint32_t min)
{
    uint8_t data;
    uint32_t sec_cnt=0, line=1;
    printf("%s"NLINE, LOG_VERSION);
    while((pop_log_info(&data)==true) && sec_cnt < (min*60))
    {
        if(sec_cnt%DATA_IN_LINE==0)printf("%ld:", line);
        sec_cnt++;
        printf("%2x ", data);
        if(sec_cnt%DATA_IN_LINE==0){
            printf(NLINE);
            line++;
            WAIT(60);
        }
    }
    if(sec_cnt%DATA_IN_LINE!=0)printf(NLINE);
    if(sec_cnt < (min*60))printf("Log buffer empty"NLINE);
}

void log_handler(void)
{
    static bool is_log_full = false, start_log = false;
    if(!is_log_full){
#ifdef LOG_VERSION_1
        if(start_log){
            int32_t t = (((adc_get_val(adc_ch_t_eng, true)-80000/* mC */)+50)/100);
            t = t>255? 255:t;
            t = t<0? 0:t;
            is_log_full = !push_log_info((uint8_t)t, 1u);
        }else{
            if(adc_get_val(adc_ch_t_eng, true) > 80000/* mC */)start_log = true;
        }
#endif
    }
}
