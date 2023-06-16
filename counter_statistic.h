#ifndef COUNTER_STATISTIC_H
#define COUNTER_STATISTIC_H

#include <sys/time.h>

typedef struct {
    int* static_requests_counter;
    int* dynamic_requests_counter;
    struct timeval* wait_time;
    struct timeval* arrival_time;
} Counter_statistic;

void initCounterStatistic(Counter_statistic* detail, int size);

#endif
