// Copyright 2024 ECCI-UCR CC-BY-4
#include <iostream>

int main(int argc, char* argv[]) {
  int process_number = -1;
  int process_count = -1;
  char process_hostname[1024] = { '\0' };

  std::cout << "Hello from main thread of process " << process_number
      << " of " << process_count << " on " << process_hostname << std::endl;
}
