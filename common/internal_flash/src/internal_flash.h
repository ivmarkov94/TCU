#ifndef INTERNAL_FLASH_H
#define INTERNAL_FLASH_H
#include "device.h"

/* Writing to flash without using call stack */
#define WRITE_2BYTES_TO_FLASH(flash_addr, data_ptr) \
{                                           \
    static uint32_t addr;                   \
    static uint8_t  data[2], i;             \
    i=0;                                    \
    addr = flash_addr;                      \
    data[0] = data_ptr[0];                  \
    data[1] = data_ptr[1];                  \
    FLASH->KEYR = 0x45670123;               \
    FLASH->KEYR = 0xCDEF89AB;               \
    while (FLASH->SR & FLASH_SR_BSY){;}     \
    if (FLASH->SR & FLASH_SR_EOP) {         \
      FLASH->SR |= FLASH_SR_EOP;            \
    }                                       \
    FLASH->CR |= FLASH_CR_PG;               \
    for (i = 0; i < 2; i += 2) {            \
      *(volatile uint16_t*)(addr + i) = (((uint16_t)data[i + 1]) << 8) + data[i];\
      while (!(FLASH->SR & FLASH_SR_EOP)){;}\
      FLASH->SR |= FLASH_SR_EOP;            \
    }                                       \
    FLASH->CR &= ~(FLASH_CR_PG);            \
} 

void internal_flash_lock(void);
void internal_flash_unlock(void);
void internal_flash_Erase(uint32_t pageAddress);
void internal_flash_Write(uint8_t* data, uint32_t address, uint32_t count);
#endif /* INTERNAL_FLASH_H */
