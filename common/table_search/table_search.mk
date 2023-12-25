TABLE_SEARCH_PATH := $(dir $(lastword $(MAKEFILE_LIST)))

C_SOURCES  += $(TABLE_SEARCH_PATH)/src/table_search.c
C_INCLUDES += -I$(TABLE_SEARCH_PATH)/src 




