/* Created by Tianpai Zhang 2023-11-21
 * SIMPLE VPN CSCD58 final project
 */

/* NOTE:
 *  1. ensure that error handling is comprehensive and robust
 *  2. This file assumes the necessary cerfiticate and key files are set up
 *  3. SSL methods may have some compatibility issues
 *  4. Some more configuration may be required
 */

/* NOTE: For how to use SSL in client and server, please refer to the
 * documentation
 */

#include "encrypt.h"

/* key to encrypt the plaintext */
const uint8_t key[] = {0x12, 0x34, 0x56, 0x78};
size_t key_length = sizeof(key);

void xor_encrypt_decrypt(const uint8_t *input, uint8_t *output, size_t length) {
  for (size_t i = 0; i < length; ++i) {
    output[i] = input[i] ^ key[i % key_length];
  }
}
