#ifndef TABLE_SEARCH_H
#define TABLE_SEARCH_H

#include <stdint.h>
typedef struct{
    int32_t req_key;
    int32_t res_value;
}search_table32_t;/* Table should be sorted from 0 to max by key */

/* Searching element in the sorted table */
int32_t search_table32(int32_t key, const search_table32_t table32[], uint32_t size);

#endif /* TABLE_SEARCH_H */