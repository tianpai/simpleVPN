#include "encrypt.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_IP "10.0.0.2"
#define PORT 4433

void start_server() {
  int server_fd, client_fd;
  struct sockaddr_in server_addr, client_addr;
  socklen_t client_addr_size;
  SSL_CTX *ctx;
  SSL *ssl;

  // Create socket
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Bind socket to IP/Port
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("Bind failed");
    exit(EXIT_FAILURE);
  }

  // Listen
  if (listen(server_fd, 1) < 0) {
    perror("Listen failed");
    exit(EXIT_FAILURE);
  }

  printf("Server listening on %s:%d\n", SERVER_IP, PORT);

  // Accept connection
  client_addr_size = sizeof(client_addr);
  client_fd =
      accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_size);
  if (client_fd < 0) {
    perror("Accept failed");
    exit(EXIT_FAILURE);
  }

  // SSL setup
  ctx = create_ssl_server_context();
  configure_context(ctx, "server_cert.pem",
                    "server_key.pem"); // Use your own cert and key files
  ssl = SSL_new(ctx);
  SSL_set_fd(ssl, client_fd);

  if (SSL_accept(ssl) <= 0) {
    ERR_print_errors_fp(stderr);
  } else {
    printf("SSL connection established\n");
    // Communication with client...
    char buf[1024] = {0};
    int bytes = SSL_read(ssl, buf, sizeof(buf) - 1);
    if (bytes > 0) {
      buf[bytes] = '\0';
      printf("Received: %s\n", buf);
    } else {
      ERR_print_errors_fp(stderr);
    }
  }

  // Cleanup
  ssl_shutdown(ssl);
  SSL_CTX_free(ctx);
  close(client_fd);
  close(server_fd);
}

int main() {
  init_ssl_lib();
  start_server();
  cleanup_ssl_lib();
  return 0;
}
