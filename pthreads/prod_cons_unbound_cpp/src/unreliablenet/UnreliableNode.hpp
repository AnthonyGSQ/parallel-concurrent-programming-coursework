// Copyright 2020-2024 Jeisson Hidalgo-Cespedes. ECCI-UCR. CC BY 4.0
#pragma once

#include <cstddef>
#include <vector>

#include "common.hpp"
#include "Queue.hpp"
#include "Socket.hpp"
#include "TcpServer.hpp"

class ReliableServer;
class UnreliableConnectionHandler;
class UnreliableClient;

/**
 * @brief Simulates an unreliable network where servers stop listening randomly
 * and clients disconnects randomly, and they re-establish again automatically.
 * 
 * This is the main controller object that also is a reliable TCP Server that
 * always listen for connections. It is reliable to keep example simple.
 * However connections with other nodes are not reliable. They disconnect
 * randomly. Therefore nodes pass keep-alive messages each @a delay milliseconds
 * in order to keep an updated state of the network.
 */
class UnreliableNode : public TcpServer {
  DISABLE_COPY(UnreliableNode);

 private:
  /// Port where this server is listening for connections. It is determined
  /// at runtime by trial-error until finding a free port
  int port = 0;
  /// Delay in milliseconds between keep-alive messages
  int delay = 1000;
  /// Socket queue of accepted connections between server and conn. handlers
  Queue<Socket> acceptedConnections;
  /// Server do not handle connections. These objects block receiving data
  // std::vector<UnreliableConnectionHandler*> connectionHandlers;
  /// Clients connect to [the server of] other nodes
  // std::vector<UnreliableClient*> clients;

 public:
  /// Constructor
  UnreliableNode() = default;
  /// Destructor
  ~UnreliableNode();
  /// Start the simulation
  int run(int argc, char* argv[]);

 private:
  /// Analyze the command line arguments
  int analyzeArguments(int argc, char* argv[]);
  /// Create network threads: producers, consumers, assemblers and dispatchers
  int findAvailablePort(const int startPort);
  /// Intercommunicate threads using queues
  void connectNode(const int port);
  /// Start the execution of threads
  void startThreads();
  /// Wait (join) the secondary threads
  void joinThreads();

 protected:
  /// This method is called each time a client connection request is accepted.
  /// Inherited classes must override this method, process the connection
  /// request, and finally close the connection socket
  void handleClientConnection(Socket& client) override;
};
