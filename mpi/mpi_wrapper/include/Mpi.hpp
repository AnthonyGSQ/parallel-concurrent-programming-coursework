// Copyright 2024 ECCI-UCR CC-BY-4
#pragma once
#include <mpi.h>
#include <string>

#include <stdexcept>

class Mpi {
    private:
      int processNumber = -1;
      int processCount = -1;
      std::string hostName;

    public:
      Mpi(int& argc, char**& argv) {
        if (MPI_Init(&argc ,&argv ) != MPI_SUCCESS) {
          std::cerr<<"error: c\n";
          throw std::runtime_error("Could not init MPI");
        }
        int process_number = -1;
        if (MPI_Comm_rank(MPI_COMM_WORLD, &process_number) != MPI_SUCCESS) {
          throw std::runtime_error("could not get MPI rank");
        }
        if (MPI_Comm_size(MPI_COMM_WORLD, &processCount) != MPI_SUCCESS) {
          throw::std::runtime_error("Could not get size");
        }
        int hostname_len = -1;
        char processHostName[MPI_MAX_PROCESSOR_NAME] = { '\0' };
        if (MPI_Get_processor_name(processHostName, &hostname_len) != MPI_SUCCESS) {
          throw std::runtime_error("could not get MPI processor name (hostName)");
        }
        this->processCount = -1;
        std::cout << "Hello from main thread of process " << process_number
        << " of " << processCount << " on " << processHostName << std::endl;
        this->hostName = processHostName;
        }
    Mpi (const Mpi&) = delete;
    Mpi (Mpi&&) = delete;
    ~Mpi() {
      MPI_Finalize ();
    }
    Mpi& operator=(const Mpi&) = delete;
    Mpi& operator=(Mpi&&) = delete;
    public:
      inline int getProcessNumber() const {
        return this->processNumber;
      }
      inline int getProcessCount() const {
        return this->processCount;
      }
      inline const std::string& getHostname() const {
        return this->hostName;
      }
      inline int rank() const {
        return this->getProcessNumber();
      }
      inline int size() const {
        return this->getProcessCount();
      }
};  
