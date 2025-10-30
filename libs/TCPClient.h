#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#define _POSIX_C_SOURCE 200112L

#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define HOST_SIZE 256
#define PORT_SIZE 16

// TCP struct: fd, host, port
typedef struct {
  int fd;
  char host[HOST_SIZE];
  char port[PORT_SIZE];
} tcp_client;

/**
 * @brief Initiates a TCP client
 *
 * Initiates a TCP client with a host address and port.
 *
 * @param self The TCP client to initiate.
 * @param host The host to connect to.
 * @param port The port to connect to.
 *
 * @return An error code
 */
int tcp_client_init(tcp_client *self, const char *host, const char *port);

/**
 * @brief Create TCP connection.
 *
 * Attempt to establish a TCP connection to the given host and port.
 *
 * @param self The TCP client.
 *
 * @return An error code.
 */
int tcp_client_connect(tcp_client *self);

/**
 * @brief Read from TCP client.
 *
 * @param self The TCP client.
 * @param buf The buffer to write data to.
 * @param len The max number of bytes to read.
 *
 * @return An error code.
 */
int tcp_client_read(tcp_client *self, void *buf, size_t len);

/**
 * @brief Write to the TCP client.
 *
 * @param self The TCP client.
 * @param buf The buffer to write data from.
 * @param len The number of bytes to send.
 *
 * @return An error code.
 */
int tcp_client_write(tcp_client *self, const void *buf, size_t len);

/**
 * @brief Disconnect a TCP client.
 *
 * @param self The TCP client.
 */
void tcp_client_disconnect(tcp_client *self);

/**
 * @brief Disposes a TCP client
 *
 * @param self The TCP client.
 */
void tcp_client_dispose(tcp_client *self);

#endif
