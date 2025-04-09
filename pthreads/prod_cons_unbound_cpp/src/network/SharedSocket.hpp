// Copyright 2021-2024 Jeisson Hidalgo-Cespedes. ECCI-UCR. CC BY 4.0
#pragma once

#include <sys/socket.h>
#include <unistd.h>
#include <sstream>
#include <string>

#include "common.hpp"

class NetworkAddress;

// TODO(jhc): Ensure error handling code is working, throw exceptions
// TODO(jhc): Create test cases for network operations

/**
 * @brief Internal representation of a socket that can be shared by several
 * Socket instances. Do not use this class, use Socket instead.
 */
struct SharedSocket {
  /// SharedSockets cannot be copied
  DISABLE_COPY(SharedSocket);

 public:
  /// IPv4 or IPv6 address of the peer on the other side of the connection
  struct sockaddr_storage peerAddress;
  /// Socket file descriptor given by OS to communicate with the peer
  int socketFileDescriptor = -1;
  /// Buffer to extract data before a read of received data
  std::ostringstream output;
  /// Buffer to store data in memory before sending to the peer
  /// TODO(any): istringstream is not suitable to manage large/binary data
  std::istringstream input;

 public:
  /// Constructor
  SharedSocket();
  /// Destructor. Closes the socket file descriptor
  ~SharedSocket();
  /// Closes the socket file descriptor
  void close();
  /// Get read-and-write access to the storage address record
  inline struct sockaddr_storage& getSockAddrStorage() {
    return this->peerAddress;
  }
  /// Get read-and-write access of the storage address as a sockaddr record
  inline struct sockaddr* getSockAddr() {
    return reinterpret_cast<struct sockaddr*>(&this->peerAddress);
  }
  /// Get read-only access of the storage address as a sockaddr record
  inline const struct sockaddr* getSockAddr() const {
    return reinterpret_cast<const struct sockaddr*>(&this->peerAddress);
  }
  /// Build a human-readable network address object of the peer
  NetworkAddress getNetworkAddress() const;
  /// Return true if this socket is connected and there were no input/output
  /// errors before
  inline bool isOk() const {
    return this->socketFileDescriptor >= 0 &&
        this->input.good() && this->output.good();
  }
};
