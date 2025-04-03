// Copyright 2025 Anthony Sanchez
#define _POSIX_C_SOURCE 199309L
#include <assert.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

typedef struct shared_data {
  uint64_t next_thread;
  uint64_t thread_count;
} shared_data_t;

// estructura de los datos privados de cada hilo
typedef struct private_data {
  // se utilizan los uint64_t para que el size de las variables
  // no dependa de la arquitectura de la computadora
  uint64_t thread_number;
  shared_data_t* shared_data;
} private_data_t;

/**
 * @brief ...
 */
void* greet(void* data);
int create_threads(shared_data_t* thread_count);

// procedure main(argc, argv[])
int main(int argc, char* argv[]) {
  int error = EXIT_SUCCESS;
  uint64_t thread_count = sysconf(_SC_NPROCESSORS_ONLN);
  if (argc == 2) {
    if (sscanf(argv[1], "%" SCNu64, &thread_count) == 1) {
    } else {
      fprintf(stderr, "Error: invalid thread count\n");
      return 11;
    }
  }

  shared_data_t * shared_data = (shared_data_t*)calloc(1,
      sizeof(shared_data_t));
  if (shared_data) {
    shared_data->thread_count = thread_count;
    struct timespec start_time, finish_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    // ahora al crear hilos reciben los datos compartidos y no el
    // numero de hilos a crear
    error = create_threads(shared_data);
    clock_gettime(CLOCK_MONOTONIC, &finish_time);
    double elapsed_time = finish_time.tv_sec - start_time.tv_sec +
      (finish_time.tv_nsec - start_time.tv_nsec) * 1e-9;
      printf("Execution time %.9lfs\n", elapsed_time);
    free(shared_data);
  } else {
    fprintf(stderr, "Error: could not allocate shared data\n");
    return 12;
  }
  return error;
}  // end procedure


int create_threads(shared_data_t* shared_data) {
  int error = EXIT_SUCCESS;
  // for thread_number := 0 to thread_count do
  // Reservamos espacio en la memoria con el size de un hilo * el numero
  // de hilos que se van a crear
  pthread_t* threads = (pthread_t*) malloc(shared_data->thread_count
    * sizeof(pthread_t));
  // se usa calloc en private_data ya que a diferencia de malloc,
  // calloc inicializa los datos, limpiando la "basura" que habia
  // en la parte de la memoria reservada
  private_data_t* private_data = (private_data_t*)
    calloc(shared_data->thread_count, sizeof(private_data_t));
    // aseguramos que se hayan creado threads y private_data
    // si no es el caso, se informa que no fue posible crear
    // dichos datos
  if (threads && private_data) {
    // for encargado de crear todos los hilos solicitados
    for (uint64_t thread_number = 0; thread_number < shared_data->thread_count
        ; ++thread_number) {
      private_data[thread_number].thread_number = thread_number;
      private_data[thread_number].shared_data = shared_data;
      // create_thread(greet, thread_number)
      error = pthread_create(&threads[thread_number], /*attr*/ NULL, greet
        , /*arg*/ &private_data[thread_number]);
        // if para tratar el caso en donde el hilo no pudo ser creado
      if (error == EXIT_SUCCESS) {
      } else {
        fprintf(stderr, "Error: could not create secondary thread\n");
        error = 21;
        break;
      }
    }

    // print "Hello from main thread"
    printf("Hello from main thread\n");
    // bucle para unir todos los hilos creados
    for (uint64_t thread_number = 0; thread_number < shared_data->thread_count
        ; ++thread_number) {
      pthread_join(threads[thread_number], /*value_ptr*/ NULL);
    }
    // liberamos toda la memoria reservada
    free(private_data);
    free(threads);
  } else {
    fprintf(stderr, "Error: could not allocate %" PRIu64 " threads\n"
      , shared_data->thread_count);
    error = 22;
  }

  return error;
}

// procedure greet:
void* greet(void* data) {
  // assert(data);
  private_data_t* private_data = (private_data_t*) data;
  shared_data_t* shared_data = private_data->shared_data;
  // print "Hello from secondary thread"
  // bucle de espera, si el hilo anterior no ha pasado de este bucle
  // y en consecuencia no ha aumentado el valor de next_thread,
  // el hilo actual continuara enciclado en el while de espera 
  // Este bucle es una espera activa TERMINANTEMENTE PROHIBIDA
  while (shared_data->next_thread < private_data->thread_number) {
    // busy-wait
  }
  // print "Hello from secondary thread"
  printf("Hello from secondary thread %" PRIu64 " of %" PRIu64 "\n"
    , (*private_data).thread_number, shared_data->thread_count);
    shared_data->next_thread++;
  return NULL;
}  // end procedure
