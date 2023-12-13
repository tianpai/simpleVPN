#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#define SERVER_PORT 5432
#define MAX_PENDING 5
#define MAX_LINE 256

int main()
{
  struct sockaddr_in sin;
  char buf[MAX_LINE];
  int len;
  int s, new_s;

  time_t current_time;        // time struct for the current time

  /* Set the received by server message */
  char received_message[28];  
  strncpy(received_message, "[from server] received at: ", 28);

  /* Set the end conversation command */
  char kill_command[6];
  strncpy(kill_command, "exit\n", 6);


  /* build address data structure */
  bzero((char *)&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(SERVER_PORT);
  /* setup passive open */
  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("simplex-talk: socket");
    exit(1);
  }
  if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0)
  {
    perror("simplex-talk: bind");
    exit(1);
  }
  listen(s, MAX_PENDING);

  /* wait for connection, then receive and print text */
  while (1)
  {
    if ((new_s = accept(s, (struct sockaddr *)&sin, &len)) < 0)
    {
      perror("simplex-talk: accept");
      exit(1);
    }

    while (len = recv(new_s, buf, sizeof(buf), 0))
    {
      time(&current_time);          // get and store the current time

      /* check if the client entered the kill command "exit"*/
      if(0 == strncmp(buf, kill_command, strlen(kill_command)))
      {
        break;
      }

      fputs(buf, stdout);           // print the client's message

      /* generate received message and send to client */
      strncat(buf, received_message, MAX_LINE);
      strncat(buf, ctime(&current_time), MAX_LINE); // add the formatted time
      
      send(new_s, buf, strlen(buf)+1, 0);  // send client the formatted message 
    }

    /* send connection terminated message to client */
    strncpy(buf, "[from server] connection terminated, server exited.\n", MAX_LINE);
    send(new_s, buf, strlen(buf)+1, 0);

    close(new_s);
    fputs("connection terminated.\n", stdout);
    return 0;
  }
}