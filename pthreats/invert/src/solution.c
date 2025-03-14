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
  long numbers[1000];
  size_t count = 0;
  while(scanf("%ld", &numbers[count])==1){
    count++;
  }

  for(long index=count-1;index>=0;--index){
    printf("%ld", numbers[index]);
  }
  return EXIT_SUCCESS;
}
