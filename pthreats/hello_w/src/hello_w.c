// Copyright 2025 Anthony Sanchez
#include <assert.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * @brief ...
 */
void* greet(void* data);
// todos los uint64_t se utilizan porque son tipos de variables definidos
// es decir, no cambian acorde a la infraestructura de donde esta siendo
// ejecutado
int create_threads(uint64_t thread_count);

// procedure main(argc, argv[])
int main(int argc, char* argv[]) {
#if 0
  for (int index = 0; index < argc; ++index) {
    printf("argv[%d] = '%s'\n", index, argv[index]);
  }
  return 0;
#endif

  int error = EXIT_SUCCESS;
  // create thread_count as result of converting argv[1] to integer
  // thread_count := integer(argv[1])
  // en caso de que el usuario no especifique el numero de hilos que
  // quiere, se asume como thread_count el total de nucleos disponibles
  uint64_t thread_count = sysconf(_SC_NPROCESSORS_ONLN);
  // al ejecutar el compilable, si no se envia un argumento (argv[1])
  // se envia el mensaje de error
  if (argc == 2) {
    if (sscanf(argv[1], "%" SCNu64, &thread_count) == 1) {
    } else {
      fprintf(stderr, "Error: invalid thread count\n");
      return 11;
    }
  }

  error = create_threads(thread_count);
  return error;
}  // end procedure

// separamos el trabajo de crear hilos del main en una funcion
int create_threads(uint64_t thread_count) {
  int error = EXIT_SUCCESS;
  // el *sizeof es para reservar memoria 8 bytes * cantidad de hilos a crear
  pthread_t* threads = (pthread_t*) malloc(thread_count * sizeof(pthread_t));
  // if para evitar "crear" 0 hilos lo cual seria invalido
  if (threads) {
    for (uint64_t thread_number = 0; thread_number < thread_count
        ; ++thread_number) {
      // create_thread(greet, thread_number)
      error = pthread_create(&threads[thread_number], /*attr*/ NULL, greet
        , /*arg*/ (void*) thread_number);
      if (error == EXIT_SUCCESS) {
      } else {
        fprintf(stderr, "Err): could not create secondary thread\n");
        error = 21;
        break;
      }
    }

    // print "Hello from main thread"
    printf("Hello from main thread\n");

    // bucle para unir todos los) hilos creados, se realiza luego de
    // que todos los hilos han hecho el trabajo delegado para evitar
    // programacion serial, debido a que si creamos un hilo y hacemos join
    // el programa debera esperar al hilo para luego crear otro. Quitando toda
    // la programacion paralela del programa
    for (uint64_t thread_number = 0; thread_number < thread_count
        ; ++thread_number) {
      pthread_join(threads[thread_number], /*value_ptr*/ NULL);
    }
    // liberamos la memoria reservada
    free(threads);
  } else {
    fprintf(stderr, "Error: could not allocate %" PRIu64 " threads\n"
      , thread_count);
    error = 22;
  }

  return error;
}

// procedure greet:
void* greet(void* data) {
  // assert(data);
  const uint64_t rank = (uint64_t) data;
  printf("Hello from secondary thread %" PRIu64 "\n", rank);
  return NULL;
}  // end procedure