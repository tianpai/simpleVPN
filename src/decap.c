/* decap (decapsulation)
 *  decapsulates packets whereelse it is needed
 */
 
#include <stdint.h>

#include "decap.h"
#include "protocol.h"


uint8_t *packet_decapsulate(uint8_t *packet) {
	return (uint8_t *)(packet + sizeof(gre_hdr_t));
}

/* Refer to decap.h for function details */
