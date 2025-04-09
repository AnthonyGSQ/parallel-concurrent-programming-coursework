// Copyright 2021 Jeisson Hidalgo-Cespedes. Universidad de Costa Rica. CC BY 4.0

#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <netdb.h>

#include "common.hpp"

class NetworkAddress;
class Socket;

/**
 * @brief Implements a Server that talks Transmission Control Protocol (TCP).
 *
 * TCP ensures data transmission send from a sender process to arrive complete
 * and maintaining the same order to other to the receiving process, unless an
 * error occurs. User Datagram Protocol (UDP) does not provide these warranties,
 * so UDP is cheaper in resources.
 *
 * TCP is connection-oriented. The connection concept comes from Electrical
 * Engineering, where two extremes are connected by a cable that transports
 * electricity (e.g: a telephone). Any signal made by an extreme of the cable
 * is transported to the other end. Therefore, a connection provides to send
 * information in full-duplex (both directions).
 *
 * Processes in the extremes of a connections must play a role. One of them
 * must be a Server and the other must be a Client. The TcpServer class
 * implements the Server role. The TcpClient class implements the client role.
 * 
 * The Server process is listening for connection requests, usually for a long
 * time. Clients connect when they want. When a Client tries to connect, it
 * sends a connection request to the port where the Server is listening to.
 * If Server accepts the connection request, a Socket is created. The Socket
 * is the "wire" that allows to transport data from one process to the other.
 * A Socket is simply a file with two cursors: one for writing (send data), and
 * other for reading (receiving data).
 * 
 * A TcpServer can be used in two ways: using composition or inheritance.
 * You can see a composition example in documentation of @a Socket class.
 * Alternatively you can inherit a class and override the
 * @a handleClientConnection() method. This method is called each time a new
 * connection is established with a client. The you may call @a listenForever()
 * method to put the TCP to listen for connection requests in some port.
 *
 * @code
  // MyServer.cpp
  class MyServer : public TcpServer {
   public:
    void run() {
      this->listenForConnections("9000");
      while (this->isListening()) {  // Same as this->listenForever("9000")
        this->acceptConnectionRequest();  // Call handleClientConnection()
      }
    }
   protected:
    void handleClientConnection(Socket& client) override {
      const int lucky = 13;
      socket << "ServerLucky " << lucky;
      std::string line;
      socket.readLine(line);
      std::cout << "Client said: " << line << std::endl;
    }
  };

  int main() {
    MyServer().run();
  }
 * @endcode
 *
 * Network ports are numbers between 1 and 65535. In order to listening on ports
 * in range [1, 1024[ require administration privileges. Failure to listening
 * on a network port typically happens when other process is using it or crashed
 * without closing it.
 */
class TcpServer {
  DISABLE_COPY(TcpServer);

 public:
  /// Default number of pending connection requests allowed in the queue
  static const int defaultConnectionQueueCapacity = 10;

 protected:
  /// Lookup criteria for searching network information about this host
  struct addrinfo hints;
  /// Available network addresses for listening in current host
  struct addrinfo* availableAddresses = nullptr;
  /// Selected network address for this TCP server
  const struct addrinfo* selectedAddress = nullptr;
  /// Socket for listening for incoming connection requests
  int connectionRequestSocket = -1;
  /// Maximum number of pending connection requests allowed in the queue
  /// This queue is called backlog in the Unix sockets manual (man listen)
  int connectionQueueCapacity = defaultConnectionQueueCapacity;

 public:
  /// Constructor
  explicit TcpServer(
    const int connectionQueueCapacity = defaultConnectionQueueCapacity);
  /// Destructor
  virtual ~TcpServer();
  /// Indefinitely listen for client connection requests and accept all of them.
  /// For each accepted connection request, the virtual onConnectionAccepted()
  /// will be called. Inherited classes must override that method
  void listenForever(const char* port);
  /// Stop listening for client connections. This method should be called when
  /// the server stops, for example, when a user presses Ctrl+C, sends a kill
  /// signal, or when the operating system is shutting down
  void stopListening();
  /// Start listening for client connections at the given port.
  /// This is a non blocking method. It just asks the operating system to create
  /// a queue of connection requests (its capacity is provided in constructor).
  /// After calling this method, the OS will just start feeding the queue with
  /// connection requests from clients. To accept the next connection request
  /// stored within the queue, call @a acceptConnectionRequest()
  /// @throw std::runtime_error on any error condition
  void listenForConnections(const char* port);
  /// Repeatedly and sequentially accept all incoming connection requests.
  /// For each accepted connection request, the pure virtual method @a
  /// handleClientConnection() is called. This method stops when the socket
  /// to listen for connections is closed calling @a stopListening() method.
  void acceptAllConnections();
  /// Accept the next pending incoming connection request on the queue. If
  /// queue is empty, the caller execution thread will be blocked, otherwise the
  /// operating system will create a socket that allows communicate with the
  /// accepted client. There are two ways to handle that socket:
  /// - You will get a copy of the Socket by the return value of this method.
  /// You can use the socket to send to and receive messages from the client.
  /// If you want to accept another connection, you will call this method again,
  /// probably within a loop.
  /// - You can inherit this TcpServer class. In the derived class override the
  /// the virtual method @a handleClientConnection(). This method will be called
  /// for each accepted connection, e.g: if you call @a listenForever().
  Socket acceptConnectionRequest();
  /// Get the network address (IP and port) where this server is listening
  NetworkAddress getNetworkAddress() const;

 protected:
  /// Fetch all available network addresses where we can listen with this port
  void fetchAvailableAddresses(const char* port);
  /// Get a socket for the first available address that works for the given port
  /// Return 0 on success, -1 on failure. Typical failure is trying to
  /// open a port that is used by other process or it is [0, 1024[ and process
  /// does not have administration privileges.
  int openConnectionRequestSocket(const char* port);
  /// This method is called each time a client connection request is accepted.
  /// Inherited classes must override this method, process the connection
  /// request, and finally close the connection socket
  /// TODO(jhc): Make acceptConnectionRequest return Socket and this method may
  /// be virtual but not abstract
  virtual void handleClientConnection(Socket& client) {
    (void)client;  // Do nothing in base class
  }
  /// Release the memory used by the network address record
  void freeNetworkAddress();
  /// Close the socket file descriptor
  void closeSocket();
};

#endif  // TCPSERVER_H
