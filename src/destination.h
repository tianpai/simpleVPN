#ifndef DESTINATION_H
#define DESTINATION_H

/*implement decap() the packet */
#include <stdint.h>

/* Following functions are used to create the destination socket as well
 * as connecting to listening to client connection
 */
int create_dest_socket();
int accept_dest_connection(int dest_fd);

#endif /* -- DESTINATION_H -- */