#ifndef COUNTER_STATISTIC_C
#define COUNTER_STATISTIC_C

#include "counter_statistic.h"
#include <stdlib.h>

// Initialize Counter_stat
void initCounterStatistic(Counter_statistic* detail, int size) {
    detail->static_requests_counter = malloc(sizeof(int) * size);
    detail->dynamic_requests_counter = malloc(sizeof(int) * size);
    detail->wait_time = malloc(sizeof(struct timeval) * size);
    detail->arrival_time = malloc(sizeof(struct timeval) * size);

    for (int i = 0; i < size; i++) {
        detail->dynamic_requests_counter[i] = 0;
        detail->static_requests_counter[i] = 0;
        detail->wait_time[i] = (struct timeval){0};
        detail->arrival_time[i] = (struct timeval){0};
    }
}

#endif