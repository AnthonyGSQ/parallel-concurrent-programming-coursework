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

typedef struct shared_data {
  sem_t* can_greet;
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
  // asumimos el numero de hilos = al numero de nucleos de la computadora
  uint64_t thread_count = sysconf(_SC_NPROCESSORS_ONLN);
  if (argc == 2) {
    if (sscanf(argv[1], "%" SCNu64, &thread_count) == 1) {
    } else {
      fprintf(stderr, "Error: invalid thread count\n");
      return 11;
    }
  }
  // reservamos memoria para el shared_data y inicializamos en 0 con calloc
  shared_data_t* shared_data = (shared_data_t*)calloc(1, sizeof(shared_data_t));
  if (shared_data) {
	// creamos el arreglo de semaforos para la funcion greet
    shared_data->can_greet = (sem_t*) calloc(thread_count, sizeof(sem_t));
    shared_data->thread_count = thread_count;
	// creamos un semaforo para cada hilo y lo inicializamos con valor 0 excepto
	// el primer hilo creado, ya que la negacion de 0 es 1 y la negacion de
	// cualquier numero mayor a 0 dara 0.
    for (uint64_t thread_number = 0; thread_number < shared_data->thread_count
        ; ++thread_number) {
      // can_greet[thread_number] := create_semaphore(not thread_number)
      error = sem_init(&shared_data->can_greet[thread_number], /*pshared*/ 0
        , /*value*/ !thread_number);
    }

    if (shared_data->can_greet) {
      struct timespec start_time, finish_time;
	  // este bloque de codigo es para indicar cuanto tiempo le tomo
	  // al programa ejecutar todo
      clock_gettime(CLOCK_MONOTONIC, &start_time);
      error = create_threads(shared_data);
      clock_gettime(CLOCK_MONOTONIC, &finish_time);
      double elapsed_time = finish_time.tv_sec - start_time.tv_sec +
        (finish_time.tv_nsec - start_time.tv_nsec) * 1e-9;
      printf("Execution time: %.9lfs\n", elapsed_time);
      free(shared_data->can_greet);
    } else {
      fprintf(stderr, "Error: could not allocate semaphores\n");
      error = 13;
    }
    free(shared_data);
  } else {
    fprintf(stderr, "Error: could not allocate shared data\n");
    error = 12;
  }
  return error;
}

int create_threads(shared_data_t* shared_data) {
	int error = EXIT_SUCCESS;
	// reservamos memoria para nuestro struct compartido
	pthread_t* threads = (pthread_t*)
	  malloc(shared_data->thread_count * sizeof(pthread_t));
	// reservamos memoria para nuestro struct privado
	private_data_t* private_data = (private_data_t*)
	  calloc(shared_data->thread_count, sizeof(private_data_t));
	// si se pudo reservar memoria para el hilo y su estructura privada
	// realizamos todo, sino retornamos error 23
	if (threads && private_data) {
		// bucle para la creacion de hilos y asignacion de datos 
		// privados para cada hilo
	  for (uint64_t thread_number = 0; thread_number < shared_data->thread_count
		  ; ++thread_number) {
		if (error == EXIT_SUCCESS) {
		  private_data[thread_number].thread_number = thread_number;
		  private_data[thread_number].shared_data = shared_data;
		  // create_thread(greet, thread_number)
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
	  }// for thread_number := 0 to thread_count do
  
	  // print "Hello from main thread"
	  printf("Hello from main thread\n");
	  // bucle para destruir todos los hilos y semaforos creados
	  for (uint64_t thread_number = 0; thread_number < shared_data->thread_count
		  ; ++thread_number) {
		pthread_join(threads[thread_number], /*value_ptr*/ NULL);
		sem_destroy(&shared_data->can_greet[thread_number]);
	  }
	  // terminamos de liberar la memoria
	  free(private_data);
	  free(threads);
	} else {
	  fprintf(stderr, "Error: could not allocate %" PRIu64 " threads\n"
		, shared_data->thread_count);
	  error = 23;
	}
  
	return error;
  }

void* greet(void* data) {
  assert(data);
  private_data_t* private_data = (private_data_t*) data;
  shared_data_t* shared_data = private_data->shared_data;

  // esperamos en el semaforo hasta que sea mi turno
  int error = sem_wait(&shared_data->can_greet[private_data->thread_number]);
  if (error) {
    fprintf(stderr, "error: could not wait for semaphore\n");
  }

  printf("Hello from secondary thread %" PRIu64 " of %" PRIu64 "\n"
    , private_data->thread_number, shared_data->thread_count);

  // indicamos cual sera el siguiente hilo a poder pasar por el
  // semaforo
  const uint64_t next_thread = (private_data->thread_number + 1)
    % shared_data->thread_count;
	// aumentamos el valor del semaforo para indicar que puede pasar
	// un hilo mas
  error = sem_post(&shared_data->can_greet[next_thread]);
  if (error) {
    fprintf(stderr, "error: could not increment semaphore\n");
  }

  return NULL;
}
