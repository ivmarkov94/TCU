NTC_SENS_PATH := $(dir $(lastword $(MAKEFILE_LIST)))

C_INCLUDES += -I$(NTC_SENS_PATH)/inc 




