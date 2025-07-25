// Copyright 2021 Jeisson Hidalgo-Cespedes <jeisson.hidalgo@ucr.ac.cr> CC-BY-4

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/random.h>
#include <stdio.h>

#include "common.h"
#include "consumer.h"
#include "producer.h"
#include "simulation.h"
// funciones privadas, pues se encuentran en el .c
int analyze_arguments(simulation_t* simulation, int argc, char* argv[]);
int create_consumers_producers(simulation_t* simulation);
int join_threads(size_t count, pthread_t* threads);
// funcion encargada de reservar memoria para todo lo necesario como los mutex
// semaforos, struct y sus variables.
simulation_t* simulation_create() {
  simulation_t* simulation = (simulation_t*) calloc(1, sizeof(simulation_t));
  if (simulation) {
    // inicializamos todos los parametros
    simulation->unit_count = 0;
    simulation->producer_count = 0;
    simulation->consumer_count = 0;
    simulation->producer_min_delay = 0;
    simulation->producer_max_delay = 0;
    simulation->consumer_min_delay = 0;
    simulation->consumer_max_delay = 0;
    // creamos la cola
    queue_init(&simulation->queue);
    // creamos el mutex para el acceso a las unidades
    pthread_mutex_init(&simulation->can_access_next_unit, /* attr */ NULL);
    simulation->next_unit = 0;
    // semaforo para saber si existen productos para consumir
    sem_init(&simulation->can_consume, /* pshared */ 0, /* value */ 0);
    pthread_mutex_init(&simulation->can_access_consumed_count, /* attr */ NULL);
    simulation->consumed_count = 0;
  }
  return simulation;
}
// funcion encargada de liberar toda la memoria reservada
void simulation_destroy(simulation_t* simulation) {
  assert(simulation);
  pthread_mutex_destroy(&simulation->can_access_consumed_count);
  sem_destroy(&simulation->can_consume);
  pthread_mutex_destroy(&simulation->can_access_next_unit);
  queue_destroy(&simulation->queue);
  free(simulation);
}
// funcion encargada de correr la simulacion y medir el tiempo que le toma
// realizar todo el trabajo
int simulation_run(simulation_t* simulation, int argc, char* argv[]) {
  int error = analyze_arguments(simulation, argc, argv);
  if (error == EXIT_SUCCESS) {
    unsigned int seed = 0;
    getrandom(&seed, sizeof(seed), GRND_NONBLOCK);
    srand(seed);

    struct timespec start_time;
    clock_gettime(/*clk_id*/CLOCK_MONOTONIC, &start_time);

    error = create_consumers_producers(simulation);

    struct timespec finish_time;
    clock_gettime(/*clk_id*/CLOCK_MONOTONIC, &finish_time);

    double elapsed = (finish_time.tv_sec - start_time.tv_sec) +
      (finish_time.tv_nsec - start_time.tv_nsec) * 1e-9;
    printf("execution time: %.9lfs\n", elapsed);
  }
  return error;
}
// funcion encargada de validar los parametros recibidos, en caso
// de fallar con uno de los parametros o mas, o si no encuentra la cantidad
// necesaria de parametros, returna un error
int analyze_arguments(simulation_t* simulation, int argc, char* argv[]) {
  int error = EXIT_SUCCESS;
  if (argc == 8) {
    if (sscanf(argv[1], "%zu", &simulation->unit_count) != 1
      || simulation->unit_count == 0) {
        fprintf(stderr, "error: invalid unit count\n");
        error = ERR_UNIT_COUNT;
    } else if (sscanf(argv[2], "%zu", &simulation->producer_count) != 1
      || simulation->producer_count == 0) {
        fprintf(stderr, "error: invalid producer count\n");
        error = ERR_PRODUCER_COUNT;
    } else if (sscanf(argv[3], "%zu", &simulation->consumer_count) != 1
      || simulation->consumer_count == 0) {
        fprintf(stderr, "error: invalid consumer count\n");
        error = ERR_CONSUMER_COUNT;
    } else if (sscanf(argv[4], "%u", &simulation->producer_min_delay) != 1) {
        fprintf(stderr, "error: invalid min producer delay\n");
        error = ERR_MIN_PROD_DELAY;
    } else if (sscanf(argv[5], "%u", &simulation->producer_max_delay) != 1) {
        fprintf(stderr, "error: invalid max producer delay\n");
        error = ERR_MAX_PROD_DELAY;
    } else if (sscanf(argv[6], "%u", &simulation->consumer_min_delay) != 1) {
        fprintf(stderr, "error: invalid min consumer delay\n");
        error = ERR_MIN_CONS_DELAY;
    } else if (sscanf(argv[7], "%u", &simulation->consumer_max_delay) != 1) {
        fprintf(stderr, "error: invalid max consumer delay\n");
        error = ERR_MAX_CONS_DELAY;
    }
  } else {
    fprintf(stderr, "usage: producer_consumer buffer_capacity rounds"
      " producer_min_delay producer_max_delay"
      " consumer_min_delay consumer_max_delay\n");
      error = ERR_NO_ARGS;
  }
  return error;
}
// funcion encargada de crear los hilos productores y consumidores
pthread_t* create_threads(size_t count, void*(*subroutine)(void*), void* data) {
  pthread_t* threads = (pthread_t*) calloc(count, sizeof(pthread_t));
  if (threads) {
    for (size_t index = 0; index < count; ++index) {
      if (pthread_create(&threads[index], /*attr*/ NULL, subroutine, data)
         == EXIT_SUCCESS) {
      } else {
        fprintf(stderr, "error: could not create thread %zu\n", index);
        join_threads(index, threads);
        return NULL;
      }
    }
  }
  return threads;
}
// funcion encargada de unir todos los hilos productores y consumidores creados
int join_threads(size_t count, pthread_t* threads) {
  int error = EXIT_SUCCESS;
  for (size_t index = 0; index < count; ++index) {
    // todo: sum could not be right
    error += pthread_join(threads[index], /*value_ptr*/ NULL);
  }
  free(threads);
  return error;
}
// funcion encargada de enviar hilos a realizar los trabajos de las funciones
// producers() y consumers(), en caso de no poder realizar dicha accion
// retorna un errir
int create_consumers_producers(simulation_t* simulation) {
  assert(simulation);
  int error = EXIT_SUCCESS;
  // creamos el productor para que realice su trabajo
  pthread_t* producers = create_threads(simulation->producer_count, produce
    , simulation);
  // creamos el consumidor para que realice su trabajo
  pthread_t* consumers = create_threads(simulation->consumer_count, consume
    , simulation);

  if (producers && consumers) {
    join_threads(simulation->producer_count, producers);
    join_threads(simulation->consumer_count, consumers);
  } else {
    fprintf(stderr, "error: could not create threads\n");
    error = ERR_CREATE_THREAD;
  }

  return error;
}
