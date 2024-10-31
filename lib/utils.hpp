#ifndef UTILS_H
#define UTILS_H

#include <limits.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>

#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

void   time_start(time_t *t_time, clock_t *c_clock);
double time_stop(time_t t_time, clock_t c_clock);


#endif
