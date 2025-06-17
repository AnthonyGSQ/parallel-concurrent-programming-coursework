// Copyright 2024-2025 ECCI-UCR CC-BY-4
#include <unistd.h>
#include <iostream>

#include "Mpi.hpp"

void play_ping_pong(Mpi& mpi, const long delay);

int main(int argc, char* argv[]) {
  try {
    Mpi mpi(argc, argv);
    if (mpi.size() == 2) {
    const long delay = argc >= 2 ? std::stol(argv[1]) : 500;  // milliseconds
    int myProbability;
    myProbability = argc >= 3 ?
      std::stoi(argv[mpi.rank() + 3]) : 50; // percent
    play_ping_pong(mpi, delay);
    } else if (mpi.rank() == 0) {
      std:: cerr << "error: only two processes are allowed" << std::endl;
      std::cerr << "usage: mpiexcec -np 2 bin/mpi_ping_pong" << std::endl;
    }
  } catch (const Mpi::Error& error) {
    std::cerr << "error: " << error.what() << std::endl;
  }
  return 0;
}

void play_ping_pong(Mpi& mpi, const long delay) {
  long ball;
  while (true) {
    if () {
      std::cout << mpi.rank() << ball << std::endl;
    usleep(1000 * delay);
    }
  }
}
