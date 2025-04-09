// Copyright 2021-2024 Jeisson Hidalgo-Cespedes. ECCI-UCR. CC BY 4.0

#ifndef SOCKET_H
#define SOCKET_H

#include <memory>
#include <string>

#include "common.hpp"
#include "SharedSocket.hpp"

class NetworkAddress;
struct SharedSocket;

/**
 * @brief A socket is a file that communicates two processes.
 *
 * If a socket intercommunicates two processes A and B, bytes written by A are
 * transported to B, no matter if A and B are at the same machine or located
 * in different countries. B can read those bytes. If B tries to read the socket
 * before the bytes arrive, it will be locked until they arrive. It is the same
 * behavior reading from any other file, such as standard input (stdin). If
 * a program read from stdin, it will be locked until user types some chars by
 * keyboard and press Enter.
 * 
 * By default a socket is full-duplex, therefore bytes written by B will be
 * transported to A, and A can read them from the socket. The following example
 * shows a TCP server A and a client B sharing textual messages.
 * 
 * @code
  // MyServer.cpp
  int main() {
    try {
      MyServer myServer;
      // Ask OS to enqueue connection requests made to port 9000
      myServer.listenForConnections("9000");
      // Accept only one client (the first one in queue) and return
      Socket socket = myServer.acceptConnectionRequest();
      const int lucky = 13;
      socket << "ServerLucky " << lucky;
      std::string line;
      socket.readLine(line);
      std::cout << "Client said: " << line << std::endl;
    } catch (const std::exception& exception) {
      std::cerr << "error: " << exception.what() << std::endl;
    }
  }

  // MyClient.cpp
  int main() {
    try {
      TcpClient client;
      Socket server = client.connect("0.0.0.0", "9000");
      std::string serverId;
      int serverLucky = -1;
      const int myLucky = 20;
      if (server >> serverId >> serverLucky && serverId == "ServerLucky") {
        server << "Our lucky sums " << serverLucky + myLucky << std::endl;
      }
    } catch (const std::exception& exception) {
      std::cerr << "error: " << exception.what() << std::endl;
    }
  }
 * @endcode
 *
 * Sockets use reference-counters and can be copied. In the following example,
 * server1 and server2 share the same internal file. The file will be closed
 * when last copy is destroyed. If you want to keep a connection alive, you
 * can keep copies of a socket stored in containers (e.g: a vector or queue).
 *
 * @code
  {
    Socket server1 = client.connect("10.1.2.3", "8000");
    {
      Socket server2 = server1;  // There are now two instances sharing a socket
      server2 << "Hello ";  // Send "Hello to the server"
    }  // server2 object is destroyed, but does not close the socket
    server1 << "World\n"; // Send "World" to the same server
  }  // server1 is the last copy, socket is closed now
 * @endcode
 */
class Socket {
 public:
  /// Objects of this class can be copied, but avoid unnecessary copies
  DECLARE_RULE4(Socket, default);
  /// Other network classes require to access internal attributes
  friend class TcpServer;
  friend class TcpClient;

 protected:
  /// Copies of this object share the same socket file descriptor and buffers
  std::shared_ptr<SharedSocket> sharedSocket;

 public:
  /// Construct an empty invalid socket
  Socket();
  /// Destructor. Closes the socket file descriptor if this is the last object
  /// using a shared socket
  ~Socket() = default;

 public:
  /// Build a human-readable network address object of the peer
  NetworkAddress getNetworkAddress() const;
  /// Closes the network connection with peer
  void close();
  /// Read a line of data received from peer
  /// @return true on success, false on error or connection closed by peer
  bool readLine(std::string& line, char separator = '\n');
  /// Send data accumulated into output buffer to the peer
  /// @return true on success, false on error or connection closed by peer
  bool send();
  /// Receive some data from peer. The read data is stored in the internal
  /// @a input buffer. This function blocks the caller
  /// @return true if a least a byte was read, false on error or connection
  /// closed by peer
  bool receive();
  /// Read the available data from peer into a buffer. The data is transferred
  /// from the socket to the buffer. No data is stored in the internal @a input
  /// buffer. This function may block/ the caller thread if socket is empty
  /// @return The amount of bytes read, 0 on connection closed by peer, -1 on
  /// error and global errno is set to the error code
  ssize_t readAvailable(char* buffer, size_t capacity);
  /// Read the requested amount of bytes from the socket to the given buffer.
  /// No data is stored in the internal @a input buffer. This procedure blocks
  /// the caller thread until the total amount of bytes are read
  /// @return The amount of bytes read, 0 on connection closed by peer, -1 on
  /// error and global errno is set to the error code
  ssize_t read(char* buffer, size_t size);
  /// @brief Send the binary value to the peer immediately, without caching it.
  /// @tparam Type Data type of the value
  /// @param value Value to be sent
  /// @return The amount of bytes sent, 0 on connection closed by peer, -1 on
  /// error and global errno is set to the error code
  template <typename Type>
  inline ssize_t read(const Type& value) {
    return this->read(reinterpret_cast<char*>(&value), sizeof(Type));
  }
  /// Send the binary data to the peer. No No data is stored in the internal
  /// @a output buffer. This procedure blocks the caller thread until the total
  /// amount of bytes are sent or an error happens.
  /// @return The amount of bytes sent, 0 on connection closed by peer, -1 on
  /// error and global errno is set to the error code
  ssize_t write(const char* buffer, size_t size);
  /// @brief Send the binary value to the peer immediately, without caching it.
  /// @tparam Type Data type of the value
  /// @param value Value to be sent
  /// @return The amount of bytes sent, 0 on connection closed by peer, -1 on
  /// error and global errno is set to the error code
  template <typename Type>
  inline ssize_t write(const Type& value) {
    return this->write(reinterpret_cast<const char*>(&value), sizeof(Type));
  }
  /// Evaluates this object within a boolean context as true if previous
  /// read and write operations were successful. Used for statements such as
  /// @code
  /// std::vector<double> values;
  /// double value = 0.0;
  /// while (socket >> value) {
  ///   values.push_back(value);
  /// }
  /// @endcode
  explicit operator bool() const;
  /// Return true if this socket is connected to a peer
  bool isConnected();
  /// Returns true if this socket represents the same connection than the other
  bool operator==(const Socket& other) const;
  /// Returns true if this socket should be ordered before the other
  /// The sockets are compared by socket file descriptor number
  bool operator<(const Socket& other) const;

 public:
  /// Extraction operator >> used to read values from peer sent data.
  /// If you want to preload data, call @a receive() before this operator
  /// @return This socket object
  template <typename Type>
  Socket& operator>>(Type& value) {
    while (!(this->sharedSocket->input >> value)) {
      if (!this->receive()) {
        return *this;
      }
    }
    return *this;
  }

  /// Insertion operator << used to send values to the peer later. Data is
  /// just stored into a buffer. If you want to the send the data immediately
  /// to peer, call @a send() after the << flow operation.
  /// @return This socket object
  template <typename Type>
  Socket& operator<<(const Type& value) {
    this->sharedSocket->output << value;
    return *this;
  }

 protected:
  /// Get the socket file descriptor, the number that identifies the socket
  int getSocketFileDescriptor() const;
  /// Set the socket file descriptor. This operation is only made by network
  /// classes, such as TcpServer
  void setSocketFileDescriptor(int socketFileDescriptor);
  /// Copies the sockaddr record as the network address for this socket
  void setNetworkAddress(const struct addrinfo* address);
  /// Get read-only access of the storage address as a sockaddr record
  struct sockaddr* getSockAddr();
};

#endif  // SOCKET_H
