/* Created by Tianpai Zhang Nov 13, 2023
* For CSCD58 final project
*/

#ifndef UTILS_H
#define UTILS_H

uint16_t cksum(const void *_data, int len);
int verify_checksum(const void *buf, int len, uint16_t old_cksum);

#endif  /* -- UTILS_H -- */
