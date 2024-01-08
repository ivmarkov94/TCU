#include "internal_flash.h"
#include "stm32f103xb.h"

void internal_flash_unlock(void)
{
    if(FLASH->CR & FLASH_CR_LOCK)
    {
        FLASH->KEYR = 0x45670123;
        FLASH->KEYR = 0xCDEF89AB;
    }
}

void internal_flash_lock(void)
{
    if((FLASH->CR & FLASH_CR_LOCK) !=0)
    {
        FLASH->CR |= FLASH_CR_LOCK;
    }
}

void internal_flash_Erase(uint32_t pageAddress) {
	while (FLASH->SR & FLASH_SR_BSY)
    {
		;
    }
	if (FLASH->SR & FLASH_SR_EOP) {
		FLASH->SR |= FLASH_SR_EOP;
	}

	FLASH->CR |= FLASH_CR_PER;
	FLASH->AR = pageAddress;
	FLASH->CR |= FLASH_CR_STRT;
	while (!(FLASH->SR & FLASH_SR_EOP))
    {
       ;
    }
	FLASH->SR |= FLASH_SR_EOP;
	FLASH->CR &= ~FLASH_CR_PER;
}

void internal_flash_Write(uint8_t* data, uint32_t address, uint32_t count) {
	while (FLASH->SR & FLASH_SR_BSY){
        ;
    }
	if (FLASH->SR & FLASH_SR_EOP) {
		FLASH->SR |= FLASH_SR_EOP;
	}

	FLASH->CR |= FLASH_CR_PG;

	for (uint32_t i = 0; i < count; i += 2) {
		*(volatile uint16_t*)(address + i) = (((uint16_t)data[i + 1]) << 8) + data[i];
		while (!(FLASH->SR & FLASH_SR_EOP)){
            ;
        }
		FLASH->SR |= FLASH_SR_EOP;
		wdgs_refresh();
	}

	FLASH->CR &= ~(FLASH_CR_PG);
}