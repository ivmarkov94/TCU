
#ifndef ADC_PRIVATE_H
#define ADC_PRIVATE_H
#include "main.h"
#include "adc.h"
#define VDD        3318 /* mV */ 
#define ADC_RES    VDD/4095.f /* mv/bit */
#define DIV12V     5.4818 
#define DIV5V      2 
#define DIVm3V     2.6 
#define ADIm3V     -1.6*VDD
#define DEVCS      216.572 /* mv/A */
#define GAIN_TENG  2.003f 

#define ADC_AVG_2NUM 6  /* number of 2 in ADC_AVG_NUM*/
#define ADC_AVG_NUM  2*2*2*2*2*2 /* multiple of 2 */
#if ((((ADC_AVG_NUM-1)&ADC_AVG_NUM)!=0) || ((ADC_AVG_NUM>>ADC_AVG_2NUM)!=1))
    #error("ADC_AVG_NUM Should be multiple of 2")
#endif
#define ADC_DMA_BUFFER_SIZE adc_ch_num*ADC_AVG_NUM

typedef enum{
    adc_st_idle,
    adc_st_conversion
}adc_state_t;

typedef struct {
    adc_state_t state;
    uint32_t ch_raw_val[adc_ch_num];
    uint32_t ch_mv_val[adc_ch_num];
    uint64_t start_time_us;
    uint64_t conv_time_us;
    uint32_t filter_step;
}adc_t;

#endif /* ADC_PRIVATE_H */
