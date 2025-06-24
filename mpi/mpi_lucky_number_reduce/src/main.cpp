// Copyright 2024 ECCI-UCR CC-BY-4

#include <cstdlib>
#include <iostream>

#include "Mpi.hpp"
#include "UniformRandom.hpp"

#define out1 std::cout << "Process " << mpi.rank() << ": "

void generate_lucky_statistics(Mpi& mpi);

int main(int argc, char* argv[]) {
  try {
    Mpi mpi(argc, argv);
    generate_lucky_statistics(mpi);
  } catch (const Mpi::Error& error) {
    std::cerr << "error: " << error.what() << std::endl;
  }
  return 0;
}

void generate_lucky_statistics(Mpi& mpi) {
  // TODO(you): Generate my lucky number
  const int my_lucky_number = -1;

  out1 << "my lucky number is " << my_lucky_number << std::endl;

  int all_min = -1;
  int all_max = -1;
  int all_sum = -1;

  // TODO(you): Update distributed statistics from processes' lucky numbers

  const double all_average = static_cast<double>(all_sum) / mpi.size();
  out1 << "all minimum = " << all_min << std::endl;
  out1 << "all average = " << all_average << std::endl;
  out1 << "all maximum = " << all_max << std::endl;
}
