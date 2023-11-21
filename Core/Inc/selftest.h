#ifndef SELFTEST
#define SELFTEST
#include "device.h" 
#define DMAX_MAXTIME 5000 /* ms */

bool selftest_get_state(void);
void selftest_short_test(void);
void selftest_whole_test(void);
void selftest_print_errors(void);
void selftest_set_print_all(bool val);
bool selftest_check_temperature_feedback(void);
#endif