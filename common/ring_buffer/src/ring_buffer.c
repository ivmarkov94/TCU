
/* Includes ------------------------------------------------------------------*/
#include "ring_buffer.h"

/* example */
/* ring_buffer_t tx_ring_buf = {.buffer = tx_buffer, .size = TX_BUFFER_SIZE, .flag_overflow = 0, .flag_error_pop = 0}; */
void ring_push(uint8_t symbol, ring_buffer_t* buf)//valid
{ 
    buf->buffer[buf->idx_in++] = symbol;
    if (buf->idx_in >= buf->size) buf->idx_in = 0;
    if (buf->idx_in == buf->idx_out) 
    {
      buf->flag_overflow = 1;
    }
}

uint8_t ring_pop(ring_buffer_t *buf)//valid
{    
    if(buf->idx_in == buf->idx_out)
    {
        if(buf->flag_overflow==0)buf->flag_error_pop=1; 
    }
    uint8_t retval = buf->buffer[buf->idx_out++];
    if (buf->idx_out >= buf->size) buf->idx_out = 0;
    return retval;
}

uint32_t ring_get_count(ring_buffer_t *buf)
{
    uint32_t retval = 0;
    uint32_t in  = buf->idx_in;
    uint32_t out = buf->idx_out;
    if (in < out) retval = in + (buf->size - out);
    else retval = in - out;
    return retval;
}

/*symbol_number = 0 to size*/
uint8_t ring_show_symbol(uint32_t symbol_number ,ring_buffer_t *buf)
{
    uint32_t pointer = buf->idx_out + symbol_number;
    //int32_t  retval = -1;
    uint8_t retval = 0;
    if (symbol_number < ring_get_count(buf))
    {
        if (pointer >= buf->size) pointer -= buf->size;
        retval = buf->buffer[ pointer ] ;
    }
    return retval;
}

void ring_clear(ring_buffer_t* buf)
{
    buf->idx_in = 0;
    buf->idx_out = 0;
}

