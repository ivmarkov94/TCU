#ifndef PID_REGULATOR_H
#define PID_REGULATOR_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#define D_PART_LST_SEC 6

typedef enum{
  init_st,
  off_control,
  bmw_control,
  custom_control,
  test_mode1_low_duty,
  test_mode2_high_duty
}cs_state_t;

typedef enum{
  wait_charge_voltage,
  check_charge_voltage,
  wait_work_temperature,
  temperature_control
}temp_control_state_t;

typedef struct{
  float    gain;
  uint32_t period_ms;
  int32_t  value; 
  uint32_t timer;
}p_t;

typedef struct{
  float gain;
  int32_t sum_err;
  uint32_t period_ms;
  int32_t saturation;
  int32_t start_cond;
  int32_t  value; 
  uint32_t timer;
}i_t;

typedef struct{
  int32_t ready;
  int32_t gain;
  int32_t last_err[D_PART_LST_SEC];
  uint32_t last_err_indx;
  uint32_t new_err_indx;
  uint32_t period_ms;
  int32_t start_cond;
  int32_t  value; 
  uint32_t timer;
}d_t;

typedef struct {
  int32_t err;
  int32_t ref;/* C*10 */

  p_t p;
  i_t i;
  d_t d;
}pid_reg_t;

typedef struct {
  cs_state_t state;
  temp_control_state_t tc_state;
  pid_reg_t   pid;
}control_system_t;

void pid_set_p(float val);
void pid_set_d(float val);
void pid_set_i(float val);
void control_system(void);
void control_system_set_state(cs_state_t new_state);
#endif /* PID_REGULATOR_H */

