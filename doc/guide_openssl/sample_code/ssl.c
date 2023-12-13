#include "ssl.h"
#include <openssl/err.h>
#include <openssl/ssl.h>

void simple_ssl_function() {
  SSL_load_error_strings();
  SSL_library_init();
  printf("Initialized SSL\n");

  // Your SSL code here

  // Clean up
  ERR_free_strings();
}
