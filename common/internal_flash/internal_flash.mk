INTERNAL_FLASH_PATH := $(dir $(lastword $(MAKEFILE_LIST)))

C_INCLUDES += -I$(INTERNAL_FLASH_PATH)/src 
C_SOURCES  += $(INTERNAL_FLASH_PATH)/src/internal_flash.c
