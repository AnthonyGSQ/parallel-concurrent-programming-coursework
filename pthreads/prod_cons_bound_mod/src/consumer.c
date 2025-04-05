// Copyright 2021 Jeisson Hidalgo-Cespedes <jeisson.hidalgo@ucr.ac.cr> CC-BY-4

#include <stdbool.h>
#include <stdio.h>

#include "common.h"
#include "consumer.h"

void* consume(void* data) {
  simulation_t* simulation = (simulation_t*)data;

  while (simulation->consumed_count < simulation->unit_count) {
    ++simulation->consumed_count;
    size_t value = 0;
    queue_dequeue(&simulation->queue, &value);
    printf("\tConsuming %zu\n", value);
    usleep(1000 * random_between(simulation->consumer_min_delay
      , simulation->consumer_max_delay));
  }

  return NULL;
}
