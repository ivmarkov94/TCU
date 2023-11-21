#include "selftest.h"
#include "selftest_private.h"
#include "pid_regulator.h"
#include "tim.h"
                         /* Left bound   Right bound    Name         Data_source*/
selftest_t slft = {.sys_state = true, .obj={{{ 1155/*mV*/,  1265/*mV*/  }, "adc"      , adc_ch_vref},  
                                            {{-45000/*mC*/, 115000/*mC*/}, "ntc_amb"  , adc_ch_ntc_amb},  
                                            {{-45000/*mC*/, 115000/*mC*/}, "ntc_pcb"  , adc_ch_ntc_pcb},  
                                            {{ 4900/*mV*/,  5100/*mV*/  }, "pwr_5v"   , adc_ch_div5v}, 
                                            {{-5000/*mV*/, -2500/*mV*/  }, "pwr_m3v"  , adc_ch_divm3v },  
                                            {{ 9000/*mV*/,  15600/*mV*/ }, "pwr_12v"  , adc_ch_div12v}, 
                                            {{ 84000/*mC*/, 87000/*mC*/ }, "amplifier", adc_ch_t_eng},
                                            {{ 200/*mC*/,   70000/*mC*/ }, "relay"    , adc_ch_t_eng},  
                                            {{ 300/*mA*/,   1400/*mA*/  }, "mosfet"   , adc_ch_cur_sens}}};

bool selftest_get_state(void)
{
    return slft.sys_state;
}

void selftest_set_print_all(bool val)
{
    slft.print_all = val;
}

void selftest_whole_test(void)
{
    obj_var_t dut = st_adc;
    slft.sys_state = true;
    while((slft.sys_state) && (dut < st_obj_num))
    {
        slft.sys_state = check_object(dut++);
    }
    selftest_print_errors();
}

void selftest_short_test(void)
{
    obj_var_t dut = st_adc;
    if(!slft.sys_state)
    {
        selftest_print_errors();
    }
    while((slft.sys_state) && (dut < st_amplifier))
    {
        slft.sys_state = check_object(dut++);
    }
}

bool selftest_check_temperature_feedback(void)
{
    bool result = true;
    static uint32_t timer_ms;
    static int32_t teng_save;
    if(get_pwm_duty() >= 1800)
    {
        if(time_elapsed_ms(timer_ms, DMAX_MAXTIME) && ((teng_save - adc_get_val(adc_ch_t_eng, true))<500))
        {
           result = false; 
        }
    }else{
        timer_ms = get_time_ms();
        teng_save = adc_get_val(adc_ch_t_eng, true);
    }
    return result;
}

bool check_object(obj_var_t dut)
{
    bool result = false;
    switch (dut)
    {
    case st_adc:
    case st_ntc_amb:
    case st_ntc_pcb:
    case st_pwr_5v:
    case st_pwr_12v:
    case st_pwr_m3v:
    case st_amplifier:
        slft.obj[dut].cur_val = adc_get_val(slft.obj[dut].adc_ch, true);
        result = check_bounds(dut);
    break;

    case st_relay:{
        prepare_relay_data(dut);
        result = check_bounds(dut);
    break;}

    case st_mosfet:
        set_pwm_duty(0);
        WAIT(400);
        if(adc_get_val(slft.obj[dut].adc_ch, true) < 50/* mA */)
        {
            set_pwm_duty(2000);
            WAIT(400);
            slft.obj[dut].cur_val = adc_get_val(slft.obj[dut].adc_ch, true);
            result = check_bounds(dut);
        }else{
            printf("Test %s is fail. Value out of bounds [50mA > %ld]"NLINE, slft.obj[dut].name, slft.obj[dut].cur_val);
        }

    break;

    default:
        break;
    }
    slft.obj[dut].status = result;

    return result;
}

bool check_bounds(obj_var_t dut)
{
    bool result = false;

    if ((slft.obj[dut].cur_val > slft.obj[dut].exp_val.l_bound) &&
        (slft.obj[dut].cur_val < slft.obj[dut].exp_val.r_bound))
    {
        result = true;
        if(slft.print_all) 
        {
            printf("Test %s is PASS. [%ld > %ld > %ld]"NLINE, slft.obj[dut].name, slft.obj[dut].exp_val.l_bound, slft.obj[dut].cur_val, slft.obj[dut].exp_val.r_bound);
            WAIT(5);
        }
    }else{
        printf("Test %s is fail. Value out of bounds [%ld > %ld > %ld]"NLINE, slft.obj[dut].name, slft.obj[dut].exp_val.l_bound, slft.obj[dut].cur_val, slft.obj[dut].exp_val.r_bound);
    }
    return result;
}

void prepare_relay_data(obj_var_t dut)
{
    int32_t delta;
    LL_GPIO_ResetOutputPin(en_relay_GPIO_Port,en_relay_Pin);
    WAIT(300)
    delta =  adc_get_val(slft.obj[dut].adc_ch, true);
    LL_GPIO_SetOutputPin(en_relay_GPIO_Port,en_relay_Pin);
    WAIT(300)
    delta -=  adc_get_val(slft.obj[dut].adc_ch, true);
    slft.obj[dut].cur_val = delta<0? delta*-1:delta; 
    LL_GPIO_ResetOutputPin(en_relay_GPIO_Port,en_relay_Pin);
    WAIT(100)
}

void selftest_print_errors(void)
{
    obj_var_t dut = st_adc; 
    if(slft.sys_state)
    {
        printf("System is ready"NLINE);
    }else{
        printf("System has next errors:"NLINE);    
        while((dut < st_obj_num))
        { 
            if(slft.obj[dut].status==false)
            {
                WAIT(5)
                printf("Test %s is fail. Value out of bounds [%ld > %ld > %ld]"NLINE, slft.obj[dut].name, slft.obj[dut].exp_val.l_bound, slft.obj[dut].cur_val, slft.obj[dut].exp_val.r_bound);
            }
            dut++;
        }
    }
}