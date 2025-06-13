// Copyright 2024 ECCI-UCR CC-BY-4
#pragma once

#include <mpi.h>
#include <string>

#include <stdexcept>

class Mpi {
 private:
  int processNumber = -1;
  int processCount = -1;
  std::string hostname;

 public:
  class Error : public std::runtime_error {
   public:
    explicit Error(const std::string& message)
      : runtime_error(message) {
    }
    Error(const std::string& message, const Mpi& mpi)
      : runtime_error(mpi.getHostname() + ':' + std::to_string(mpi.rank())
      + ": " + message) {
    }
    Error(const std::string& message, const Mpi& mpi, const int threadNumber)
      : runtime_error(mpi.getHostname() + ':' + std::to_string(mpi.rank())
      + '.' + std::to_string(threadNumber) + ": " + message) {
    }
  };
  
 public:
  Mpi(int& argc, char**& argv) {
    if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
      throw Error("could not init MPI");
    }
    if (MPI_Comm_rank(MPI_COMM_WORLD, &this->processNumber) != MPI_SUCCESS) {
      throw Error("could not get MPI rank");
    }
    if (MPI_Comm_size(MPI_COMM_WORLD, &this->processCount) != MPI_SUCCESS) {
      throw Error("could not get MPI size");
    }
    char processHostname[MPI_MAX_PROCESSOR_NAME] = { '\0' };
    int hostnameLen = -1;
    if (MPI_Get_processor_name(processHostname, &hostnameLen) != MPI_SUCCESS) {
      throw Error("could not get MPI processor name");
    }
    this->hostname = processHostname;
  }
  Mpi(const Mpi&) = delete;
  Mpi(Mpi&&) = delete;
  ~Mpi() {
    MPI_Finalize();
  }
  Mpi& operator=(const Mpi&) = delete;
  Mpi& operator=(Mpi&&) = delete;

 public:  // Accessors
  inline int getProcessNumber() const {
    return this->processNumber;
  }
  inline int getProcessCount() const {
    return this->processCount;
  }
  inline const std::string& getHostname() const {
    return this->hostname;
  }
  inline int rank() const {
    return this->getProcessNumber();
  }
  inline int size() const {
    return this->getProcessCount();
  }

  public:
  static inline MPI_Datatype map(bool) { return MPI_C_BOOL; }
  static inline MPI_Datatype map(char) { return MPI_CHAR; }
  static inline MPI_Datatype map(unsigned char) { return MPI_UNSIGNED_CHAR; }
  static inline MPI_Datatype map(short) { return MPI_SHORT; }
  static inline MPI_Datatype map(unsigned short) { return MPI_UNSIGNED_SHORT; }
  static inline MPI_Datatype map(int) { return MPI_INT; }
  static inline MPI_Datatype map(unsigned) { return MPI_UNSIGNED; }
  static inline MPI_Datatype map(long) { return MPI_LONG; }
  static inline MPI_Datatype map(unsigned long) { return MPI_UNSIGNED_LONG; }
  static inline MPI_Datatype map(long long) { return MPI_LONG_LONG; }
  static inline MPI_Datatype map(unsigned long long) {
    return MPI_UNSIGNED_LONG_LONG; }
  static inline MPI_Datatype map(float) { return MPI_FLOAT; }
  static inline MPI_Datatype map(double) { return MPI_DOUBLE; }
  static inline MPI_Datatype map(long double) { return MPI_LONG_DOUBLE; }

  public: // send
  template <typename Type>
  void send(const Type& value, const int toProcess, const int tag = 0) {
    if (MPI_Send(&value, /*count*/ 1, Mpi::map(value), toProcess, tag,
      MPI_COMM_WORLD) != MPI_SUCCESS) {
      throw Mpi::Error("could not send value", *this);
    }
  }
  public: // receive
  template <typename Type>
  void receive(Type& value, const int fromProcess = MPI_ANY_SOURCE,
    const int tag = MPI_ANY_TAG) {
    if (MPI_Recv(&value, /*count*/ 1, Mpi::map(value), fromProcess,
      tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE) != MPI_SUCCESS)  {
      throw Mpi::Error("could not receive value", *this);
    }
  }
};
