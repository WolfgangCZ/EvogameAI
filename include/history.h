#pragma once

#include "config.h"

#define HISTORY_LEN          2000
#define HISTORY_SAMPLE_TICKS  10   /* record one sample every N ticks */

/* Ring buffer of population and trait history for line charts */
typedef struct {
    int   population[HISTORY_LEN];
    int   food[HISTORY_LEN];
    float avgSpeed[HISTORY_LEN];
    float avgMetabolism[HISTORY_LEN];
    int   head;    /* index of next write position */
    int   count;   /* number of valid samples (0..HISTORY_LEN) */
} History;

/* Record one sample; advances head with modulo wrap, increments count up to HISTORY_LEN */
void HistoryRecord(History *h, int population, int food, float avgSpeed, float avgMetabolism);
