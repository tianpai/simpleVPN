/* Created by In Kim Nov 14, 2023
 * CSCD58 final project
 * Simple VPN
 */

/* Importing the libraries needed */
#include "client.h"
#include "host_info.h"
#include "packet.h"
#include "protocol.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h> // TODO: remove - used for testing
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

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

/**
 * commandline arguements:
 * - client_host_name: the client's host name on Mininet
 * - server_host_name: the VPN server's host name on Mininet
 */
int main(int argc, char const *argv[]) {
  /* read commandline arguements */
  if (argc != 3) {
    printf("Unexpected number of arguements. \
    Enter arguements: client_host_name VPNserver_host_name");
    return 0;
  }
  const char *client_host_name = argv[1];
  const char *server_host_name = argv[2];

  char dest_host_name[MAX_NAME_LENGTH];
  printf("Enter the destination name (must be less than 5 characters): ");
  fgets(dest_host_name, MAX_NAME_LENGTH, stdin);
  dest_host_name[strcspn(dest_host_name, "\n")] = '\0';

  /* get IP addresses of hosts */
  const char *client_ip = get_host_ip(client_host_name);
  const char *server_ip = get_host_ip(server_host_name);
  const char *dest_ip = get_host_ip(dest_host_name);

  if ((client_ip == NULL) || (server_ip == NULL) || (dest_ip == NULL)) {
    printf("Invalid client, server, or destination name.\n");
    return 0;
  }

  if ((strcmp(client_ip, server_ip) == 0) ||
      (strcmp(client_ip, dest_ip) == 0) || (strcmp(server_ip, dest_ip) == 0)) {
    printf("Cannot have same name for client, server, or client.\n");
    return 0;
  }
  /* Create client socket */
  int client_fd = create_client_socket();
  if (client_fd == -1) {
    return -1;
  }

  /* Connect client to server */
  if (connect_to_server(client_fd, server_ip) == -1) {
    close(client_fd);
    return -1;
  }
  printf("Checking before create packet.\n");
  uint8_t *packet = create_packets(client_ip, dest_ip, ip_protocol_tcp,
                                   "payload", tcp_flag_syn);
  print_packet(packet);

  if (send_and_free_packet_vpn(client_fd, packet, ip_protocol_tcp,
                               strlen("payload")) == -1) {
    fprintf(stderr, "Error during sending packet to the server via socket.\n");
  }

  uint8_t *resp_pkt = NULL;
  resp_pkt = serv_rec_from_cli(client_fd);

  print_packet(resp_pkt);
  free(resp_pkt);

  /* Close the client socket */
  close(client_fd);
  return 0;
}
