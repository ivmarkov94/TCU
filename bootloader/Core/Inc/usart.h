/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
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
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "stdio.h"
#include <string.h>
#define  UART_RX_BUF ((uint32_t)0x2000 + (uint32_t)0x10)
#define  UART_TX_BUF 200
#define  CONSOLE_HELP_CMD NLINE

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
typedef enum{
  dbg_var_base,
  dbg_var_adc_channels_scaled_part1,
  dbg_var_adc_channels_scaled_part2,
  dbg_var_adc_channels_raw_part1,
  dbg_var_adc_channels_raw_part2,
  dbg_var_amplifier_dif_input
}dbg_var_e;
/* USER CODE END Private defines */

void MX_USART3_UART_Init(void);

/* USER CODE BEGIN Prototypes */
uint8_t console_cmd(void);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

