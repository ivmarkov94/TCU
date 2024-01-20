#ifndef BL_LOGIC_H
#define BL_LOGIC_H
#include "main.h"

#define CRC_SIZE 1
#define PAGE_SIZE 1024
#define FLASH_END (FLASH_BASE + FLASH_MAX_SIZE*1024)/*byte*/
#define WAIT_TRANSMIT_START_MS 90/* ms timeout between bytes frames */ * 3/* to safe */

#define BL_CMD_SET_FLASH_ADDR "_sfa"
#define BL_CMD_SET_FLASH_FRAME_SIZE "_sff"
#define BL_CMD_ERASE_FLASH "_efl"         
#define BL_CMD_GET_AND_FLASH_FW_FRAME "_gfw"       
#define BL_CMD_READY_ST "_rds"           
#define BL_CMD_DONE_ST "_dns"            
#define BL_CMD_ERROR_ST "_ers"
#define BL_CMD_CRC_ERR_ST "_crc"

void erase_flash(void);
void get_fw_frame(void);
void set_flash_address(uint32_t addr);
void set_flash_frame_size(uint32_t size);
#endif /* BL_LOGIC_H */

