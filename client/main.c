#include "HTTP.h"
#include "TCPClient.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TIMEOUT 10

typedef void (*http_callback_func)(HTTPResponse *);

typedef struct {
  u_int64_t uuid;
  time_t time;
  double temperature;
} data;

int get_data(u_int64_t sensor_id, data *result) {
  result->uuid = sensor_id;
  time(&result->time);
  result->temperature = 20.0 + ((double)rand() / RAND_MAX) * (10);
  return 0;
}
int send_data(tcp_client *tcp, data *d) {
  HTTPRequest *message = HTTPRequest_new(POST, "/post");
  HTTPRequest_add_header(message, "Host", "httpbin.org");
  HTTPRequest_add_header(message, "Accept", "application/json");

  char content_buf[512];
  snprintf(content_buf, sizeof(content_buf),
           "{\n"
           "\t\"device\":\"%lu\",\n"
           "\t\"time\":\"%ld\",\n"
           "\t\"temperature\":\"%f\"°C\n"
           "}",
           d->uuid, d->time, d->temperature);

  char content_length[10];
  snprintf(content_length, sizeof(content_length), "%d",
           (int)strlen(content_buf));

  HTTPRequest_add_header(message, "Content-Length", content_length);

  char *header_str = (char *)HTTPRequest_tostring(message);
  char message_buf[512];
  snprintf(message_buf, sizeof(message_buf), "%s%s", header_str, content_buf);
  free(header_str);

  int bytes_left = strlen(message_buf);
  int bytes_sent = 0;
  while (bytes_left > 0) {
    int n = tcp_client_write(tcp, message_buf + bytes_sent, bytes_left);
    bytes_sent += n;
    bytes_left -= n;
  }
  HTTPRequest_Dispose(&message);
  return 0;
}
void print_response(HTTPResponse *resp) {
  char *resp_str = (char *)HTTPResponse_tostring(resp);
  printf("%s\n", resp_str);
  free(resp_str);
  HTTPResponse_Dispose(&resp);
}

int get_response(tcp_client *tcp, http_callback_func callback) {
  time_t start_time, current_time;
  time(&start_time);

  int buf_size = 128;
  char *read_buf = (char *)malloc(sizeof(char) * (buf_size + 1));
  int bytes_read = 0;

  char *header_end = NULL;
  do {
    time(&current_time);
    if (difftime(current_time, start_time) > TIMEOUT) {
      free(read_buf);
      return -1;
    }
    int n = tcp_client_read(tcp, read_buf + bytes_read, buf_size - bytes_read);
    if (n < 0)
      continue;
    bytes_read += n;
    if (bytes_read == buf_size) {
      buf_size *= 2;
      char *tmp = (char *)realloc(read_buf, buf_size + 1);
      if (tmp == NULL) {
        free(read_buf);
        return -1;
      }
      read_buf = tmp;
    }
    read_buf[bytes_read] = '\0';
    header_end = strstr(read_buf, "\r\n\r\n");
  } while (header_end == NULL);

  char *content_length_str = strstr(read_buf, "Content-Length: ");
  if (content_length_str == NULL) {
    return -1;
  }

  content_length_str += strlen("Content-Length: ");
  int content_length = (int)strtol(content_length_str, NULL, 10);

  int header_length = header_end - read_buf + 4;

  while (bytes_read - header_length < content_length) {
    // TODO time out
    int n = tcp_client_read(tcp, read_buf + bytes_read, buf_size - bytes_read);
    if (n < 0)
      continue;
    bytes_read += n;
    if (bytes_read == buf_size) {
      buf_size *= 2;
      char *tmp = (char *)realloc(read_buf, buf_size + 1);
      if (tmp == NULL) {
        free(read_buf);
        return -1;
      }
      read_buf = tmp;
    }
    read_buf[bytes_read] = '\0';
  }

  HTTPResponse *resp = HTTPResponse_fromstring(read_buf);
  free(read_buf);
  callback(resp);

  return 0;
}

int main() {
  srand(time(NULL));

  tcp_client tcp;
  int err = tcp_client_init(&tcp, "httpbin.org", "80");

  if (err < 0) {
    perror("Failed to init");
    return -1;
  }

  while (1) {
    data d;
    get_data(1, &d);

    err = tcp_client_connect(&tcp);
    if (err < 0) {
      perror("Failed to connect");
      return -1;
    }
    printf("SENDING: ...\n");
    send_data(&tcp, &d);
    printf("RECIEVING: ...\n");
    get_response(&tcp, print_response);
    tcp_client_disconnect(&tcp);
    sleep(1);
  }
}