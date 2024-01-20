#include "bl_logic.h"
#include "usart_console.h"
#include "internal_flash.h"
static uint32_t flash_addr = APL_START_ADDRES;
static uint32_t frame_size = 0;

static void flash_fw_frame(void);

void set_flash_address(uint32_t addr)
{
    if((addr >= APL_START_ADDRES) && (addr < FLASH_END))
    {
        flash_addr = addr;
        printf("%ld"NLINE,flash_addr);
    }else{
        printf(BL_CMD_ERROR_ST NLINE);
    }
}

void set_flash_frame_size(uint32_t size)
{
    if(((flash_addr + size) <= FLASH_END))
    {
        frame_size = size;
        printf("%ld"NLINE,frame_size);
    }else{
        printf(BL_CMD_ERROR_ST NLINE);
    }
}

void erase_flash(void)
{
    uint32_t flash_addr = APL_START_ADDRES;
    uint32_t flash_page_offset = PAGE_SIZE;

    __disable_irq();
    internal_flash_unlock();
    while(flash_addr < FLASH_END)
    {
        wdgs_refresh();
        internal_flash_Erase(flash_addr);
        flash_addr += flash_page_offset;
    }
    internal_flash_lock();
    __enable_irq();
    printf(BL_CMD_DONE_ST NLINE);
}

void get_fw_frame(void)
{
    uint32_t time_ms;
    int8_t res_st = 0;
    
    ring_clear(&uart3.rx_fifo);
    printf(BL_CMD_READY_ST NLINE);
    time_ms = get_time_ms();

    while(!uart_is_cmd_ready() || !time_elapsed_ms(time_ms, WAIT_TRANSMIT_START_MS))
    {
        if(ring_get_count(&uart3.rx_fifo) >= (frame_size+CRC_SIZE))
        {
            if(hex_crc_check(uart3.rx_fifo.buffer, (frame_size+CRC_SIZE)))
            {
                res_st = 1; 
            }else{
                res_st = -1;
            }
            break;
        }
        wdgs_refresh();
    }

    if(res_st > 0){
        printf(BL_CMD_DONE_ST NLINE);
        flash_fw_frame();
    }else if(res_st < 0){
        printf(BL_CMD_CRC_ERR_ST NLINE);
    }else{
        printf(BL_CMD_ERROR_ST NLINE);
    }
    ring_clear(&uart3.rx_fifo);
}

static void flash_fw_frame(void)
{
    __disable_irq();
    internal_flash_unlock();
    internal_flash_Write(uart3.rx_fifo.buffer, flash_addr, frame_size);
    internal_flash_lock();
    __enable_irq();
    printf(BL_CMD_DONE_ST NLINE);
}