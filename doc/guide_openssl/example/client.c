#include "encrypt.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_IP "10.0.0.2"
#define PORT 4433
#define MSG "Hello, server!"

void start_client() {
  int client_fd;
  struct sockaddr_in server_addr;
  SSL_CTX *ctx;
  SSL *ssl;

  // Create socket
  client_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (client_fd == -1) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Connect to server
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
  if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("Connect failed");
    exit(EXIT_FAILURE);
  }

  // SSL setup
  ctx = create_ssl_client_context();
  ssl = SSL_new(ctx);
  SSL_set_fd(ssl, client_fd);

  if (SSL_connect(ssl) <= 0) {
    ERR_print_errors_fp(stderr);
  } else {
    printf("SSL connection established\n");
    // Communication with server...
    int bytes = SSL_write(ssl, MSG, strlen(MSG));
    if (bytes <= 0) {
      ERR_print_errors_fp(stderr);
    } else {
      printf("Sent %d bytes to server\n", bytes);
    }
  }

  // Cleanup
  ssl_shutdown(ssl);
  SSL_CTX_free(ctx);
  close(client_fd);
}

int main() {
  init_ssl_lib();
  start_client();
  cleanup_ssl_lib();
  return 0;
}
