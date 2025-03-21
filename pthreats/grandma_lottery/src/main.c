// Copyright <Year> <You> CC-BY-4
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void* greet(void* data);

// procedure main:
int main(void) {
  // create_thread(greet)
  pthread_t thread;
  int error = pthread_create(&thread, /*attr*/ NULL, greet, /*arg*/ NULL);
  if (error == EXIT_SUCCESS) {
    // print "Hello from main thread"
    printf("Hello from main thread\n");
    pthread_join(thread, /*value_ptr*/ NULL);
  } else {
    fprintf(stderr, "Error: could not create secondary thread\n");
  }
  return error;
}  // end procedure

// procedure greet:
void* greet(void* data) {
  (void)data;
  // print "Hello from secondary thread"
  printf("Hello from secondary thread\n");
  return NULL;
}  // end procedure
