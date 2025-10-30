#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "TCPClient.h"

#define MAX_CLIENTS 10
#define SERVER_BUFFER_SIZE 512

typedef struct {
  int listen_fd;
  tcp_client clients[MAX_CLIENTS];
} tcp_server;

/**
 * @brief Initiate a TCP server.
 *
 * Initiates a TCP server on a given port.
 *
 * @param self A pointer to the tcp_server to be initiated.
 * @param port The port on on which the server is to listen.
 *
 * @return An error code.
 */
int tcp_server_initiate(tcp_server *self, const char *port);

/**
 * @brief Accept incoming connection.
 *
 * Accepts an incoming connection. If no connection is waiting to be accepted
 * this does nothing.
 *
 * @param self The server.
 *
 * @return An error code. 0 is returned if there is no incoming connection
 *
 */
int tcp_server_accept(tcp_server *self);

/**
 * @brief Echo work function.
 *
 * The server accepts an incoming connection and then reads from each connection
 * and replies with the same data.
 *
 * @param self The server.
 */
void tcp_server_work(tcp_server *self);

/**
 * @brief Dispose a TCP server
 *
 * Disposes a TCP server by disposing each of its connections and closing its
 * listening file descriptor.
 *
 * @param self The server.
 *
 * @note The caller is responsible for freeing the memory used by the server, if
 * any.
 *
 */
void tcp_server_dispose(tcp_server *self);

// Inline function for setting a file descriptor as non-blocking
static inline int tcp_server_nonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags < 0)
    return -1;

  return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}
#endif