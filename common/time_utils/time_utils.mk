TIME_UTILS_PATH := $(dir $(lastword $(MAKEFILE_LIST)))

C_SOURCES  += $(TIME_UTILS_PATH)/src/time_utils.c
C_INCLUDES += -I$(TIME_UTILS_PATH)/src 

