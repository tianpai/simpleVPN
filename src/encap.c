/* Created by In Kim Nov 23, 2023
 * CSCD58 final project
 * Simple VPN
 */ /* Importing the libraries needed */
#include <linux/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "encap.h"
#include "protocol.h"
#include "utils.h"

void packet_encapsulate(uint8_t *packet) {
  /* Type casting to access GRE header */
  gre_hdr_t *enc_gre_hdr = (gre_hdr_t *)(packet);

  /* Assigning values to GRE */
  enc_gre_hdr->c = htons(1);
  /* Assuming we only use IPv4 */
  enc_gre_hdr->protocol = htons(ethertype_ipv4);
  enc_gre_hdr->gre_sum = 0;
  enc_gre_hdr->gre_sum = htons(cksum(enc_gre_hdr, sizeof(struct gre_hdr)));
}