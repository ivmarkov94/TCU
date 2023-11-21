/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
#include "device.h"
#include "tim.h"
#include "adc.h"
#include "pid_regulator.h"
#include "selftest.h"
uart_t uart3;
uint8_t tx_buffer[UART_TX_BUF];
ring_buffer_t tx_ring_buf = {.buffer = tx_buffer, .size = UART_TX_BUF, .flag_overflow = 0, .flag_error_pop = 0};

/* static int32_t get_int_arg(uint8_t* ptr); */
static float get_float_arg(uint8_t* ptr);
/* USER CODE END 0 */

/* USART3 init function */

void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  LL_USART_InitTypeDef USART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);

  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
  /**USART3 GPIO Configuration
  PB10   ------> USART3_TX
  PB11   ------> USART3_RX
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_10;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_11;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_FLOATING;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USART3 interrupt Init */
  NVIC_SetPriority(USART3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),1, 0));
  NVIC_EnableIRQ(USART3_IRQn);

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  USART_InitStruct.BaudRate = 115200;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART3, &USART_InitStruct);
  LL_USART_ConfigAsyncMode(USART3);
  LL_USART_Enable(USART3);
  /* USER CODE BEGIN USART3_Init 2 */
  LL_USART_EnableIT_RXNE(USART3);
  LL_USART_EnableIT_TC(USART3);
  /* USER CODE END USART3_Init 2 */

}

/* USER CODE BEGIN 1 */
int _write(int file, char *ptr, int len)
{
  uint16_t ind = 0;
  while (ind<len)
  {
    ring_push(ptr[ind],&tx_ring_buf);
    ind++;
  }
  LL_USART_TransmitData8(USART3, ring_pop(&tx_ring_buf));
  return len;
}

void uart_buff_put_char(uint8_t data)
{
  if(uart3.last_indx < UART_RX_BUF)
  {
    uart3.rx_buffer[uart3.last_indx++] = data;
    uart3.last_upd_ms = get_time_ms();
  }else{
    uart_rx_buff_reset();
  }
}

void uart_rx_buff_reset(void)
{
  uart3.last_indx = 0u;
}

uint8_t uart_is_str_ready(void)
{
  uint8_t result = false;
  if(time_elapsed_ms(uart3.last_upd_ms, 200) && uart3.last_indx!=0)
  {
    result = true;
  }
  return result;
}

static uint8_t dbg_active;
static dbg_var_e dbg_var;
void receive_cmd(void)
{
  if(uart_is_str_ready())
  {
    if(IS_CMD_RECIVED("adc1?"))
    {
      dbg_var = dbg_var_adc_channels_scaled_part1;
      dbg_active ^=1;
    }else if(IS_CMD_RECIVED("adc2?"))
    {
      dbg_var = dbg_var_adc_channels_scaled_part2;
      dbg_active ^=1;
    }else if(IS_CMD_RECIVED("adcr1?"))
    {
      dbg_var = dbg_var_adc_channels_raw_part1;
      dbg_active ^=1;
    }else if(IS_CMD_RECIVED("adcr2?"))
    {
      dbg_var = dbg_var_adc_channels_raw_part2;
      dbg_active ^=1;
    }else if(IS_CMD_RECIVED("amp?"))
    {
      dbg_var = dbg_var_amplifier_dif_input;
      dbg_active ^=1;
    }else if(IS_CMD_RECIVED("dbg_base"))
    {
      dbg_var = dbg_var_base;
      dbg_active ^=1;
    }else if(IS_CMD_RECIVED("p="))
    {
      pid_set_p((get_float_arg(&uart3.rx_buffer[2])));
    }else if(IS_CMD_RECIVED("d="))
    {
      pid_set_d(get_float_arg(&uart3.rx_buffer[2]));
    }else if(IS_CMD_RECIVED("i="))
    {
      pid_set_i(get_float_arg(&uart3.rx_buffer[2]));
    }else if(IS_CMD_RECIVED("inpwm?"))
    {
      printf("T=%6.1fms, f=%5.1fHz"NLINE,pwm_capt.pwm_period_us/1000.f, 1000.f/(pwm_capt.pwm_period_us/1000.f));
    }else if(IS_CMD_RECIVED("c_bmw"))
    {
      control_system_set_state(bmw_control);
    }else if(IS_CMD_RECIVED("c_cust"))
    {
      control_system_set_state(custom_control);
    }else if(IS_CMD_RECIVED("c_off"))
    {
      control_system_set_state(off_control);
    }else if(IS_CMD_RECIVED("tm1"))
    {
      control_system_set_state(test_mode1_low_duty);
    }else if(IS_CMD_RECIVED("tm2"))
    {
      control_system_set_state(test_mode2_high_duty);
    }else if(IS_CMD_RECIVED("wdt")) 
    {
      while(1)
      {
        ;
      }
    }else if(IS_CMD_RECIVED("st?"))/* selftest print */
    {
      selftest_print_errors();
    }else if(IS_CMD_RECIVED("st_wh"))/* selftest whole test */
    {
      selftest_set_print_all(true);
      selftest_whole_test();
      selftest_set_print_all(false);
    }else
    {
      printf("Unknowned cmd %s"NLINE"Cmd list:"NLINE"adc1?"NLINE"adc2?"NLINE"adcr1?"NLINE"adcr2?"NLINE"amp?"NLINE"dbg_base"NLINE"wdt"NLINE"c_off"NLINE"c_cust"NLINE"c_bmw"NLINE"inpwm?"NLINE"tm1"NLINE"tm2"NLINE"p="NLINE"d="NLINE"i="NLINE,(char*)&uart3.rx_buffer[0]);
    }
    uart_rx_buff_reset();
  }
}

void debug_handler(void)
{
  if(dbg_active)
  {
    switch (dbg_var)
    {
    case dbg_var_base:
      printf("Ta=%4.1fC V=%2.1fV, I=%3.2fA, Te=%4.1fC, aD=%4.1f rD=%4.1f"NLINE,adc_get_val(adc_ch_ntc_amb, true)/1000.f, adc_get_val(adc_ch_div12v, true)/1000.f, adc_get_val(adc_ch_cur_sens, true)/1000.f, adc_get_val(adc_ch_t_eng, true)/1000., TIM4->CCR4/20.f, pwm_capt.req_duty/20.f);
    break;
    
    case dbg_var_adc_channels_scaled_part1:
      printf("Ta=%4.1fC, Tc=%4.1fC, Tb=%4.1fC, Teng=%4.1fC"NLINE, adc_get_val(adc_ch_ntc_amb, true)/1000.f, adc_get_val(adc_ch_int_temp, true)/1000.f, adc_get_val(adc_ch_ntc_pcb, true)/1000.f, adc_get_val(adc_ch_t_eng, true)/1000.);
    break;

    case dbg_var_adc_channels_scaled_part2:
      printf("12v=%5.3fV, 5v=%4.3fV, -3.2v=%4.3fV, Vref=%4ldmV, I=%4.3fA"NLINE, adc_get_val(adc_ch_div12v, true)/1000.f, adc_get_val(adc_ch_div5v, true)/1000.f, adc_get_val(adc_ch_divm3v, true)/1000.f, adc_get_val(adc_ch_vref, true), adc_get_val(adc_ch_cur_sens, true)/1000.f);
    break;

    case dbg_var_adc_channels_raw_part1:
      printf("Ta=%4ldmV, Tc=%4ldmV, Tb=%4ldmV, Teng=%4ldmV"NLINE, adc_get_raw_val(adc_ch_ntc_amb), adc_get_raw_val(adc_ch_int_temp), adc_get_raw_val(adc_ch_ntc_pcb), adc_get_raw_val(adc_ch_t_eng));
    break;

    case dbg_var_adc_channels_raw_part2:
      printf("12v=%4ldmV, 5v=%4ldmV, -3.2v=%4ldmV Vref=%4ldmV, I=%4ldmV"NLINE, adc_get_raw_val(adc_ch_div12v), adc_get_raw_val(adc_ch_div5v), adc_get_raw_val(adc_ch_divm3v), adc_get_raw_val(adc_ch_vref), adc_get_raw_val(adc_ch_cur_sens));
    break;

    case dbg_var_amplifier_dif_input:
      printf("in_v=%4ldmV T=%4.1f"NLINE, adc_get_special_val(adc_ch_t_eng), adc_get_val(adc_ch_t_eng, true)/1000.f);
    break;

    default:
      break;
    } 
    LL_GPIO_TogglePin(led_GPIO_Port,led_Pin);
  }
}

/* static int32_t get_int_arg(uint8_t* ptr)
{
  int32_t val = atoi((const char*)ptr);
  printf("Updated to = %ld"NLINE,val);
  return val;
} */

static float get_float_arg(uint8_t* ptr)
{
  float val = atof((const char*)ptr);
  printf("Updated to = %5.3f"NLINE,val);
  return val;
}
/* USER CODE END 1 */
