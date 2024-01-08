#include"pid_regulator.h"
#include "usart_console.h"
#include "selftest.h"
#include "adc.h"
#include "tim.h"

#define ENGIN_OFF_VOLTAGE 12800 /* mV */
#define RELAY_OFF_DELAY 1000 /* ms */
#define CHARGE_VOLTAGE 13500 /* mV */
#define CHARGE_VOLTAGE_CHECK_TIME 5000 /* ms */
#define RELAX_PAUSE 5000 /* ms */

static control_system_t cs = {0};

void pid_init(pid_reg_t *pid);
int32_t custom_temperature_control(void);

void control_system(void)
{
  switch (cs.state)
  {
  case init_st:
    control_system_set_state(custom_control);
    break;

  case off_control:
    ;
    break;

  case bmw_control:
    set_pwm_duty(pwm_capt.req_duty);
    break;

  case custom_control:
    set_pwm_duty(custom_temperature_control());
    break;

  case test_mode1_low_duty:
    set_pwm_duty(101);
    break;
  case test_mode2_high_duty:
    set_pwm_duty(1899);
    break;

  default:
    break;
  }
}

void control_system_set_state(cs_state_t new_state)
{
  switch (new_state)
  {/* preparing */
  case off_control:
    set_pwm_duty(0);
    break;

  case custom_control:
    cs.tc_state = wait_charge_voltage;
    break;
  
  default:
    break;
  }
  cs.state = new_state;
}

void pid_set_p(float val)
{
  cs.pid.p.gain = val;
}

void pid_set_d(float val)
{
  cs.pid.d.gain = val;
}

void pid_set_i(float val)
{
  cs.pid.i.gain = val;
}

void pid_init(pid_reg_t *pid)
{
  memset(pid, 0, sizeof(pid_reg_t));

  pid->ref = 900;

  pid->p.gain = 10;
  pid->p.period_ms = 100;

  pid->d.gain = 20;
  pid->d.period_ms = 1000;
  pid->d.start_cond = 10;/* delta between ref and current temperature [C] */

  pid->i.gain  = 0.5; 
  pid->i.sum_err = 1000/pid->i.gain;
  pid->i.period_ms  = 2500;
  pid->i.saturation = MAX_TIM_DUTY;
  pid->i.start_cond = 10;/* delta between ref and current temperature [C] */
}


void p_process(p_t* p, int32_t err);
void i_process(i_t* i, int32_t err);
void d_process(d_t* d, int32_t err);
int32_t get_last_err(d_t* d);
void set_new_err(d_t* d, int32_t n_err);
int32_t check_duty_borders(int32_t val);


int32_t custom_temperature_control(void)
{
  static uint32_t voltage_timer_ms = 0, dis_voltage_timer_ms = 0, upd_adc_timer_ms = 0, pause_timer = 0;
  int32_t pwm_duty = 0;

  pid_reg_t* pid = &cs.pid;
  p_t* p = &cs.pid.p;
  d_t* d = &cs.pid.d;
  i_t* i = &cs.pid.i;

  if(adc_get_val(adc_ch_div12v, false) < ENGIN_OFF_VOLTAGE)
  {
    if(LL_GPIO_IsOutputPinSet(en_relay_GPIO_Port, en_relay_Pin) && time_elapsed_ms(dis_voltage_timer_ms, RELAY_OFF_DELAY))
    {
      LL_GPIO_ResetOutputPin(en_relay_GPIO_Port, en_relay_Pin);
      printf("Engin disabled, relay disabled"NLINE);
      cs.tc_state = wait_charge_voltage;
    }
  }else{
    dis_voltage_timer_ms = get_time_ms();
  }

  if(selftest_get_state())
  {
    switch (cs.tc_state)
    {
    case wait_charge_voltage:
      if(adc_get_val(adc_ch_div12v, true) > CHARGE_VOLTAGE)
      {
        if(LL_GPIO_IsOutputPinSet(en_relay_GPIO_Port, en_relay_Pin))
        {
          cs.tc_state = wait_work_temperature;
          upd_adc_timer_ms = get_time_ms();
        }else{
          voltage_timer_ms = get_time_ms();
          cs.tc_state = check_charge_voltage;
        }
      }
      break;
      
    case check_charge_voltage:
      if(adc_get_val(adc_ch_div12v, true) > CHARGE_VOLTAGE)
      {/* add waiting time */
        if(time_elapsed_ms(voltage_timer_ms,CHARGE_VOLTAGE_CHECK_TIME))
        {
          LL_GPIO_SetOutputPin(en_relay_GPIO_Port, en_relay_Pin);
          cs.tc_state = wait_work_temperature;
          upd_adc_timer_ms = get_time_ms();
          printf("Engin enabled, relay enabled"NLINE);
        }
      }else{
        cs.tc_state = wait_charge_voltage;
      }
      break;

    case wait_work_temperature:
      if(time_elapsed_ms(upd_adc_timer_ms,ADC_UPD_AVG_MS))
      {
        if(adc_get_val(adc_ch_t_eng, true) > 85000)
        {
          cs.tc_state = temperature_control;
          pid_init(&cs.pid);
        }
      }
      break;

    case temperature_control:
      if(selftest_check_temperature_feedback())
      {
        if(time_elapsed_ms(pause_timer, RELAX_PAUSE))
        {
          pid->err = -(pid->ref - (adc_get_val(adc_ch_t_eng, true)/100));
          p_process(p, pid->err);
          i_process(i, pid->err);
          d_process(d, pid->err);

          pwm_duty = p->value + i->value + d->value; 
          pwm_duty = pwm_duty > MAX_DUTY? MAX_DUTY:pwm_duty;
          pwm_duty = pwm_duty < 0?        0       :pwm_duty;
          TASK_ARG(printf("V=%3.1fV, Te=%4.1fC, D=%4.1f, e=%3ld p=%4ld d=%4ld i=%4ld"NLINE, (adc_get_val(adc_ch_div12v, false)/1000.f), (adc_get_val(adc_ch_t_eng, true)/1000.f), pwm_duty/20.f, pid->err, p->value, d->value, i->value), 1000);
        }
      }else{
        printf("MAX Duty during %dsec, but temperature doesn't change, pause %dsec"NLINE,DMAX_MAXTIME/1000, RELAX_PAUSE/1000);
        pause_timer = get_time_ms();
      }
      break;
    
    default:
      break;
    }
  }
  return pwm_duty;
}


void p_process(p_t* p, int32_t err)
{
  if(time_elapsed_ms(p->timer, p->period_ms))
  {
    p->timer = get_time_ms();
    p->value = check_duty_borders(p->gain*err);
  }
}

void i_process(i_t* i, int32_t err)
{
  if(time_elapsed_ms(i->timer, i->period_ms))
  {
    i->timer = get_time_ms();
    if((err < i->start_cond*10) && 
      (err > -i->start_cond*10))
    {
      if((i->sum_err + err)<0)
      {
        i->sum_err = 0;
      }else{
        if(i->gain*(i->sum_err + err) <= i->saturation)
        {
          i->sum_err += err;
        }
      }
      i->value = check_duty_borders(i->gain*i->sum_err);
    }
  }
}

void d_process(d_t* d, int32_t err)
{
  if(time_elapsed_ms(d->timer, d->period_ms))
  {
    d->timer = get_time_ms();
    if(d->ready){
      d->value = d->gain*(err - get_last_err(d));
      set_new_err(d, err);
      if((err < d->start_cond*10) && 
        (err > -d->start_cond*10))
      {
        d->value = check_duty_borders(d->value);
      }else{
        d->value = 0;
      }
    }else{
      set_new_err(d, err);
    }
  }
}

int32_t check_duty_borders(int32_t val)
{
  val = val >  MAX_DUTY?  MAX_DUTY :val;
  val = val < -MAX_DUTY? -MAX_DUTY :val;
  return val;
}

void set_new_err(d_t* d, int32_t n_err)
{
  d->last_err[d->new_err_indx++] = n_err;
  if(d->new_err_indx >= D_PART_LST_SEC)
  {
    d->new_err_indx = 0;
    d->ready = true;
  }
}

int32_t get_last_err(d_t* d)
{
  uint32_t last_err;

  last_err = d->last_err[d->last_err_indx++];
  if(d->last_err_indx >= D_PART_LST_SEC)
  {
    d->last_err_indx = 0;
  }
  return last_err;
}
