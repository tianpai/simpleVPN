#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>
#include <unistd.h>

uint8_t get_packet_size(uint8_t ip_protocol, uint8_t payload_size);

/* Takes in the ethernet and IP destination, the ethertype,
 * the IP protocol, the payload, its size, the flag which sees
 * if the function needs to be encapsulated and then creates
 * a packet with all of the necessary info.
 */
uint8_t *create_packets(const char *ip_src, const char *ip_dest,
                        uint8_t ip_protocol, const char *payload,
                        uint8_t flags);

uint8_t *create_packets_des(const char *ip_src, const char *ip_dest,
                            uint8_t ip_protocol, const char *payload,
                            uint8_t flags);
/* Takes in the socket FD, the pointer to packet, size of packet,
 * the protocol of IP, the payload size, and sends the packet through
 * the socket; packet freed after sending. Can be used for client or
 * server to send packet to the other side. Returns 0 on success, -1
 * if error.
 */
int send_and_free_packet_vpn(int sockfd, uint8_t *packet_to_send,
                             uint8_t ip_protocol, uint8_t payload_size);

/* Takes in the socket FD of client and receives the packet sent by
 * the client. Returns the pointer of the packet on success and NULL
 * if error occured.
 */
uint8_t *serv_rec_from_cli(int sockfd);

uint8_t *serv_rec_from_des(int sockfd);
/* Takes in the socket FD of destination and receives the packet that arrives.
 * Returns the pointer of the packet on success and NULL if error occured.
 */
uint8_t *dest_rec_pkt(int sockfd);

/* Takes in a packet pointer and the server name. Changes the ip_src
 * to the server's and returns the packet pointer decapsulated.
 */
uint8_t *serv_handle_pkt(uint8_t *packet, const char *server_name);

int serv_cli_encrypt_free(int sockfd, uint8_t *packet_to_send,
                          uint8_t ip_protocol, uint8_t payload_size);
/* Takes in a packet pointer and the server name. Changes the ip_dst
 * to the server's and returns the packet pointer decapsulated. Used
 * when server handles packets from destination.
 */
uint8_t *serv_handle_pkt_dest(uint8_t *packet, const char *server_name);

/* Takes in a packet while encapsulated and returns the client's
 * IP address.
 */
void save_client_ip(uint32_t *client_ip, uint8_t *packet);

/* Prints the contents of the packet including all of the info from
 * gre header, ip header, tcp header, and the payload message.
 */
void print_packet(uint8_t *packet);

/* Prints the contents of the unencapsulated packet including all of
 * the info from ip header, tcp header, and the payload message.
 */
void print_packet_unencap(uint8_t *packet);
#endif /* -- PACKET_H -- */
