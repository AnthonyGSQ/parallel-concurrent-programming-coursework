// Copyright 2021 Jeisson Hidalgo-Cespedes <jeisson.hidalgo@ucr.ac.cr> CC-BY-4

#include <stdio.h>

#include "common.h"
#include "producer.h"

void* produce(void* data) {
  // const private_data_t* private_data = (private_data_t*)data;
  simulation_t* simulation = (simulation_t*)data;

  while (simulation->next_unit < simulation->unit_count) {
    size_t my_unit = ++simulation->next_unit;
    usleep(1000 * random_between(simulation->producer_min_delay
      , simulation->producer_max_delay));
    queue_enqueue(&simulation->queue, my_unit);
    printf("Produced %zu\n", my_unit);
  }

  return NULL;
}
