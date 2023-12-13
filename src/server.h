/* Created by In Kim Nov 14, 2023
 * CSCD58 final project
 * Simple VPN
 */

#ifndef SERVER_H
#define SERVER_H

/* Following functions are used to create the server socket as well
 * as accepting clients
 */
int create_server_socket();
int accept_client_connection(int server_fd);

#endif /* -- SERVER_H -- */