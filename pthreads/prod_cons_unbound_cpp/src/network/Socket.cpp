// Copyright 2021-2025 Jeisson Hidalgo-Cespedes. ECCI-UCR. CC BY 4.0

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>

#include <cassert>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include "Log.hpp"
#include "NetworkAddress.hpp"
#include "Socket.hpp"

Socket::Socket()
  : sharedSocket{new SharedSocket()} {
}

void Socket::close() {
  this->sharedSocket->close();
}

NetworkAddress Socket::getNetworkAddress() const {
  return this->sharedSocket->getNetworkAddress();
}

int Socket::getSocketFileDescriptor() const {
  return this->sharedSocket->socketFileDescriptor;
}

void Socket::setSocketFileDescriptor(int socketFileDescriptor) {
  this->sharedSocket->socketFileDescriptor = socketFileDescriptor;
}

struct sockaddr* Socket::getSockAddr() {
  return this->sharedSocket->getSockAddr();
}

void Socket::setNetworkAddress(const struct addrinfo* address) {
  std::memcpy(&this->sharedSocket->peerAddress, address->ai_addr
    , address->ai_addrlen);
}

bool Socket::operator==(const Socket& other) const {
  return this->sharedSocket->socketFileDescriptor
    == other.sharedSocket->socketFileDescriptor;
}

bool Socket::operator<(const Socket& other) const {
  return this->sharedSocket->socketFileDescriptor
    < other.sharedSocket->socketFileDescriptor;
}

bool Socket::readLine(std::string& line, char separator) {
  while (!std::getline(this->sharedSocket->input, line, separator)) {
    if (!this->receive()) {
      return false;  // error
    }
    // receive() clears stream error flags, and retry
  }
  return true;  // success read
}

bool Socket::receive() {
  const size_t capacity = 65535;
  std::vector<char> buffer(capacity);
  ssize_t res = this->readAvailable(buffer.data(), buffer.size());
  if (res > 0) {
    this->sharedSocket->input.str(buffer.data());
    this->sharedSocket->input.clear();
    return true;
  }
  return false;
}

ssize_t Socket::readAvailable(char* buffer, size_t capacity) {
  ssize_t byte_count = ::recv(this->sharedSocket->socketFileDescriptor, buffer,
      capacity, 0);  // MSG_PEEK
  if (byte_count == 0) {  // Peer disconnected
    this->close();
  }
  return byte_count;
}

ssize_t Socket::read(char* buffer, size_t size) {
  ssize_t received = 0;
  while (size_t(received) < size) {
    const ssize_t result =
      this->readAvailable(buffer + received, size - received);
    // If error happened, stop and return 0 or negative
    if (result == 0) {  // Peer disconnected
      this->close();
    }
    if (result <= 0) {
      return result;
    }
    // Success
    received += result;
    assert(size_t(received) <= size);
  }
  return received;
}

bool Socket::send() {
  // Send the bytes stored in internal buffer
  const std::string& text = this->sharedSocket->output.str();
  const char* buffer = text.c_str();
  const size_t length = text.length();
  const ssize_t result = this->write(buffer, length);
  // If all bytes were sent, clear the buffer
  if (size_t(result) >= length) {
    this->sharedSocket->output.str("");
  }
  return result;
}

ssize_t Socket::write(const char* buffer, size_t size) {
  ssize_t sent = 0;
  // Try and retry to send the data until all data is sent to peer
  while (true) {
    // Send a chunk of data, result indicates the amount of bytes sent in this
    // call to send()
    const ssize_t result =
      ::send(this->sharedSocket->socketFileDescriptor, buffer + sent
        , size - sent, 0);
    // If peer disconnected
    if (result == 0) {
      this->close();  // We close the socket, instead of reconnect
    }
    // If an error happened including disconnection, return the error code
    if (result <= 0) {
      return result;
    }
    // No errors happened, increase the amount of bytes effectively sent
    sent += result;
    // If all bytes were sent, stop the loop
    if (size_t(sent) >= size) {
      return sent;  // success: all data was sent
    }
  }
}

Socket::operator bool() const {
  return this->sharedSocket->isOk();
}

bool Socket::isConnected() {
  const int fd = this->sharedSocket->socketFileDescriptor;
  if (fd < 0) {
    return false;
  }
  char dummy = '\0';
  const ssize_t read_bytes = ::recv(fd, &dummy, sizeof(dummy), MSG_PEEK);
  if (read_bytes > 0) {
    return true;
  }
  if (read_bytes == 0) {
    this->close();
  }
  // TODO(jhc): An error happened, we do not know if it is connected or not
  return false;
}
