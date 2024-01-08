/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef RING_BUFFER_H
#define RING_BUFFER_H

/* Includes ------------------------------------------------------------------*/
#include "device.h"
typedef struct
{
     uint8_t *buffer;
     uint32_t idx_in;
     uint32_t idx_out;
     uint32_t size;
     uint8_t flag_overflow;
     uint8_t flag_error_pop;
}ring_buffer_t;

/* Prototype -----------------------------------------------------------------*/
void     ring_push(uint8_t symbol, ring_buffer_t *buf);
uint8_t  ring_pop(ring_buffer_t *buf);
uint32_t ring_get_count(ring_buffer_t *buf);
uint8_t  ring_show_symbol(uint32_t symbol_number, ring_buffer_t *buf);
void     ring_clear(ring_buffer_t* buf);
#endif /* RING_BUFFER_H */
