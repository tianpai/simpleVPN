/* Created by Tianpai Zhang Nov 13, 2023
 * For CSCD58 final project
 */

#ifndef ENCRYPT_H
#define ENCRYPT_H

/* Ubuntu
 * 'sudo apt-get install libssl-dev'
 *
 * Fedora
 *  sudo dnf install openssl-devel
 * */
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>

void init_ssl_lib();    /* initialize the SSL library */
void cleanup_ssl_lib(); /* cleanup the SSL library */

/* create a new SSL context */
SSL_CTX *create_ssl_server_context();
SSL_CTX *create_ssl_client_context();

/* configure the SSL context */
void configure_context(SSL_CTX *ctx, char *cert_path, char *key_file);

/* create a new SSL connection */
SSL *ssl_connect(int sockfd, SSL_CTX *ctx);
int ssl_accept_connection(SSL_CTX *ssl); /* handle individual connection */

void ssl_shutdown(SSL *ssl);

#endif /* -- ENCRYPT_H -- */
