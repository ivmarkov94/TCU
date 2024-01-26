#ifndef LOG_H
#define LOG_H
#include "device.h" 
#define LOG_BUFFER_SIZE 7200 /* 2 hours if 1 byte per second */
#define LOG_TIMEOUT 1000/* ms */
#define LOG_VERSION_1 "X [TEMPERATURE=80+X/10]" 
#define LOG_VERSION LOG_VERSION_1 

void log_handler(void);
void print_log_info(uint32_t line_num);
bool push_log_info(uint8_t data, uint8_t size);
#endif/* LOG_H */