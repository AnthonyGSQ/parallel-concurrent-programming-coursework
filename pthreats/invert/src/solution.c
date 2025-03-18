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
  // Create a dynamic array to store the numbers because we do not
  // know the number of elements beforehand
  long* numbers = malloc(1000*sizeof(long));
  size_t count = 0, capacity = 1000;
  // Read numbers until end-of-file
  while (scanf("%ld", &numbers[count]) == 1) {
    // If we do not have capacity to store more numbers
    if (++count == capacity) {
      // Increase capacity of array
      capacity *= 10;
      numbers = realloc(numbers, capacity* sizeof(long));
    }
  }
  // Print all numbers
  for (long index=count-1; index >=0; --index) {
    printf("%ld", numbers[index]);
    printf("\n");
  }
  free(numbers);
  return EXIT_SUCCESS;
}
