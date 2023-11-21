/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef RING_BUFFER_H
#define RING_BUFFER_H

/* Includes ------------------------------------------------------------------*/
#include "device.h"
typedef struct
{
     volatile uint8_t included;
     uint8_t *buffer;
     uint8_t idx_in;
     uint8_t idx_out;
     uint8_t size;
     uint8_t flag_overflow;
     uint8_t flag_error_pop;
}ring_buffer_t;

/* Prototype -----------------------------------------------------------------*/
void     ring_push(uint8_t symbol, ring_buffer_t *buf);
uint8_t  ring_pop(ring_buffer_t *buf);
uint8_t  ring_get_count(ring_buffer_t *buf);
uint8_t  ring_show_symbol(uint8_t symbol_number ,ring_buffer_t *buf);
void     ring_clear(ring_buffer_t* buf);
#endif /* RING_BUFFER_H */
