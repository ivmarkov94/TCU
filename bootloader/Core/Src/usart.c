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
uart_t uart3;
uint8_t tx_buffer[UART_TX_BUF];
ring_buffer_t tx_ring_buf = {.buffer = tx_buffer, .size = UART_TX_BUF, .flag_overflow = 0, .flag_error_pop = 0};

/* static int32_t get_int_arg(uint8_t* ptr); */
/* static float get_float_arg(uint8_t* ptr); */
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

void receive_cmd(void)
{
  if(uart_is_str_ready())
  {
    if(IS_CMD_RECIVED("wdt")) 
    {
      while(1)
      {
        ;
      }
    }else
    {
      printf("Unknowned cmd %s"NLINE"Cmd list:"NLINE"wdt"NLINE,(char*)&uart3.rx_buffer[0]);
    }
    uart_rx_buff_reset();
  }
}

/* static int32_t get_int_arg(uint8_t* ptr)
{
  int32_t val = atoi((const char*)ptr);
  printf("Updated to = %ld"NLINE,val);
  return val;
} */

/* static float get_float_arg(uint8_t* ptr)
{
  float val = atof((const char*)ptr);
  printf("Updated to = %5.3f"NLINE,val);
  return val;
} */
/* USER CODE END 1 */
