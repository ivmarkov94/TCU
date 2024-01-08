#ifndef SELFTEST_PRIVATE
#define SELFTEST_PRIVATE
#include "device.h"
#include "adc.h"
#include "usart_console.h"

typedef enum{
    st_adc,
    st_ntc_amb,
    st_ntc_pcb,
    st_pwr_5v,
    st_pwr_m3v,
    st_pwr_12v,
    st_amplifier,
    st_relay,
    st_mosfet,
    st_obj_num
}obj_var_t;

typedef struct{
    int32_t l_bound;
    int32_t r_bound;
}exp_val_t;

typedef struct{
    exp_val_t     exp_val;
    char*         name;
    adc_channel_t adc_ch;
    int32_t       cur_val;
    bool          status;/* true it is ok */
}object_t;

typedef struct{
    bool sys_state; /* true it is ok */
    object_t obj[st_obj_num];
    bool print_all; /* false, print errors only */
}selftest_t;

bool check_object(obj_var_t dut);
bool check_bounds(obj_var_t dut);
void prepare_relay_data(obj_var_t dut);
#endif