#include "TCPClient.h"

int tcp_client_init(tcp_client *self, const char *host, const char *port) {
  if (self == NULL) {
    return -1;
  }
  self->fd = -1;

  // Ensure the host name or port are not too long
  if (strlen(host) > HOST_SIZE - 1 || strlen(port) > PORT_SIZE - 1) {
    return -1;
  }

  // Copy host and port
  strncpy(self->host, host, HOST_SIZE - 1);
  self->host[HOST_SIZE - 1] = '\0';

  strncpy(self->port, port, PORT_SIZE - 1);
  self->port[PORT_SIZE - 1] = '\0';
  return 0;
}

int tcp_client_connect(tcp_client *self) {
  if (self == NULL) {
    return -1;
  }

  struct addrinfo hints = {0}, *res = NULL;
  hints.ai_socktype = SOCK_STREAM;
  // Look for TCP connections to the host and port and store the results in res
  // as a linked list
  if (getaddrinfo(self->host, self->port, &hints, &res) != 0) {
    return -1;
  }

  int fd = -1;
  // Iterate over socket options
  for (struct addrinfo *rp = res; rp; rp = rp->ai_next) {
    // Attempt to create the socket
    fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (fd < 0) {
      continue;
    }

    // Attempt to connect the socket
    if (connect(fd, rp->ai_addr, rp->ai_addrlen) == 0) {
      // Break out of loop if socket successfully connected
      break;
    }

    // Close file descriptor if it failed to connect
    close(fd);
    fd = -1;
  }

  // Free memory used by res
  freeaddrinfo(res);
  if (fd < 0) {
    return -1;
  }

  // Set file descriptor if successfully connected
  self->fd = fd;
  return 0;
}

void tcp_client_disconnect(tcp_client *self) {
  // Close file descriptor
  if (self->fd >= 0) {
    close(self->fd);
  }
  self->fd = -1;
}

int tcp_client_read(tcp_client *self, void *buf, size_t len) {
  if (self->fd < 0) {
    return -1;
  }
  return recv(self->fd, buf, len, MSG_NOSIGNAL);
}

int tcp_client_write(tcp_client *self, const void *buf, size_t len) {
  return send(self->fd, buf, len, MSG_DONTWAIT);
}

void tcp_client_dispose(tcp_client *self) { tcp_client_disconnect(self); }
