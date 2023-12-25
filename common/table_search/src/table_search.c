#include "table_search.h"

static int32_t search_lower_bound(int32_t key32, const search_table32_t table32 [], uint32_t table_size);

/* half division method */
static int32_t search_lower_bound(int32_t key32, const search_table32_t table32 [], uint32_t table_size) {
    int32_t indx_min = 0;
    uint32_t cnt = table_size;
    while (cnt > 0U) {
        uint32_t half = cnt >> 1U;
        int32_t indx_mid = indx_min + (int32_t)half;
        int32_t diff = table32 [indx_mid].req_key - key32;
        if (diff < 0) {
            indx_min = indx_mid + 1;
            cnt -= half + 1U;
        } else if(diff > 0){
            cnt = half;
        } else{
            indx_min = indx_mid;
            break;
        }
    }
    if ((indx_min >= (int32_t)table_size) || (table32 [indx_min].req_key != key32)) {
        indx_min = -indx_min - 1;
    }
    return indx_min;
}

int32_t search_table32(int32_t key, const search_table32_t table32[], uint32_t size) {
    int32_t result;
    int32_t key_indx = search_lower_bound (key, table32, size);
    if (key_indx >= 0) {
        result = table32 [key_indx].res_value;
    } else {
        key_indx = -key_indx - 1;
        if (key_indx == (int32_t)size) {
            result = table32[size - 1U].res_value;
        } else {
            if (key_indx == 0) {
                result = table32[0].res_value;
            } else {
                int32_t x1 = table32 [key_indx - 1].req_key;
                int32_t x2 = table32 [key_indx].req_key;
                int32_t y1 = table32 [key_indx - 1].res_value;
                int32_t y2 = table32 [key_indx].res_value;
                int32_t k = ((y2 - y1) * (int32_t)100) / (x2 - x1);
                result = y1 + (((key - x1) * k) / 100);
            }
        }
    }
    return result;
}
