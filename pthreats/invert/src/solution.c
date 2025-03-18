// Copyright <year> <You>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Start program execution.
 *
 * @return Status code to the operating system, 0 means success.
 */
int main(void) {
  long* numbers = malloc(1000*sizeof(long));
  size_t count = 0, capacity = 1000;
  while (scanf("%ld", &numbers[count]) == 1) {
    if (count == capacity) {
      capacity *= 10;
      numbers = realloc(numbers, capacity* sizeof(long));
    }
  }

  for (long index=count-1; index >=0; --index) {
    printf("%ld", numbers[index]);
    printf("\n");
  }
  free(numbers);
  return EXIT_SUCCESS;
}
