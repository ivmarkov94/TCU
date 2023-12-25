RING_BUFFER_PATH := $(dir $(lastword $(MAKEFILE_LIST)))

C_INCLUDES += -I$(RING_BUFFER_PATH)/src 
C_SOURCES  += $(RING_BUFFER_PATH)/src/ring_buffer.c



