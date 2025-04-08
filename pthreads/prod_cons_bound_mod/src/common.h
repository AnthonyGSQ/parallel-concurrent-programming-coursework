// Copyright 2021 Jeisson Hidalgo-Cespedes <jeisson.hidalgo@ucr.ac.cr> CC-BY-4

#ifndef COMMON_H
#define COMMON_H
#define _POSIX_C_SOURCE 200809L
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "queue.h"
// enumeramos los errores posibles
enum {
  ERR_NOMEM_SHARED = EXIT_FAILURE + 1,
  ERR_NOMEM_BUFFER,
  ERR_NO_ARGS,
  ERR_UNIT_COUNT,
  ERR_PRODUCER_COUNT,
  ERR_CONSUMER_COUNT,
  ERR_MIN_PROD_DELAY,
  ERR_MAX_PROD_DELAY,
  ERR_MIN_CONS_DELAY,
  ERR_MAX_CONS_DELAY,
  ERR_CREATE_THREAD,
};

typedef struct simulation {
  // parametros recibidos en argv[]
  size_t unit_count;
  size_t producer_count;
  size_t consumer_count;
  useconds_t producer_min_delay;
  useconds_t producer_max_delay;
  useconds_t consumer_min_delay;
  useconds_t consumer_max_delay;
  // la cola dinamica utilizada para el guardado de productos
  queue_t queue;
  pthread_mutex_t can_access_next_unit;
  size_t next_unit;
  sem_t can_consume;
  pthread_mutex_t can_access_consumed_count;
  size_t consumed_count;
} simulation_t;
// funcion utilizada para generar el tiempo que se tarda en
// producir y consumir un producto
useconds_t random_between(useconds_t min, useconds_t max);

#endif  // COMMON_H
