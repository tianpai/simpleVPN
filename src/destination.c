/* Importing the libraries needed */
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "decap.h"
#include "host_info.h"
#include "packet.h"
#include "protocol.h"

int create_dest_socket() {
  int dest_fd;
  struct sockaddr_in address;
  int opt = 1;

  /* Creating socket file descriptor */
  if ((dest_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  /* Attaching socket */
  if (setsockopt(dest_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  /* Assigning the socket to IP and port */
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  /* Binding socket */
  if (bind(dest_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  /* wait for incoming connections from clients */
  if (listen(dest_fd, 3) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  return dest_fd;
}

int accept_dest_connection(int dest_fd) {
  struct sockaddr_in address;
  int new_socket;
  int addrlen = sizeof(address);

  // Accepting the client connection
  if ((new_socket = accept(dest_fd, (struct sockaddr *)&address,
                           (socklen_t *)&addrlen)) < 0) {
    perror("accept");
    exit(EXIT_FAILURE);
  }

  return new_socket;
}

int main(int argc, char const *argv[]) {
  /* read destination host name from commandline inputs */
  if (argc != 2) {
    printf("Unexpected number of arguements. \
    Enter arguements: VPNserver_host_name");
    return 0;
  }
  const char *dest_host_name = argv[1];

  /* get destination IP address */
  const char *host_ip = get_host_ip(dest_host_name);
  if (host_ip == NULL) {
    printf("Invalid host name: %s\n", dest_host_name);
    return 0;
  }

  /* set up socket to send and receive packets */
  int dest_fd = create_dest_socket();
  int new_socket = accept_dest_connection(dest_fd);

  /* receive packet */
  uint8_t *rec_packet = NULL;
  rec_packet = dest_rec_pkt(new_socket);
  if (rec_packet == NULL) {
    printf("Error receiving packet.\n");
    return -1;
  }

  /* print packet */
  print_packet_unencap(rec_packet);

  /* send response packet */
  const char *response_msg = "Hello from the other side!";
  ip_hdr_t *ip_header = (ip_hdr_t *)rec_packet; /* we don't have GRE */

  char *client_ip_str = malloc(sizeof(char) * 16);
  parse_ip_addr_to_str(client_ip_str, ip_header->ip_src);

  uint8_t *response_pkt = create_packets_des(
      host_ip, client_ip_str, ip_protocol_tcp, response_msg, tcp_flag_ack);
  /* Need to decap since this part should not be encapsulated */
  uint8_t *fixed_pkt = packet_decapsulate(response_pkt);

  print_packet_unencap(fixed_pkt);

  size_t pack_len =
      (size_t)get_packet_size(ip_protocol_tcp, strlen(response_msg));
  if (send(new_socket, fixed_pkt, pack_len, 0) == -1) {
    return -1;
  }

  /* clean up */
  free(rec_packet);
  free(response_pkt);
  free(client_ip_str);
  close(new_socket);
  close(dest_fd);

  return 0;
}
