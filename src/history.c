#include "history.h"

#include <assert.h>

/* Append one sample to the ring buffer */
void HistoryRecord(History *h, int population, int food, float avgSpeed, float avgMetabolism) {
    assert(h != NULL);

    h->population[h->head]    = population;
    h->food[h->head]          = food;
    h->avgSpeed[h->head]      = avgSpeed;
    h->avgMetabolism[h->head] = avgMetabolism;

    h->head = (h->head + 1) % HISTORY_LEN;
    if (h->count < HISTORY_LEN) {
        h->count++;
    }
}
