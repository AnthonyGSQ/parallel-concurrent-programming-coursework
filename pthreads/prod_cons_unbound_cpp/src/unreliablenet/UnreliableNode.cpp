// Copyright 2020-2024 Jeisson Hidalgo-Cespedes. ECCI-UCR. CC BY 4.0

#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "Log.hpp"
#include "UnreliableNode.hpp"
// #include "UnreliableConnectionHandler.hpp"
// #include "UnreliableClient.hpp"

const char* const usage =
  "Usage: unrnet [delay]\n"
  "\n"
  "  delay  milliseconds between messages, negative for random maximum\n";

const int START_PORT = 10000;

UnreliableNode::~UnreliableNode() {
  // for (UnreliableConnectionHandler* handler : this->connectionHandlers) {
  //   delete handler;
  // }
  // for (UnreliableClient* client : this->clients) {
  //   delete client;
  // }
}

int UnreliableNode::run(int argc, char* argv[]) {
  // Parse arguments and store values in this object's attributes
  if (int error = this->analyzeArguments(argc, argv)) {
    return error;
  }
  // Find the next available port and open it for listening for connections
  this->port = this->findAvailablePort(START_PORT);
  std::cerr << "Listening on localhost:" << this->port << std::endl;
  // Create clients to connect to the other unreliable nodes
  const int otherNodes = this->port - START_PORT;
  for (int counter = 0; counter < otherNodes; ++counter) {
    this->connectNode(START_PORT + counter);
  }
  try {
    // Accept connections, for each one create a client
    this->acceptAllConnections();
  } catch (...) {
    // TODO(jhc): Log::
  }
  // Wait for objects to finish the simulation
  this->joinThreads();
  // Simulation finished
  return EXIT_SUCCESS;
}

int UnreliableNode::analyzeArguments(int argc, char* argv[]) {
  if (argc >= 2) {
    if (std::strcmp(argv[0], "--help") == 0) {
      std::cout << usage;
    } else {
      this->delay = std::atoi(argv[1]);
    }
  }
  return EXIT_SUCCESS;
}

int UnreliableNode::findAvailablePort(const int startPort) {
  for (int current = startPort; current < 65536; ++current) {
    try {
      this->listenForConnections(std::to_string(current).c_str());
      return current;  // This port was available, use it
    } catch (const std::runtime_error& error) {
      // Do nothing, try next port
      std::cerr << "error: " << error.what() << std::endl;
    }
  }
  return 0;
}

void UnreliableNode::connectNode(const int port) {
  std::cerr << this->port << ": connecting to node " << port << std::endl;
}

void UnreliableNode::handleClientConnection(Socket& client) {
  this->acceptedConnections.enqueue(client);
}

void UnreliableNode::joinThreads() {
  // for (UnreliableConnectionHandler* handler : this->handlers) {
  //   handler->waitToFinish();
  //   delete handler;
  // }
  // for (UnreliableClient* client : this->clients) {
  //   client->waitToFinish();
  //   delete client;
  // }
}
