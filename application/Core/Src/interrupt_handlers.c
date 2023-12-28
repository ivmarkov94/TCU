#include "interrupt_handlers.h"
#include "device.h"
#include "adc.h"
#include "tim.h"
#include "adc_private.h"


void dma_irq_callback(void)
{
  if(LL_DMA_IsActiveFlag_TC1(DMA1) == 1)
  {
    dma_tc_callback();
    LL_DMA_ClearFlag_TC1(DMA1);
  }else{
    if(LL_DMA_IsActiveFlag_TE1(DMA1) == 1)
    {
      LL_DMA_ClearFlag_TE1(DMA1);
      dma_te_callback();
    }else{
      LL_GPIO_ResetOutputPin(led_GPIO_Port,led_Pin);
    }
  }
}

void tim1_up_irq_callback(void);
{
  if(LL_TIM_IsActiveFlag_UPDATE(TIM1))
  {
    LL_TIM_ClearFlag_UPDATE(TIM1);
    pwm_capt.tim_up_cnt++;
    if((pwm_capt.tim_up_cnt > 20))
    {
      if(LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_8) == 0){
        pwm_capt.req_duty = 2000;
      }else{
        pwm_capt.req_duty = 0;
      }
    }
  }
}

void tim1_cc_irq_callback(void)
{
  if(LL_TIM_IsActiveFlag_CC1(TIM1))
  {
    LL_TIM_ClearFlag_CC1(TIM1);
    TIM1->CNT = 0;
    pwm_capt.pwm_period_us = pwm_capt.tim_up_cnt*(TIM1->ARR) + TIM1->CCR1;
    pwm_capt.tim_up_cnt = 0;
    if(pwm_capt.pwm_period_us != 0)
    {
      pwm_capt.req_duty = (((pwm_capt.pwm_period_us - pwm_capt.pwm_duty_us)*2000)/pwm_capt.pwm_period_us);
    }

  }
  if(LL_TIM_IsActiveFlag_CC2(TIM1))
  {
    LL_TIM_ClearFlag_CC2(TIM1);
    pwm_capt.pwm_duty_us = pwm_capt.tim_up_cnt*(TIM1->ARR) + TIM1->CCR2;
  }
}

void tim2_irq_callback(void)
{
  if(LL_TIM_IsActiveFlag_UPDATE(TIM2)) /* every 4ms ADC support */
  {
    LL_TIM_ClearFlag_UPDATE(TIM2);
    tim2_upd_callback();
  }else{
      LL_GPIO_ResetOutputPin(led_GPIO_Port,led_Pin);
  }
}

