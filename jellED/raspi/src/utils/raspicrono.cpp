#include "utils/raspicrono.h"
#include <time.h>

unsigned long RaspiCrono::currentTimeMicros() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long us = ((long)ts.tv_sec * 1000000) + ((long)ts.tv_nsec / 1000000);
    return us;
}
