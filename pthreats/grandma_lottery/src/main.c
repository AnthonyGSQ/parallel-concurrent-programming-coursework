// Copyright <Year> <You> CC-BY-4

#define _POSIX_C_SOURCE 199506L
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void* buy_lottery(void* data);

// procedure main:
int main(void) {
  // create_thread(greet)
  pthread_t grand_child1, grand_child2;
  const int error1 = pthread_create(&grand_child1, /*attr*/ NULL, buy_lottery,
      /*arg*/ NULL);
  const int error2 = pthread_create(&grand_child2, /*attr*/ NULL, buy_lottery,
      /*arg*/ NULL);
  if (error1 != EXIT_SUCCESS || error2 != EXIT_SUCCESS) {
    fprintf(stderr, "Error: could not create both threads");
    return EXIT_FAILURE;
  }
  void* number1 = NULL;
  void* number2 = NULL;
  pthread_join(grand_child1, &number1);
  pthread_join(grand_child2, &number2);
  printf("My numbers are %zu and %zu\n", (size_t) number1, (size_t) number2);

  return EXIT_SUCCESS;
}  // end procedure

// procedure to buy lottery
void* buy_lottery(void* data) {
  (void)data;
  sleep(1);
  unsigned seed = time(NULL) + clock() + pthread_self();
  size_t number = rand_r(&seed) % 100;
  return (void*) number;
}  // end procedure
