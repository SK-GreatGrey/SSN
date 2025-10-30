#include "TCPServer.h"
#include <errno.h>

int tcp_server_initiate(tcp_server *self, const char *port) {
  if (self == NULL) {
    return -1;
  }
  struct addrinfo hints = {0}, *res = NULL;
  // Use any address family (IPv4 or IPv6)
  hints.ai_family = AF_UNSPEC;
  // The socket is to use TCP
  hints.ai_socktype = SOCK_STREAM;
  // Allow getddrinfo to tell us where to bind the socket
  hints.ai_flags = AI_PASSIVE;

  // Find suitable socket options and store them in res as a linked list
  if (getaddrinfo(NULL, port, &hints, &res) != 0) {
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

    int yes = 1;
    // Attempt to bind the socket
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    if (bind(fd, rp->ai_addr, rp->ai_addrlen) == 0) {
      // Break out of loop if socket is successfully created and bound
      break;
    }

    // Close socket file descriptor if it failed to bind
    close(fd);
    fd = -1;
  }

  // Free memory used by res
  freeaddrinfo(res);
  if (fd < 0) {
    printf("Failed to initialize host");
    return -1;
  }

  // Attempt to listen for connections on the socket
  if (listen(fd, MAX_CLIENTS) < 0) {
    close(fd);
    return -1;
  }

  // Set the socket as non-blocking
  tcp_server_nonblocking(fd);

  // Set the socket as the servers listening file descriptor
  self->listen_fd = fd;
  // Initialize all client file descriptors to -1
  for (int i = 0; i < MAX_CLIENTS; i++) {
    self->clients[i].fd = -1;
  }

  return 0;
}

void tcp_server_dispose(tcp_server *self) {
  // Dispose all clients
  for (int i = 0; i < MAX_CLIENTS; i++) {
    tcp_client_dispose(&self->clients[i]);
  }
  // Close socket, if any
  if (self->listen_fd >= 0) {
    close(self->listen_fd);
  }
  self->listen_fd = -1;
}

int tcp_server_accept(tcp_server *self) {
  // Accept a connection
  int client_fd = accept(self->listen_fd, NULL, NULL);
  if (client_fd < 0) {
    // Accept returns an error if there are no new connections
    // this is not considered an error
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return 0;
    }
    return -1;
  }

  // Set the new client connection as non-blocking
  tcp_server_nonblocking(client_fd);

  // Find a space for the new connection to be stored
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (self->clients[i].fd < 0) {
      self->clients[i].fd = client_fd;
      printf("Client #%d connected\n", i);
      return 0;
    }
  }

  // Close the file descriptor if no space was found
  close(client_fd);
  return 1;
}

void tcp_server_work(tcp_server *self /*, SMW smw*/) {
  // Accept a new connection
  tcp_server_accept(self);

  char buf[SERVER_BUFFER_SIZE];

  // Iterate over each client
  for (int i = 0; i < MAX_CLIENTS; i++) {
    tcp_client *client = &self->clients[i];
    // Skip inactive clients
    if (client->fd < 0) {
      continue;
    }

    // Read data from client
    size_t n = tcp_client_read(client, buf, sizeof(buf));

    if (n > 0) {
      // Write data back to client, if any
      tcp_client_write(client, buf, n);
    } else if (n == 0) {
      // Close connection when there is no more data to read
      printf("Client #%d disconnected\n", i);
      tcp_client_dispose(client);
    }
  }
}
