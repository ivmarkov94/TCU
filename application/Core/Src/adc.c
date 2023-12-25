/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.c
  * @brief   This file provides code for the configuration
  *          of the ADC instances.
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
#include "adc.h"

/* USER CODE BEGIN 0 */
#include "adc_private.h"
#include "ntcg203nh103jtds.h"
#include "ntc_bmw_f30.h"
#include "main.h"
#include "tim.h"
#include <stdio.h>
volatile uint16_t dma_buffer[ADC_DMA_BUFFER_SIZE];
volatile static adc_t adc = {0};

void adc_check_buf_settings(void);

void tim2_upd_callback(void)/* every 50ms */
{
    adc_start();
}

void dma_tc_callback(void)
{
    adc.filter_step++;
    if(adc.filter_step >= ADC_AVG_NUM)
    {
        adc_upd_process();
        adc.filter_step=0;
    }else{
        LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_1, (uint32_t)&dma_buffer[adc.filter_step*adc_ch_num]);
        LL_ADC_REG_StartConversionSWStart(ADC1);
    }
}

void dma_te_callback(void)
{
    adc.state = adc_st_idle;
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);
    adc_start();
}

void adc_upd_process(void)
{
    for (uint32_t i = 0U; i < adc_ch_num; i++)
    {
        adc.ch_raw_val[i] = 0u;
        for (uint32_t j = 0u; j < ADC_AVG_NUM; j++)
        {
            adc.ch_raw_val[i] += dma_buffer[i+j*adc_ch_num];
        }
        adc.ch_raw_val[i] = adc.ch_raw_val[i]>>ADC_AVG_2NUM;
        adc.ch_mv_val[i]  = (uint32_t)adc.ch_raw_val[i]*ADC_RES;
    }   
/*     for(int i = 0; i < (sizeof(dma_buffer)/sizeof(uint16_t)-adc_ch_num); i++)
    {
        dma_buffer[i]=0;
    } */
    adc.conv_time_us = get_time_us() - adc.start_time_us;
    adc.state = adc_st_idle;
}

void adc_start(void)
{
    if(adc.state == adc_st_idle)
    {
        adc.filter_step = 0;
        LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_1, (uint32_t)&dma_buffer[0]);
        LL_ADC_REG_StartConversionSWStart(ADC1);
        adc.state = adc_st_conversion;
        adc.start_time_us = get_time_us();
    }
}

int32_t adc_get_val(adc_channel_t adc_ch, bool avg)
{
    int32_t res = 0;
    uint32_t adc_val_mv, buf_start_indx;
    if(avg)
    {
        adc_val_mv = adc.ch_mv_val[adc_ch];
    }else{
        buf_start_indx = adc.filter_step;
        buf_start_indx = buf_start_indx==0? ((ADC_AVG_NUM-1U)*adc_ch_num):((buf_start_indx-1U)*adc_ch_num);
        adc_val_mv = (uint32_t)(dma_buffer[buf_start_indx+adc_ch]*ADC_RES);
    }
        
    switch (adc_ch)
    {
    case adc_ch_ntc_pcb:
    case adc_ch_ntc_amb:
        res = 10*search_table32 ((int16_t)(adc_val_mv*10), table_ntcg203nh103jtdc, ARRAY_SIZE(table_ntcg203nh103jtdc));/* mC */
        break;

    case adc_ch_div12v:
        res = adc_val_mv * DIV12V;
        break;

    case adc_ch_cur_sens:
        res = adc_val_mv*1000 / DEVCS;
        break;

    case adc_ch_int_temp:
        res = (((int32_t)1430 - (int32_t)adc_val_mv)/4.3)*(int32_t)1000 + (int32_t)25000;
        break;

    case adc_ch_vref:
        res = adc_val_mv;
        break;

    case adc_ch_divm3v:
        res = adc_val_mv * DIVm3V + ADIm3V;
        break;
    
    case adc_ch_div5v:
        res = adc_val_mv * DIV5V;
        break;

    case adc_ch_t_eng:
        {
            uint32_t t_val = adc_val_mv/GAIN_TENG;
            res = 10*search_table32 ((int16_t)t_val*10u, table_ntc_bmw_f30_coolant, ARRAY_SIZE(table_ntc_bmw_f30_coolant));/* mC */
            break;
        }
    
    default:
        break;
    }
    return res;
}

int32_t adc_get_raw_val(adc_channel_t adc_ch)
{
    return adc.ch_mv_val[adc_ch];
}

int32_t adc_get_special_val(adc_channel_t adc_ch)
{
    int32_t res = 0;
    switch (adc_ch)
    {
    case adc_ch_t_eng:/*amplifier input differential signal in mV */
        {
            res = adc.ch_mv_val[adc_ch]/GAIN_TENG;
            break;
        }
    default:
        break;
    }
    return res;
}

/* USER CODE END 0 */

/* ADC1 init function */
void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  LL_ADC_InitTypeDef ADC_InitStruct = {0};
  LL_ADC_CommonInitTypeDef ADC_CommonInitStruct = {0};
  LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_ADC1);

  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
  /**ADC1 GPIO Configuration
  PA0-WKUP   ------> ADC1_IN0
  PA1   ------> ADC1_IN1
  PA2   ------> ADC1_IN2
  PA4   ------> ADC1_IN4
  PA5   ------> ADC1_IN5
  PA6   ------> ADC1_IN6
  PA7   ------> ADC1_IN7
  */
  GPIO_InitStruct.Pin = ntc_amb_Pin|div12v_Pin|cur_sens_Pin|ntc_pcb_Pin
                          |div5v_Pin|divm3v_Pin|Teng_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* ADC1 DMA Init */

  /* ADC1 Init */
  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_1, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

  LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_1, LL_DMA_PRIORITY_MEDIUM);

  LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MODE_NORMAL);

  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_1, LL_DMA_PERIPH_NOINCREMENT);

  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MEMORY_INCREMENT);

  LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_1, LL_DMA_PDATAALIGN_HALFWORD);

  LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MDATAALIGN_HALFWORD);

  /* USER CODE BEGIN ADC1_Init 1 */
  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
  ADC_InitStruct.SequencersScanMode = LL_ADC_SEQ_SCAN_ENABLE;
  LL_ADC_Init(ADC1, &ADC_InitStruct);
  ADC_CommonInitStruct.Multimode = LL_ADC_MULTI_INDEPENDENT;
  LL_ADC_CommonInit(__LL_ADC_COMMON_INSTANCE(ADC1), &ADC_CommonInitStruct);
  ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE;
  ADC_REG_InitStruct.SequencerLength = LL_ADC_REG_SEQ_SCAN_ENABLE_9RANKS;
  ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
  ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_SINGLE;
  ADC_REG_InitStruct.DMATransfer = LL_ADC_REG_DMA_TRANSFER_UNLIMITED;
  LL_ADC_REG_Init(ADC1, &ADC_REG_InitStruct);

  /** Configure Regular Channel
  */
  LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_0);
  LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_0, LL_ADC_SAMPLINGTIME_239CYCLES_5);

  /** Configure Regular Channel
  */
  LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_2, LL_ADC_CHANNEL_1);
  LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_1, LL_ADC_SAMPLINGTIME_239CYCLES_5);

  /** Configure Regular Channel
  */
  LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_3, LL_ADC_CHANNEL_2);
  LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_2, LL_ADC_SAMPLINGTIME_239CYCLES_5);

  /** Configure Regular Channel
  */
  LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_4, LL_ADC_CHANNEL_TEMPSENSOR);
  LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_TEMPSENSOR, LL_ADC_SAMPLINGTIME_239CYCLES_5);
  LL_ADC_SetCommonPathInternalCh(__LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_PATH_INTERNAL_TEMPSENSOR);

  /** Configure Regular Channel
  */
  LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_5, LL_ADC_CHANNEL_VREFINT);
  LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_VREFINT, LL_ADC_SAMPLINGTIME_239CYCLES_5);
  LL_ADC_SetCommonPathInternalCh(__LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_PATH_INTERNAL_VREFINT);

  /** Configure Regular Channel
  */
  LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_6, LL_ADC_CHANNEL_4);
  LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_4, LL_ADC_SAMPLINGTIME_239CYCLES_5);

  /** Configure Regular Channel
  */
  LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_7, LL_ADC_CHANNEL_5);
  LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_5, LL_ADC_SAMPLINGTIME_239CYCLES_5);

  /** Configure Regular Channel
  */
  LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_8, LL_ADC_CHANNEL_6);
  LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_6, LL_ADC_SAMPLINGTIME_239CYCLES_5);

  /** Configure Regular Channel
  */
  LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_9, LL_ADC_CHANNEL_7);
  LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_7, LL_ADC_SAMPLINGTIME_239CYCLES_5);
  /* USER CODE BEGIN ADC1_Init 2 */
  LL_DMA_ConfigTransfer(DMA1,
                        LL_DMA_CHANNEL_1,
                        LL_DMA_DIRECTION_PERIPH_TO_MEMORY |
                        LL_DMA_MODE_CIRCULAR              |
                        LL_DMA_PERIPH_NOINCREMENT         |
                        LL_DMA_MEMORY_INCREMENT           |
                        LL_DMA_PDATAALIGN_HALFWORD        |
                        LL_DMA_MDATAALIGN_HALFWORD        |
                        LL_DMA_PRIORITY_HIGH               );
  LL_DMA_ConfigAddresses(DMA1,
                         LL_DMA_CHANNEL_1,
                         LL_ADC_DMA_GetRegAddr(ADC1, LL_ADC_DMA_REG_REGULAR_DATA),
                         (uint32_t)&dma_buffer,
                         LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
  LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_1, adc_ch_num);
  LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_1);
  LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_1);
  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);
  LL_ADC_Enable(ADC1);
  
  uint32_t wait_loop_index = ((LL_ADC_DELAY_ENABLE_CALIB_ADC_CYCLES * 32) >> 1);
  while(wait_loop_index != 0)
  {
    wait_loop_index--;
  }
  LL_ADC_StartCalibration(ADC1);
  while (LL_ADC_IsCalibrationOnGoing(ADC1) != 0) {;}
  
  adc_check_buf_settings();
  /* USER CODE END ADC1_Init 2 */

}

/* USER CODE BEGIN 1 */
/************************* check ADC settings *************************/
void adc_check_buf_settings(void){
    if((((ADC1->SQR1>>ADC_SQR1_L_Pos)&0xF)+1) != adc_ch_num)
    {
        Error_Handler();
    }
}
/* USER CODE END 1 */
