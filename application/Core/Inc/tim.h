/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    tim.h
 * @brief   This file contains all the function prototypes for
 *          the tim.c file
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
#ifndef __TIM_H__
#define __TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

  /* USER CODE BEGIN Includes */
  typedef struct{
    uint32_t tim_up_cnt;
    uint32_t pwm_period_us;
    uint32_t pwm_duty_us;
    uint32_t req_duty;
  }pwm_capture_t;
/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
#define MAX_TIM_DUTY (TIM4->ARR+1)
#define MAX_DUTY 2000
  /* USER CODE END Private defines */

  void MX_TIM1_Init(void);
  void MX_TIM2_Init(void);
  void MX_TIM4_Init(void);

  /* USER CODE BEGIN Prototypes */
  void set_pwm_duty(int32_t duty);
  uint32_t get_pwm_duty(void);

  extern volatile pwm_capture_t pwm_capt;
  /* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __TIM_H__ */

