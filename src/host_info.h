#ifndef HOST_INFO_H
#define HOST_INFO_H

#include "protocol.h"
#include <stdint.h>

#define H1_IP "10.0.0.1"
#define H2_IP "10.0.0.2"
#define H3_IP "10.0.0.3"
#define H4_IP "10.0.0.4"
#define H5_IP "10.0.0.5"
#define H6_IP "10.0.0.6"
#define H7_IP "10.0.0.7"

typedef struct {
  const char *name;
  const char *ip;
} HostInfo;

uint32_t parse_ip_addr(const char *ip);
void parse_ip_addr_to_str(char *str_ip_addr, uint32_t ip_addr);

/** @brief Get the network information of the interface
 *  @param iface: the name of the interface
 *  @param ipAddr: the IP address of the interface
 *
 *  @NOTE Read the IP address of the interface in the form of string
 * and assign it to ipAddr
 */
void read_ip_from_iface(const char *iface, char *ipAddr);

const char *get_host_ip(const char *host_name);

#endif // HOST_INFO_H
