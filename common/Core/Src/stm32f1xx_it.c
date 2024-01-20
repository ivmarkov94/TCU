/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f1xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "stm32f1xx_it.h"
#include "main.h"
#include "usart_console.h"
#include "internal_flash.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
uint32_t app_call_st __attribute__((section(".noinit")));
uint32_t app_call_cnt __attribute__((section(".noinit")));
static uint32_t heap_overflow  = 0;
static uint32_t stack_overflow = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
#ifdef CHECK_STACK_HEAP
void heap_check_handler();
void stack_check_handler();
#endif /* CHECK_STACK_HEAP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */
__WEAK void tim2_irq_callback(void);
__WEAK void tim1_up_irq_callback(void);
__WEAK void tim1_cc_irq_callback(void);
__WEAK void dma_irq_callback(void);
void tim2_irq_callback(void)
{
  ;
}
void tim1_up_irq_callback(void)
{
  ;
}
void tim1_cc_irq_callback(void)
{
  ;
}
void dma_irq_callback(void)
{
  ;
}
/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M3 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */
  static uint8_t  error_code[2] = {0xFF,0xFF};
  if(*((uint8_t*)REBOOT_INFO_AREA) == 0xFF )
  {
    if(stack_overflow || (__get_MSP() <= SRAM_BASE))
    {
      error_code[0] = STACK_OVERFLOW_ERROR;
    }else if(heap_overflow){
      error_code[0] = HEAP_OVERFLOW_ERROR;
    }else{
      error_code[0] = UNKNOWN_ERROR;
    }
    WRITE_2BYTES_TO_FLASH(REBOOT_INFO_AREA, error_code)
  }
  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */
  extern volatile uint32_t timer_ms;
  static bool app_call_init = false;

  timer_ms += 1U;
  
  if(!app_call_init)
  {
    if(get_time_ms() > 5000)
    {
      uint16_t* u16ptr = (uint16_t*)&app_call_cnt;
      app_call_init = true;
      u16ptr[0] = RESET_SIGNATURE;
      u16ptr[1] = 0;
    }
  }
#ifdef CHECK_STACK_HEAP
  static uint32_t check_timer_ms;
  if((check_timer_ms+50/* ms */)<timer_ms)
  {
    check_timer_ms = timer_ms;
    heap_check_handler();
    stack_check_handler();
  }
#endif /* CHECK_STACK_HEAP */
  /* USER CODE END SysTick_IRQn 0 */

  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F1xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f1xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles DMA1 channel1 global interrupt.
  */
void DMA1_Channel1_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel1_IRQn 0 */
  dma_irq_callback();
  /* USER CODE END DMA1_Channel1_IRQn 0 */

  /* USER CODE BEGIN DMA1_Channel1_IRQn 1 */

  /* USER CODE END DMA1_Channel1_IRQn 1 */
}

/**
  * @brief This function handles TIM1 update interrupt.
  */
void TIM1_UP_IRQHandler(void)
{
  /* USER CODE BEGIN TIM1_UP_IRQn 0 */
  tim1_up_irq_callback();
  /* USER CODE END TIM1_UP_IRQn 0 */
  /* USER CODE BEGIN TIM1_UP_IRQn 1 */

  /* USER CODE END TIM1_UP_IRQn 1 */
}

/**
  * @brief This function handles TIM1 capture compare interrupt.
  */
void TIM1_CC_IRQHandler(void)
{
  /* USER CODE BEGIN TIM1_CC_IRQn 0 */
  tim1_cc_irq_callback();
  /* USER CODE END TIM1_CC_IRQn 0 */
  /* USER CODE BEGIN TIM1_CC_IRQn 1 */

  /* USER CODE END TIM1_CC_IRQn 1 */
}

/**
  * @brief This function handles TIM2 global interrupt.
  */
void TIM2_IRQHandler(void)
{
  /* USER CODE BEGIN TIM2_IRQn 0 */
  tim2_irq_callback();
  /* USER CODE END TIM2_IRQn 0 */
  /* USER CODE BEGIN TIM2_IRQn 1 */

  /* USER CODE END TIM2_IRQn 1 */
}

/**
  * @brief This function handles USART3 global interrupt.
  */
void USART3_IRQHandler(void)
{
  /* USER CODE BEGIN USART3_IRQn 0 */
  if(LL_USART_IsActiveFlag_RXNE(USART3))
  {
    LL_USART_ClearFlag_RXNE(USART3);
    uart3.rx_fifo_upd_ms = get_time_ms();
    if(ring_get_count(&uart3.rx_fifo) < uart3.rx_fifo.size)
    {
      ring_push((uint8_t)USART3->DR, &uart3.rx_fifo);
    }else{
      printf("Error rx_buf overflow, reset rx buffer"NLINE);
      ring_clear(&uart3.rx_fifo);
    }
  }
  if(LL_USART_IsActiveFlag_TC(USART3))
  {
    LL_USART_ClearFlag_TC(USART3);
    if(ring_get_count(&uart3.tx_fifo)>0u)
    {
      LL_USART_TransmitData8(USART3, ring_pop(&uart3.tx_fifo));
    }else{
      uart3.transfer_completed = true;
    }
  }
  if(LL_USART_IsActiveFlag_ORE(USART3))
  {
    LL_USART_ClearFlag_ORE(USART3);
  }
  /* USER CODE END USART3_IRQn 0 */
  /* USER CODE BEGIN USART3_IRQn 1 */

  /* USER CODE END USART3_IRQn 1 */
}

/* USER CODE BEGIN 1 */
#ifdef CHECK_STACK_HEAP
#define START_STACK_FILLED_SIZE 32/* bytes */
#define INIT_BYTE 0x4C
const uint32_t init_word = 0x4C4C4C4C;

extern uint32_t _heap_start_addr;
extern uint32_t _heap_end_addr;
extern uint32_t _estack;/* stack top */

static uint32_t stack_occupied = START_STACK_FILLED_SIZE;
static uint32_t heap_occupied  = 0;
static uint32_t heap_size;
static uint32_t stack_size;
volatile uint8_t* hp_ptr;
volatile uint8_t* st_ptr;
void init_heap_check()
{
  __disable_irq();
  heap_size = (uint32_t)&_heap_end_addr - (uint32_t)&_heap_start_addr;
  hp_ptr =  (volatile uint8_t*)&_heap_start_addr;
  memset((uint8_t*)hp_ptr, INIT_BYTE, heap_size);
  __enable_irq();
}

void init_stack_check()
{
  __disable_irq();
  stack_size = (uint32_t)&_estack - (uint32_t)&_heap_end_addr;
  st_ptr = (uint8_t*)((uint32_t)&_estack-stack_size);
  memset((uint8_t*)st_ptr, INIT_BYTE, (stack_size-START_STACK_FILLED_SIZE));
  __enable_irq();
}

void heap_check_handler()
{
  if(heap_size!=0)
  {
    if(heap_occupied < heap_size)
    {
      while((*((uint32_t*)((uint32_t)&_heap_start_addr+heap_occupied)) != init_word))
      {
        heap_occupied++;/* bytes */
      }
    }else{
      heap_overflow = 1;
      HardFault_Handler();
    }
  }
}

void stack_check_handler()
{
  if(stack_occupied < stack_size)
  {
    while(*((uint32_t*)((uint32_t)&_estack-stack_occupied)) != init_word)
    {
      stack_occupied++;/* bytes */
    }
  }else{
    stack_overflow = 1;
    HardFault_Handler();
  }
}

void stack_heap_print_st(void)
{
  printf("stack:%ldB/%ldB=>%3ld%% heap:%ldB/%ldB=>%3ld%%"NLINE,stack_occupied,stack_size,(stack_occupied*100)/stack_size, heap_occupied,heap_size,(heap_occupied*100)/heap_size);
}
#endif /* CHECK_STACK_HEAP */
/* USER CODE END 1 */
