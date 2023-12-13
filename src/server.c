/* Created by In Kim Nov 14, 2023
 * CSCD58 final project
 * Simple VPN
 */

/* Importing the libraries needed */

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "encap.h"
#include "host_info.h"
#include "packet.h"
#include "protocol.h"
#include "server.h"

int create_client_socket() {
  int client_fd;

  /* Creating a socket */
  if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("\n Socket creation error \n");
    return -1;
  }

  return client_fd;
}

int connect_to_server(int client_fd, const char *server_ip) {
  struct sockaddr_in serv_addr;

  /* Setting up server address */
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  /* Convert IPv4 address from text to binary form */
  if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  /* Connect to server */
  if (connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <
      0) {
    printf("\nConnection Failed \n");
    return -1;
  }
  printf("Connected to server\n");

  return 0;
}

int create_server_socket() {
  int server_fd;
  struct sockaddr_in address;
  int opt = 1;

  /* Creating socket file descriptor */
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  /* Attacking socket */
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  /* Assigning the socket to IP and port */
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  /* Binding socket */
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  /* wait for incoming connections from clients */
  if (listen(server_fd, 3) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  return server_fd;
}

int accept_client_connection(int server_fd) {
  struct sockaddr_in address;
  int new_socket;
  int addrlen = sizeof(address);

  // Accepting the client connection
  if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                           (socklen_t *)&addrlen)) < 0) {
    perror("accept");
    exit(EXIT_FAILURE);
  }

  return new_socket;
}

int main(int argc, char const *argv[]) {
  if (argc != 2) {
    printf("Unexpected number of arguements. \
    Enter arguements: VPNserver_host_name");
    return 0;
  }
  const char *server_host_name = argv[1];

  /* get IP addresses of hosts */
  const char *server_ip = get_host_ip(server_host_name);
  if (server_ip == NULL) {
    printf("Invalid server name.\n");
    return 0;
  }

  int server_fd = create_server_socket();

  int new_socket = accept_client_connection(server_fd);

  uint8_t *rec_packet = NULL;
  rec_packet = serv_rec_from_cli(new_socket);
  if (rec_packet == NULL) {
    fprintf(stderr, "Error occured when rec packet from client via socket.\n");
    return -1;
  }

  print_packet(rec_packet);
  uint32_t client_ip = 0;
  save_client_ip(&client_ip, rec_packet);
  uint8_t *fixed_pkt = serv_handle_pkt(rec_packet, server_ip);

  /* Create client socket */
  int client_fd = create_client_socket();
  if (client_fd == -1) {
    return -1;
  }

  char *dest_ip = malloc(4 * 4 * sizeof(char));
  ;
  parse_ip_addr_to_str(dest_ip, ((ip_hdr_t *)fixed_pkt)->ip_dst);
  /* Connect client to server */
  if (connect_to_server(client_fd, dest_ip) == -1) {
    close(client_fd);
    return -1;
  }
  free(dest_ip);

  size_t pack_len =
      (size_t)(MAX_PAYLOAD_SIZE + sizeof(tcp_hdr_t) + sizeof(ip_hdr_t));
  if (send(client_fd, fixed_pkt, pack_len, 0) == -1) {
    fprintf(stderr, "Error during sending packet to the server via socket.\n");
    return -1;
  }

  free(rec_packet);

  printf("---Packet sent to the destination. Waiting for a response.---\n");

  uint8_t *rec_packet_dest = NULL;
  rec_packet_dest = serv_rec_from_des(client_fd);

  uint8_t packet_size = (uint8_t)(MAX_PAYLOAD_SIZE + sizeof(tcp_hdr_t) +
                                  sizeof(gre_hdr_t) + sizeof(ip_hdr_t));

  uint8_t *enc_pkt = (uint8_t *)calloc(packet_size, sizeof(uint8_t));
  packet_encapsulate(enc_pkt);
  memcpy((uint8_t *)(enc_pkt + sizeof(gre_hdr_t)), rec_packet_dest,
         sizeof(ip_hdr_t) + sizeof(tcp_hdr_t) + MAX_PAYLOAD_SIZE);

  char *client_str_ip = malloc(4 * 4 * sizeof(char));
  parse_ip_addr_to_str(client_str_ip, client_ip);

  serv_handle_pkt_dest(enc_pkt, client_str_ip);

  print_packet(enc_pkt);
  serv_cli_encrypt_free(new_socket, enc_pkt, ip_protocol_tcp,
                        strlen((char *)(enc_pkt + sizeof(gre_hdr_t) +
                                        sizeof(ip_hdr_t) + sizeof(tcp_hdr_t))));

  /* Freeing any allocs */
  free(rec_packet_dest);
  free(client_str_ip);
  /* closing the connected socket */
  close(new_socket);
  /* Shutting down the server */
  shutdown(server_fd, SHUT_RDWR);

  return 0;
}
