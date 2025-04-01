// Copyright 2025 Anthony Sanchez
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <inttypes.h>

/**
 * @brief Start program execution.
 *
 * @return Status code to the operating system, 0 means success.
 */

typedef struct {
  uint64_t total_threads;
  uint64_t punches_needed;
  pthread_mutex_t can_punch;
  pthread_cond_t cond_punch;
} shared_data_t;

typedef struct {
  uint64_t thread_number;
  uint64_t punches_gived;
  shared_data_t *shared_data;
} private_data_t;

int create_threads(size_t threads_count, uint64_t punches_needed_);
void* punch(void* data);
int main(int argc, char* argv[]) {
  if (argc != 3) {
    printf("Error: Not the right amounth of parameters, make sure to only"
      "specify the number of threads and the amounth of punches needed to"
      " break the pinata\n");
      return EXIT_FAILURE;
  }
  uint64_t thread_count = sysconf(_SC_NPROCESSORS_ONLN);
  uint64_t punches_needed;
  if (argc == 2) {
    if (sscanf(argv[1], "%" SCNu64, &thread_count) == 1) {
    } else {
      printf("Error: invalid thread count\n");
      return EXIT_FAILURE;
    }
  }
  if (argc == 3) {
    if (sscanf(argv[2], "%" SCNu64, &punches_needed) == 1) {
    } else {
      printf("Error: invalid amounth of punches needed\n");
      return EXIT_FAILURE;
    }
  }
  int error = create_threads(thread_count, punches_needed);
  if (error == EXIT_FAILURE) {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int create_threads(uint64_t thread_count, uint64_t punches_needed_) {
  // si llegamos hasta aqui significa que todos los parametros
  // existen y fueron leidos correctamente
  pthread_t* threads = (pthread_t*)malloc(thread_count * sizeof(pthread_t));
  size_t error = 0;
  // reservamos memoria para los n structs privados para los n hilos
  // solicitados, inicializando todo en cero con calloc
  private_data_t* private_data = (private_data_t*)calloc(thread_count,
    sizeof(private_data_t));
  // creamos un solo struct shared_data pues sera el mismo para todos
  // los hilos
  shared_data_t* shared_data = (shared_data_t*)calloc(1, sizeof(shared_data_t));
  shared_data->punches_needed = punches_needed_;
  // creamos el mutex
  error = pthread_mutex_init(&shared_data->can_punch, NULL);
  // si no se pudo crear el mutex, retornarmos EXIT_FAILURE
  if (error == EXIT_FAILURE) {
    printf("Error: could not create the mutex\n");
    return EXIT_FAILURE;
  }
  for (uint64_t i = 0; i < thread_count; i++) {
    private_data[i].thread_number = i + 1;
    private_data[i].shared_data = shared_data;
    error = pthread_create(&threads[i], NULL, punch, &private_data[i]);
    if (error == EXIT_FAILURE) {
      printf("Error: Could not create secondary thread\n");
      return EXIT_FAILURE;
    }
  }

  // ciclo para liberar todos los hilos
  for (size_t i = 0; i < thread_count; i++) {
    pthread_join(threads[i], NULL);
  }
  pthread_mutex_destroy(&shared_data->can_punch);
  free(private_data);
  free(shared_data);
  free(threads);
  return EXIT_SUCCESS;
}
void* punch(void *data) {
  // asignamos los private data y shared a variables dentro de la
  // funcion
  private_data_t* private_data = (private_data_t*) data;
  shared_data_t* shared_data = private_data->shared_data;
  // en caso de punches_needed ser 0, significa que la pinata ya se
  // rompio y por ende no puede realizar el golpe.
  if (shared_data->punches_needed == 0) {
    return NULL;
  }
  while (shared_data->punches_needed > 0) {
    // bloqueamos la parte de bloquear para que solo un hilo pueda golpear
  // a la vez.
  pthread_mutex_lock(&shared_data->can_punch);
  if (shared_data->punches_needed == 0) {
    printf(", I broke the pinata!");
    pthread_mutex_unlock(&shared_data->can_punch);
  } else {
    printf("\n");
  }
  private_data->punches_gived++;
  // imprimimos el numero de hilo actual y cuantos golpes lleva
  printf("Thread %" PRIu64 "/%" PRIu64 ": %" PRIu64 " hits ",
    private_data->thread_number,
    shared_data->total_threads, private_data->punches_gived);
  shared_data->punches_needed--;
  // si el hilo actual consigue romper la pinata, imprimimos dicha situacion
  //pthread_cond_broadcast(&shared_data->cond_punch);
  // desbloqueamos pues el hilo que estaba golpeando la pinata, ya termino
  // de golpear.
  pthread_mutex_unlock(&shared_data->can_punch);
  }
  return NULL;
}
