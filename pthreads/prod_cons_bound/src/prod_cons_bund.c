// Copyright 2021 Jeisson Hidalgo-Cespedes <jeisson.hidalgo@ucr.ac.cr> CC-BY-4
// Simulates a producer and a consumer that share a bounded buffer

// @see `man feature_test_macros`
#define _DEFAULT_SOURCE

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>
#include <unistd.h>

// enumeramos los errores que pueden presentarse
enum {
  ERR_NOMEM_SHARED = EXIT_FAILURE + 1,
  ERR_NOMEM_BUFFER,
  ERR_NO_ARGS,
  ERR_BUFFER_CAPACITY,
  ERR_ROUND_COUNT,
  ERR_MIN_PROD_DELAY,
  ERR_MAX_PROD_DELAY,
  ERR_MIN_CONS_DELAY,
  ERR_MAX_CONS_DELAY,
  ERR_CREATE_THREAD,
};
// struct de datos compartidos entre los hilos, dicho struct
// contiene la capacidad del buffer, el buffer en si, los rounds
// los tiempos minimos-maximos de produccion y el consumo de productos
// tambien guarda los semaforos a utilizar
typedef struct {
  size_t thread_count;
  size_t buffer_capacity;
  double* buffer;
  size_t rounds;
  useconds_t producer_min_delay;
  useconds_t producer_max_delay;
  useconds_t consumer_min_delay;
  useconds_t consumer_max_delay;

  sem_t can_produce;
  sem_t can_consume;
} shared_data_t;

// estructura privada con el numero del hilo actual y un puntero
// al struct compartido
typedef struct {
  size_t thread_number;
  shared_data_t* shared_data;
} private_data_t;
// funcion para leer y validar los argumentos minimos indispensables para
// el funcionamiento del programa
int analyze_arguments(int argc, char* argv[], shared_data_t* shared_data);
// funcion encargada de crear hilos para la produccion y el consumo de productos
int create_threads(shared_data_t* shared_data);
// funcion encargada de la simulacion de produccion de productos
void* produce(void* data);
// funcion encargada de la simulacion del consumo de productos
void* consume(void* data);
// funcion encargada de enviar el tiempo que se necesitara para producir
// y consumir x producto
useconds_t random_between(useconds_t min, useconds_t max);

int main(int argc, char* argv[]) {
  int error = EXIT_SUCCESS;
  // reservamos memoria para el struct compartido con calloc
  // para evitar que el struct tenga variables no inicializados.
  shared_data_t* shared_data = (shared_data_t*)
    calloc(1, sizeof(shared_data_t));

  if (shared_data) {
    // primeramente llamamos a analyze_arguments para verificar
    // los argumentos recibidos del usuario
    error = analyze_arguments(argc, argv, shared_data);
    if (error == EXIT_SUCCESS) {
      // inicializamos el buffer en ceros con calloc
      shared_data->buffer = (double*)
        calloc(shared_data->buffer_capacity, sizeof(double));
      if (shared_data->buffer) {
        // inicializacion de los semaforos
        sem_init(&shared_data->can_produce, /*pshared*/ 0,
          shared_data->buffer_capacity);
        sem_init(&shared_data->can_consume, /*pshared*/ 0, /*value*/ 0);

        unsigned int seed = 0u;
        // generamos una semilla distinta desde el hardware
        getrandom(&seed, sizeof(seed), GRND_NONBLOCK);
        srandom(seed);

        struct timespec start_time;
        clock_gettime(/*clk_id*/CLOCK_MONOTONIC, &start_time);
        // iniciamos el proceso de creacion de hilos y produccion y
        // consumo de productos
        error = create_threads(shared_data);
        // vemos cuanto tiempo tomo realizar el trabajo
        struct timespec finish_time;
        clock_gettime(/*clk_id*/CLOCK_MONOTONIC, &finish_time);

        double elapsed = (finish_time.tv_sec - start_time.tv_sec) +
          (finish_time.tv_nsec - start_time.tv_nsec) * 1e-9;
        printf("execution time: %.9lfs\n", elapsed);
        // destruimos los semaforos
        sem_destroy(&shared_data->can_consume);
        sem_destroy(&shared_data->can_produce);
        free(shared_data->buffer);
      } else {
        fprintf(stderr, "error: could not create buffer\n");
        error = ERR_NOMEM_BUFFER;
      }
    }
    // liberamos la memoria reservada para shared_data
    free(shared_data);
  } else {
    fprintf(stderr, "Error: could not allocate shared data\n");
    error = ERR_NOMEM_SHARED;
  }

  return error;
}
// esta funcion lee cada argumento y si este no es valido o no existe
// imprime con stderr un mensaje de error
int analyze_arguments(int argc, char* argv[], shared_data_t* shared_data) {
  int error = EXIT_SUCCESS;
  // todos estos ifs son manejo de errores
  if (argc == 7) {
    if (sscanf(argv[1], "%zu", &shared_data->buffer_capacity) != 1
      || shared_data->buffer_capacity == 0) {
        fprintf(stderr, "error: invalid buffer capacity\n");
        error = ERR_BUFFER_CAPACITY;
    } else if (sscanf(argv[2], "%zu", &shared_data->rounds) != 1
      || shared_data->rounds == 0) {
        fprintf(stderr, "error: invalid round count\n");
        error = ERR_ROUND_COUNT;
    } else if (sscanf(argv[3], "%u", &shared_data->producer_min_delay) != 1) {
        fprintf(stderr, "error: invalid min producer delay\n");
        error = ERR_MIN_PROD_DELAY;
    } else if (sscanf(argv[4], "%u", &shared_data->producer_max_delay) != 1) {
        fprintf(stderr, "error: invalid max producer delay\n");
        error = ERR_MAX_PROD_DELAY;
    } else if (sscanf(argv[5], "%u", &shared_data->consumer_min_delay) != 1) {
        fprintf(stderr, "error: invalid min consumer delay\n");
        error = ERR_MIN_CONS_DELAY;
    } else if (sscanf(argv[6], "%u", &shared_data->consumer_max_delay) != 1) {
        fprintf(stderr, "error: invalid max consumer delay\n");
        error = ERR_MAX_CONS_DELAY;
    }
  } else {
    fprintf(stderr, "usage: prod_cons_bound buffer_capacity rounds"
      " producer_min_delay producer_max_delay"
      " consumer_min_delay consumer_max_delay\n");
      error = ERR_NO_ARGS;
  }
  return error;
}

int create_threads(shared_data_t* shared_data) {
  assert(shared_data);
  int error = EXIT_SUCCESS;

  pthread_t producer, consumer;
  // enviamos a trabajar a los hilos y en caso de que estos fallen
  // manejamos el error con los if
  error = pthread_create(&producer, /*attr*/ NULL, produce, shared_data);
  if (error == EXIT_SUCCESS) {
    error = pthread_create(&consumer, /*attr*/ NULL, consume, shared_data);
    if (error != EXIT_SUCCESS) {
      fprintf(stderr, "error: could not create consumer\n");
      error = ERR_CREATE_THREAD;
    }
  } else {
    fprintf(stderr, "error: could not create producer\n");
    error = ERR_CREATE_THREAD;
  }

  if (error == EXIT_SUCCESS) {
    // si todo resulto bien, liberamos los hilos
    pthread_join(producer, /*value_ptr*/ NULL);
    pthread_join(consumer, /*value_ptr*/ NULL);
  }

  return error;
}

void* produce(void* data) {
  // const private_data_t* private_data = (private_data_t*)data;
  shared_data_t* shared_data = (shared_data_t*)data;
  size_t count = 0;
  for (size_t round = 0; round < shared_data->rounds; ++round) {
    for (size_t index = 0; index < shared_data->buffer_capacity; ++index) {
      // revisamos si existe espacio en el buffer para poder producir
      sem_wait(&shared_data->can_produce);

      usleep(1000 * random_between(shared_data->producer_min_delay
        , shared_data->producer_max_delay));
      shared_data->buffer[index] = ++count;
      printf("Produced %lg\n", shared_data->buffer[index]);

      // signal(can_consume)
      sem_post(&shared_data->can_consume);
    }
  }

  return NULL;
}

void* consume(void* data) {
  // const private_data_t* private_data = (private_data_t*)data;
  shared_data_t* shared_data = (shared_data_t*)data;
  for (size_t round = 0; round < shared_data->rounds; ++round) {
    for (size_t index = 0; index < shared_data->buffer_capacity; ++index) {
      // esperamos a que existan elementos para consumir
      sem_wait(&shared_data->can_consume);

      double value = shared_data->buffer[index];
      usleep(1000 * random_between(shared_data->consumer_min_delay
        , shared_data->consumer_max_delay));
      printf("\tConsumed %lg\n", value);

      // avisamos al producer que se ha consumido un producto y en consecuencia
      // tiene un espacio mas para producir
      sem_post(&shared_data->can_produce);
    }
  }

  return NULL;
}

useconds_t random_between(useconds_t min, useconds_t max) {
  return min + (max > min ? (random() % (max - min)) : 0);
}