#ifndef DECAP_H
#define DECAP_H

/*implement decap() the packet */
#include <stdint.h>

/* Takes in a packet pointer and moves the pointer so that
 * it doesn't point to the GRE header and points to IP header,
 * therefore decapsulating it.
 */
uint8_t *packet_decapsulate(uint8_t *packet);

#endif /* -- DECAP_H -- */
