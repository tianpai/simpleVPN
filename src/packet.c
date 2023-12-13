
#include <arpa/inet.h>
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "decap.h"
#include "encap.h"
#include "encrypt.h"
#include "host_info.h"
#include "packet.h"
#include "protocol.h"
#include "utils.h"

/* function headers */

uint8_t get_packet_size(uint8_t ip_protocol, uint8_t payload_size);
void set_ip(ip_hdr_t *ip, const char *ip_dest, const char *ip_host,
            uint8_t ip_protocol, uint8_t payload_size);
void set_tcp(tcp_hdr_t *tcp, uint8_t flags);

static inline void free_packet(uint8_t *packet);
static inline gre_hdr_t *get_gre_hdr(uint8_t *packet_start);
static inline ip_hdr_t *get_ip_hdr(uint8_t *packet_start);
static inline tcp_hdr_t *get_tcp_hdr(uint8_t *packet_start);
static inline char *get_payload(uint8_t *packet_start);

void print_gre_header(uint8_t *packet);
void print_tcp_header(uint8_t *packet);
void print_ip_header(uint8_t *packet);
void print_payload(uint8_t *packet);
void print_packet(uint8_t *packet_start);

/* ===================================================================*/

#define PACKET_SIZE(header)                                                    \
  (uint8_t)(payload_size + sizeof(header) + sizeof(gre_hdr_t) +                \
            sizeof(ip_hdr_t));

/* @brief: Gets the size of the packet
 * @param: uint8_t - packet size
 * @param: uint8_t - IP protocol code
 * @param: uint8_t - payload size
 * @return: uint8_t - packet size (in bytes), -1 if error
 */
uint8_t get_packet_size(uint8_t ip_protocol, uint8_t payload_size) {
  if (ip_protocol == ip_protocol_tcp) {
    return PACKET_SIZE(tcp_hdr_t);
  } else {
    fprintf(stderr, "-- Invalid ip protocol code of %u --\n",
            (unsigned int)ip_protocol);
  }
  return -1;
}

static inline void free_packet(uint8_t *packet) { free(packet); }

/* ===================================================================*/
/* below are functions to set up each header                          */
/* ===================================================================*/

void set_ip(ip_hdr_t *ip, const char *ip_dest, const char *ip_host,
            uint8_t ip_protocol, uint8_t payload_size) {
  ip->ip_v = IP_VERSION;
  /* No need for ip_hl since the ip header size is fixed */
  /* No need for ip_tos since it's not used */
  ip->ip_len = htons(payload_size + sizeof(ip_hdr_t));
  /* No need for ip_id, ip_off since fragmentation not used */
  ip->ip_p = ip_protocol;
  ip->ip_src = parse_ip_addr(ip_host);
  ip->ip_dst = parse_ip_addr(ip_dest);
  ip->ip_sum = 0;
  ip->ip_sum = htons(cksum(ip, sizeof(ip_hdr_t)));
}

void set_tcp(tcp_hdr_t *tcp, uint8_t flags) {
  tcp->src_port = htons(PORT);
  tcp->dst_port = htons(PORT); /* PORTS might need to be changed later depending
                         on destination's c code */
  tcp->flags = flags;
  tcp->tcp_sum = htons(cksum(tcp, sizeof(tcp_hdr_t)));
}

/* @brief: Sets the payload msg
 * @param: const char * - payload msg
 * @return: uint8_t - packet size (in bytes), -1 if error
 */

/* ===================================================================*/
/* Below are functions to get the starting position of each header    */
/* ===================================================================*/

static inline gre_hdr_t *get_gre_hdr(uint8_t *packet_start) {
  return (gre_hdr_t *)(packet_start);
}

static inline ip_hdr_t *get_ip_hdr(uint8_t *packet_start) {
  return (ip_hdr_t *)(packet_start + sizeof(gre_hdr_t));
}

static inline tcp_hdr_t *get_tcp_hdr(uint8_t *packet_start) {
  return (tcp_hdr_t *)(packet_start + sizeof(gre_hdr_t) + sizeof(ip_hdr_t));
}

static inline char *get_payload(uint8_t *packet_start) {
  return (char *)(packet_start + sizeof(gre_hdr_t) + sizeof(ip_hdr_t) +
                  sizeof(tcp_hdr_t));
}

/* ===================================================================*/
/* Below are functions to create a packet                             */
/* ===================================================================*/

uint8_t *create_packets(const char *ip_src, const char *ip_dest,
                        uint8_t ip_protocol, const char *payload,
                        uint8_t flags) {
  /*
   * NOTE:
   * Create a new packet and set it up
   * The new packet should look like this:
   *  --------------------------------------------------------
   * |ethernet header (already done in socket API)|gre header
   * |ip header|tcp header|payload |
   * --------------------------------------------------------
   */
  int payload_size = MAX_PAYLOAD_SIZE;
  uint8_t packet_size = get_packet_size(ip_protocol, payload_size);
  uint8_t *new_packet = (uint8_t *)calloc(packet_size, sizeof(uint8_t));
  uint8_t *encrypt_packet = (uint8_t *)calloc(packet_size, sizeof(uint8_t));

  packet_encapsulate(new_packet);

  ip_hdr_t *new_ip = get_ip_hdr(new_packet);
  set_ip(new_ip, ip_dest, ip_src, ip_protocol, payload_size);

  /* ==== For now, just assume the new packet is TCP ==== */
  tcp_hdr_t *new_tcp = get_tcp_hdr(new_packet);
  set_tcp(new_tcp, flags);

  char *new_payload = get_payload(new_packet);
  int payload_length = strlen(payload);
  strncpy(new_payload, payload, payload_length);
  payload_length = (payload_length) > (MAX_PAYLOAD_SIZE - 1)
                       ? MAX_PAYLOAD_SIZE - 1
                       : payload_length;
  new_payload[payload_length] = '\0';

  /* Encrypt the payload */
  xor_encrypt_decrypt(new_packet, encrypt_packet, packet_size);
  char *encrypt_payload = get_payload(encrypt_packet);
  encrypt_payload[payload_length] = '\0';

  free(new_packet);
  return encrypt_packet;
}

uint8_t *create_packets_des(const char *ip_src, const char *ip_dest,
                            uint8_t ip_protocol, const char *payload,
                            uint8_t flags) {
  /*
   * NOTE:
   * Create a new packet and set it up
   * The new packet should look like this:
   *  --------------------------------------------------------
   * |ethernet header (already done in socket API)|gre header
   * |ip header|tcp header|payload |
   * --------------------------------------------------------
   */
  int payload_size = MAX_PAYLOAD_SIZE;
  uint8_t packet_size = get_packet_size(ip_protocol, payload_size);
  uint8_t *new_packet = (uint8_t *)calloc(packet_size, sizeof(uint8_t));

  packet_encapsulate(new_packet);

  ip_hdr_t *new_ip = get_ip_hdr(new_packet);
  set_ip(new_ip, ip_dest, ip_src, ip_protocol, payload_size);

  /* ==== For now, just assume the new packet is TCP ==== */
  tcp_hdr_t *new_tcp = get_tcp_hdr(new_packet);
  set_tcp(new_tcp, flags);

  char *new_payload = get_payload(new_packet);
  int payload_length = strlen(payload);
  strncpy(new_payload, payload, payload_length);
  payload_length = (payload_length) > (MAX_PAYLOAD_SIZE - 1)
                       ? MAX_PAYLOAD_SIZE - 1
                       : payload_length;
  new_payload[payload_length] = '\0';

  return new_packet;
}

int send_and_free_packet_vpn(int sockfd, uint8_t *packet_to_send,
                             uint8_t ip_protocol, uint8_t payload_size) {
  size_t pack_len = (size_t)get_packet_size(ip_protocol, payload_size);
  if (send(sockfd, packet_to_send, pack_len, 0) == -1) {
    return -1;
  }

  free_packet(packet_to_send);

  return 0;
}

int serv_cli_encrypt_free(int sockfd, uint8_t *packet_to_send,
                          uint8_t ip_protocol, uint8_t payload_size) {
  size_t pack_len = (size_t)get_packet_size(ip_protocol, payload_size);
  


  uint8_t *encrypt_packet = (uint8_t *)calloc(pack_len, sizeof(uint8_t));


  xor_encrypt_decrypt(packet_to_send, encrypt_packet, pack_len);
  char *encrypt_payload = get_payload(encrypt_packet);
  encrypt_payload[(int)payload_size] = '\0';


  if (send(sockfd, encrypt_packet, pack_len, 0) == -1) {
    
    return -1;
  }
  free_packet(packet_to_send);
  free_packet(encrypt_packet);

  return 0;
}

uint8_t *serv_rec_from_cli(int sockfd) {
  size_t pkt_size = sizeof(gre_hdr_t) + sizeof(ip_hdr_t) + sizeof(tcp_hdr_t) +
                    MAX_PAYLOAD_SIZE;
  uint8_t *new_rec_pkt = (uint8_t *)calloc(pkt_size, sizeof(uint8_t));
  if (recv(sockfd, new_rec_pkt, pkt_size, 0) == -1) {
    return NULL;
  }

  char *en_payload = get_payload(new_rec_pkt);
  int len = strlen(en_payload);

  /* ADD A POINTER POINTING TO PAYLOAD AND DECRYPT THE PAYLOAD HERE */
  uint8_t *decrypt_packet = (uint8_t *)calloc(pkt_size, sizeof(uint8_t));
  xor_encrypt_decrypt(new_rec_pkt, decrypt_packet, pkt_size);

  char *de_payload = get_payload(decrypt_packet);

  len = (len) > (MAX_PAYLOAD_SIZE - 1) ? MAX_PAYLOAD_SIZE - 1 : len;

  de_payload[len] = '\0';

  free(new_rec_pkt);
  return decrypt_packet;
}

uint8_t *serv_rec_from_des(int sockfd) {
  size_t pkt_size = sizeof(gre_hdr_t) + sizeof(ip_hdr_t) + sizeof(tcp_hdr_t) +
                    MAX_PAYLOAD_SIZE;
  uint8_t *new_rec_pkt = (uint8_t *)calloc(pkt_size, sizeof(uint8_t));
  if (recv(sockfd, new_rec_pkt, pkt_size, 0) == -1) {
    return NULL;
  }

  return new_rec_pkt;
}

uint8_t *dest_rec_pkt(int sockfd) {
  size_t pkt_size = sizeof(ip_hdr_t) + sizeof(tcp_hdr_t) + MAX_PAYLOAD_SIZE;
  uint8_t *new_rec_pkt = (uint8_t *)calloc(pkt_size, sizeof(uint8_t));
  if (recv(sockfd, new_rec_pkt, pkt_size, 0) == -1) {
    return NULL;
  }
  return new_rec_pkt;
}

uint8_t *serv_handle_pkt(uint8_t *packet, const char *server_ip) {
  uint8_t *fixed_pkt = packet_decapsulate(packet);
  ip_hdr_t *fixed_ip = get_ip_hdr(packet);

  uint16_t temp_len = ntohs(fixed_ip->ip_len);
  fixed_ip->ip_len = 0;
  fixed_ip->ip_len = temp_len;

  uint32_t temp_addr = parse_ip_addr(server_ip);
  fixed_ip->ip_src = 0;
  fixed_ip->ip_src = ntohl(temp_addr);

  temp_addr = 0;
  temp_addr = ntohl(fixed_ip->ip_dst);
  fixed_ip->ip_dst = 0;
  fixed_ip->ip_dst = temp_addr;

  fixed_ip->ip_sum = 0;
  fixed_ip->ip_sum = htons(cksum(fixed_ip, sizeof(ip_hdr_t)));

  temp_addr = 0;
  temp_addr = htonl(fixed_ip->ip_src);
  fixed_ip->ip_src = 0;
  fixed_ip->ip_src = temp_addr;

  temp_addr = 0;
  temp_addr = htonl(fixed_ip->ip_dst);
  fixed_ip->ip_dst = 0;
  fixed_ip->ip_dst = temp_addr;

  temp_len = 0;
  temp_len = htons(fixed_ip->ip_len);
  fixed_ip->ip_len = 0;
  fixed_ip->ip_len = temp_len;

  return fixed_pkt;
}

uint8_t *serv_handle_pkt_dest(uint8_t *packet, const char *server_ip) {
  uint8_t *fixed_pkt = packet_decapsulate(packet);
  ip_hdr_t *fixed_ip = get_ip_hdr(packet);

  uint16_t temp_len = ntohs(fixed_ip->ip_len);
  fixed_ip->ip_len = 0;
  fixed_ip->ip_len = temp_len;

  uint32_t temp_addr = parse_ip_addr(server_ip);
  fixed_ip->ip_dst = 0;
  fixed_ip->ip_dst = ntohl(temp_addr);

  temp_addr = 0;
  temp_addr = ntohl(fixed_ip->ip_src);
  fixed_ip->ip_src = 0;
  fixed_ip->ip_src = temp_addr;

  fixed_ip->ip_sum = 0;
  fixed_ip->ip_sum = htons(cksum(fixed_ip, sizeof(ip_hdr_t)));

  temp_addr = 0;
  temp_addr = htonl(fixed_ip->ip_dst);
  fixed_ip->ip_dst = 0;
  fixed_ip->ip_dst = temp_addr;

  temp_addr = 0;
  temp_addr = htonl(fixed_ip->ip_src);
  fixed_ip->ip_src = 0;
  fixed_ip->ip_src = temp_addr;

  temp_len = 0;
  temp_len = htons(fixed_ip->ip_len);
  fixed_ip->ip_len = 0;
  fixed_ip->ip_len = temp_len;

  return fixed_pkt;
}

void save_client_ip(uint32_t *client_ip, uint8_t *packet) {
  ip_hdr_t *ip_pointer = (ip_hdr_t *)(packet + sizeof(gre_hdr_t));
  memcpy(client_ip, &ip_pointer->ip_src, sizeof(uint32_t));
}

/* ===================================================================*/
/* Below are functions that prints packet infomation                  */
/* ===================================================================*/

void print_gre_header(uint8_t *packet) {
  gre_hdr_t *gre_header = get_gre_hdr(packet);
  printf("------------------------------------\n");
  printf("[ GRE header ]\n");

  printf("c:\t%d\n", gre_header->c);
  printf("protocol:\t%d\n", gre_header->protocol);
  printf("gre_sum:\t%d\n", gre_header->gre_sum);
}

void print_tcp_header(uint8_t *packet) {
  tcp_hdr_t *tcp_header = get_tcp_hdr(packet);
  printf("------------------------------------\n");
  printf("[ TCP header ]\n");

  printf("src_port: %d\n", tcp_header->src_port);
  printf("dst_port: %d\n", tcp_header->dst_port);
  printf("seq_num: %d\n", tcp_header->seq_num);
  printf("ack_num: %d\n", tcp_header->ack_num);
  printf("data_offset: %d\n", tcp_header->data_offset);
  printf("flags: %d\n", tcp_header->flags);
  printf("window: %d\n", tcp_header->window);
  printf("tcp_sum: %d\n", tcp_header->tcp_sum);
  printf("urgent_pointer: %d\n", tcp_header->urgent_pointer);
}

/**
 * @brief: Prints the IP header
 * @param: uint8_t * - packet
 */
void print_ip_header(uint8_t *packet) {
  ip_hdr_t *ip_header = get_ip_hdr(packet);
  char *ip_src_str = malloc(4 * 4 * sizeof(char));
  char *ip_dst_str = malloc(4 * 4 * sizeof(char));
  parse_ip_addr_to_str(ip_src_str, ip_header->ip_src);
  parse_ip_addr_to_str(ip_dst_str, ip_header->ip_dst);

  printf("------------------------------------\n");
  printf("[ IP header ]\n");

  printf("ip_tos: %d\n", ip_header->ip_tos);
  printf("ip_len: %d\n", ip_header->ip_len);
  printf("ip_id: %d\n", ip_header->ip_id);
  printf("ip_off: %d\n", ip_header->ip_off);
  printf("ip_ttl: %d\n", ip_header->ip_ttl);
  printf("ip_p: %d\n", ip_header->ip_p);
  printf("ip_sum: %d\n", ip_header->ip_sum);

  printf("ip_src: %s\n", ip_src_str);
  printf("ip_dst: %s\n", ip_dst_str);

  free(ip_src_str);
  free(ip_dst_str);
}

/**
 * @brief: Prints the payload
 * @param: uint8_t * - packet
 */
void print_payload(uint8_t *packet) {
  char *payload = get_payload(packet);
  printf("------------------------------------\n");
  printf("Payload msg: %s\n", payload);
}

/**
 * @brief: Prints the packet
 * @param: uint8_t * - packet
 */
void print_packet(uint8_t *packet_start) {
  print_gre_header(packet_start);
  print_ip_header(packet_start);
  print_tcp_header(packet_start);
  print_payload(packet_start);
}

void print_packet_unencap(uint8_t *packet_start) {
  /* IP header */
  ip_hdr_t *ip_header = (ip_hdr_t *)packet_start;
  char *ip_src_str = malloc(4 * 4 * sizeof(char));
  char *ip_dst_str = malloc(4 * 4 * sizeof(char));
  parse_ip_addr_to_str(ip_src_str, ip_header->ip_src);
  parse_ip_addr_to_str(ip_dst_str, ip_header->ip_dst);

  printf("------------------------------------\n");
  printf("[ IP header ]\n");

  printf("ip_tos: %d\n", ip_header->ip_tos);
  printf("ip_len: %d\n", ip_header->ip_len);
  printf("ip_id: %d\n", ip_header->ip_id);
  printf("ip_off: %d\n", ip_header->ip_off);
  printf("ip_ttl: %d\n", ip_header->ip_ttl);
  printf("ip_p: %d\n", ip_header->ip_p);
  printf("ip_sum: %d\n", ip_header->ip_sum);

  printf("ip_src: %s\n", ip_src_str);
  printf("ip_dst: %s\n", ip_dst_str);

  free(ip_src_str);
  free(ip_dst_str);

  /* TCP header */
  tcp_hdr_t *tcp_header = (tcp_hdr_t *)(packet_start + sizeof(ip_hdr_t));
  printf("------------------------------------\n");
  printf("[ TCP header ]\n");

  printf("src_port: %d\n", tcp_header->src_port);
  printf("dst_port: %d\n", tcp_header->dst_port);
  printf("seq_num: %d\n", tcp_header->seq_num);
  printf("ack_num: %d\n", tcp_header->ack_num);
  printf("data_offset: %d\n", tcp_header->data_offset);
  printf("flags: %d\n", tcp_header->flags);
  printf("window: %d\n", tcp_header->window);
  printf("tcp_sum: %d\n", tcp_header->tcp_sum);
  printf("urgent_pointer: %d\n", tcp_header->urgent_pointer);

  /* Payload */
  char *payload = (char *)(packet_start + sizeof(ip_hdr_t) + sizeof(tcp_hdr_t));
  printf("------------------------------------\n");
  printf("Payload msg: %s\n", payload);
}
