// Copyright 2021-2025 Jeisson Hidalgo-Cespedes. ECCI-UCR. CC BY 4.0

#include <string>

#include "Log.hpp"
#include "NetworkAddress.hpp"
#include "SharedSocket.hpp"

SharedSocket::SharedSocket() {
  ::memset(&this->peerAddress, 0, sizeof(this->peerAddress));
}

SharedSocket::~SharedSocket() {
  this->close();
}

void SharedSocket::close() {
  if (this->socketFileDescriptor >= 0) {
    // Close connection with client
    ::close(this->socketFileDescriptor);
    // Log the disconnection event
    const NetworkAddress& address = this->getNetworkAddress();
    Log::append(Log::INFO, "socket", std::string("connection -----closed")
      + address.getIP() + " port " + std::to_string(address.getPort()));
    // Reset network address memory
    ::memset(&this->peerAddress, 0, sizeof(this->peerAddress));
    this->socketFileDescriptor = -1;
  }
}

NetworkAddress SharedSocket::getNetworkAddress() const {
  return NetworkAddress(this->getSockAddr());
}
