// Copyright 2025 Anthony Sanchez
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void* greet(void* data);
// procedure main:
int main(void) {
  // create_thread(greet)
  size_t number = 2;
  pthread_t thread;
  // se inicializa el thread, seguido van los atributos, luego
  // la tarea que quiero que haga

  // el retorno int es un codigo que indica si hubo error en la subrutina
  int error = pthread_create(&thread,  /* attr */  NULL, greet,
    /*args*/  (void*)&number);
  if (error == EXIT_SUCCESS) {
    printf("\nHello from main thread\n");
    // el hilo principal main "espera" a que el hilo segundario termine
    pthread_join(thread,  /*value_ptr*/  NULL);
  } else {
    // stderr imprime en un archivo de errores, es buena practica
    // de programacion
    fprintf(stderr, "Error: could not create secondary thread\n");
  }
  // print "Hello from main thread"
  return error;
}
// end procedure

// un nuevo hilo de ejecucion que trabajara independientemente del hilo main
// procedure greet:
void* greet(void* data) {
  // print "Hello from secondary thread"
  size_t number = *(size_t*)data;
  if (number == 0){
    printf("Number %ld detected, bye bye\n", number);
    return NULL;
  }
  printf("Number %ld is bigger than 0, hello! :)", number);
  return NULL;
}  // end procedure
