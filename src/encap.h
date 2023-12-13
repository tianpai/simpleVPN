/* Created by In Kim Nov 23, 2023
 * CSCD58 final project
 * Simple VPN
 */

#ifndef ENCAP_H
#define ENCAP_H

#include "protocol.h"

/* Params: Takes in a memory address of ip_packet pointer,
 * encapsulates it, and returns the same memory address with
 * the info filled.
 */
void packet_encapsulate(uint8_t *packet);

/* Takes in a network interface name and finds its corresp.
 * MAC address. If this doesn't work for Mininet, just hardcode
 * the client MAC and IP address.
 */
char *get_mac_address(const char *interface);

/* Takes in a newwork interface name and finds its corresp.
 * IP address. If this doesn't work for Mininet, just hardcode
 * the client MAC and IP address.
 */
char *get_ip_address(const char *interface);

#endif /* -- ENCAPSULATION_H -- */
