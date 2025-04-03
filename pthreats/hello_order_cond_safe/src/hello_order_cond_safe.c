// Copyright 2025 Anthony Sanchez
#define _POSIX_C_SOURCE 199309L
#include <assert.h>
#include <inttypes.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// size para el arreglo de saludos
#define MAX_GREET_LEN 256

// thread_shared_data_t
typedef struct shared_data {
  char** greets;
  uint64_t thread_count;
} shared_data_t;

// thread_private_data_t
typedef struct private_data {
  uint64_t thread_number;  // rank
  shared_data_t* shared_data;
} private_data_t;

/**
 * @brief ...
 */
void* greet(void* data);
int create_threads(shared_data_t* shared_data);

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
  // utilizamos calloc para evitar posibles valores basura
  shared_data_t* shared_data = (shared_data_t*)calloc(1, sizeof(shared_data_t));
  if (shared_data) {
    shared_data->greets = (char**) calloc(thread_count, sizeof(char*));
    shared_data->thread_count = thread_count;

    if (shared_data->greets) {
      // logica para el calculo e impresion del tiempo que le toma al
      // programa realizar todo
      struct timespec start_time, finish_time;
      clock_gettime(CLOCK_MONOTONIC, &start_time);

      error = create_threads(shared_data);

      clock_gettime(CLOCK_MONOTONIC, &finish_time);
      double elapsed_time = finish_time.tv_sec - start_time.tv_sec +
        (finish_time.tv_nsec - start_time.tv_nsec) * 1e-9;

      printf("Execution time: %.9lfs\n", elapsed_time);

      free(shared_data->greets);
    } else {
      fprintf(stderr, "Error: could not allocate greets\n");
      error = 13;
    }
    free(shared_data);
  } else {
    fprintf(stderr, "Error: could not allocate shared data\n");
    error = 12;
  }
  return error;
}  // end procedure


int create_threads(shared_data_t* shared_data) {
  int error = EXIT_SUCCESS;
  // creamos n hilos reservando memoria con malloc
  pthread_t* threads = (pthread_t*)
    malloc(shared_data->thread_count * sizeof(pthread_t));
    // al ser un struct utilizamos calloc para no trabajar con
    // una variable struct no inicializada
  private_data_t* private_data = (private_data_t*)
    calloc(shared_data->thread_count, sizeof(private_data_t));
  if (threads && private_data) {
    for (uint64_t thread_number = 0; thread_number < shared_data->thread_count
        ; ++thread_number) {
          // reservamos memoria para los n saludos de 256 caracteres
      shared_data->greets[thread_number] = (char*)
        malloc(MAX_GREET_LEN * sizeof(char));
      if (shared_data->greets[thread_number]) {
        // inicializamso el primer caracter en 0
        shared_data->greets[thread_number][0] = '\0';
        // le indicamos a cada hilo sus datos privados.
        private_data[thread_number].thread_number = thread_number;
        private_data[thread_number].shared_data = shared_data;
        error = pthread_create(&threads[thread_number], /*attr*/ NULL, greet
          , /*arg*/ &private_data[thread_number]);
        if (error == EXIT_SUCCESS) {
        } else {
          fprintf(stderr, "Error: could not create secondary thread\n");
          error = 21;
          break;
        }
      } else {
        fprintf(stderr, "Error: could not init semaphore\n");
        error = 22;
        break;
      }
    }

    printf("Hello from main thread\n");

    for (uint64_t thread_number = 0; thread_number < shared_data->thread_count
        ; ++thread_number) {
      pthread_join(threads[thread_number], /*value_ptr*/ NULL);
    }
    // for para imprimir los saludos
    for (uint64_t thread_number = 0; thread_number < shared_data->thread_count
        ; ++thread_number) {
      printf("%s\n", shared_data->greets[thread_number]);
      // free despues del ultimo uso de greets
      free(shared_data->greets[thread_number]);
    }  // end for

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
  assert(data);
  private_data_t* private_data = (private_data_t*) data;
  shared_data_t* shared_data = private_data->shared_data;
  // sprintf es para realizar la impresion al buffer greets
  sprintf(shared_data->greets[private_data->thread_number]
    , "Hello from secondary thread %" PRIu64 " of %" PRIu64
    , private_data->thread_number, shared_data->thread_count);

  return NULL;
}  // end procedure