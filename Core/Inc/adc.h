/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.h
  * @brief   This file contains all the function prototypes for
  *          the adc.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "stdbool.h"
#define RELAY_CONNECT_MS 1 /* ms */
#define ADC_UPD_AVG_MS 210 /* ms */
#define ADC_UPD_RAW_MS 55 /* ms */
/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
typedef enum{
    adc_ch_ntc_amb,  /* mC */
    adc_ch_div12v,   /* mv */
    adc_ch_cur_sens, /* mA */
    adc_ch_int_temp, /* mC */
    adc_ch_vref,     /* mV */
    adc_ch_ntc_pcb,  /* mC */
    adc_ch_div5v,    /* mV */
    adc_ch_divm3v,   /* mV */
    adc_ch_t_eng,    /* mC */
    adc_ch_num      
}adc_channel_t;
/* USER CODE END Private defines */

void MX_ADC1_Init(void);

/* USER CODE BEGIN Prototypes */
void adc_start(void);
int32_t adc_get_raw_val(adc_channel_t adc_ch);
int32_t adc_get_special_val(adc_channel_t adc_ch);
int32_t adc_get_val(adc_channel_t adc_ch, bool avg);

void tim2_upd_callback(void);
void adc_upd_process(void);
void dma_tc_callback(void);
void dma_te_callback(void);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __ADC_H__ */

