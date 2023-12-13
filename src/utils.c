/* Created by Tianpai Zhang Nov 13, 2023
* CSCD58 final project
* Simple VPN
*/


/* 
 * not sure if there are extra headers needed
 * or fewer headers needed
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "utils.h"


/* @ brief: checksum function 
 * @ param: buf, len 
 * @ return: checksum value
 */
uint16_t cksum(const void *_data, int len) {
  const uint8_t *data = _data;
  uint32_t sum;

  for (sum = 0;len >= 2; data += 2, len -= 2)
    sum += data[0] << 8 | data[1];
  if (len > 0)
    sum += data[0] << 8;
  while (sum > 0xffff)
    sum = (sum >> 16) + (sum & 0xffff);
  sum = htons (~sum);
  return sum ? sum : 0xffff;
}


/* @ brief: verify checksum function 
 * @ param: buf, len, old_sum 
 * @ return: 0 if checksum is correct, -1 otherwise
 */
int verify_checksum(const void *buf, int len, uint16_t old_cksum)
{
  return cksum(buf, len) == old_cksum ? 0 : -1;
}

/* end of utils.c */
