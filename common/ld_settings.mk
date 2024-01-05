APL_START_ADDRES = 0x8004000
APL_FW_MAX_SIZE  = 47K
BL_FW_MAX_SIZE   = 16K

FLASH_START_ADDRES = 0x8000000
FLASH_MAX_SIZE   = 63K 
#we will use last page 1K to REBOOT_INFO
ifeq ($(APL_FOR_BL), Y)
	C_DEFS += -DUSER_VECT_TAB_ADDRESS
	C_DEFS += -DAPL_START_ADDRES=$(APL_START_ADDRES)
	FW_START_ADDRES = $(APL_START_ADDRES)
	FW_MAX_SIZE = $(APL_FW_MAX_SIZE)
	FW_NAME = APPLICATION_FOR_BL
else ifeq ($(BL_FW), Y)
	C_DEFS += -DAPL_START_ADDRES=$(APL_START_ADDRES)
	FW_START_ADDRES = $(FLASH_START_ADDRES)
	FW_MAX_SIZE = $(BL_FW_MAX_SIZE)
	FW_NAME = BOOTLOADER
else
	FW_START_ADDRES = $(FLASH_START_ADDRES)
	FW_MAX_SIZE = $(FLASH_MAX_SIZE)
	FW_NAME = APPLICATION
endif

# For LD file preprocessoring. 
C_DEFS += -DFW_MAX_SIZE=$(FW_MAX_SIZE)  
C_DEFS += -DFW_START_ADDRES=$(FW_START_ADDRES) 
# For LD file preprocessoring  