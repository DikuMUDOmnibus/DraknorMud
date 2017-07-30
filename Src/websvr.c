/* ROM 2.4 Integrated Web Server - Version 1.0
 *
 * This is my first major snippet... Please be kind. ;-)
 * Copyright 1998 -- Defiant -- Rob Siemborski -- mud@towers.crusoe.net
 *
 * Many thanks to Russ and the rest of the developers of ROM for creating
 * such an excellent codebase to program on.
 *
 * If you use this code on your mud, I simply ask that you place my name
 * someplace in the credits.  You can put it where you feel it is
 * appropriate.
 *
 * I offer no guarantee that this will work on any mud except my own, and
 * if you can't get it to work, please don't bother me.  I wrote and tested
 * this only on a Linux 2.0.30 system.  Comments about bugs, are, however,
 * appreciated.
 *
 */

/* Go to http://www.landsofdraknor.net:7001/wholist */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/time.h>
#include "merc.h"

#define MAXDATA 1024
char *sreplace(char *str, char *orig, char *rep);

typedef struct web_descriptor WEB_DESCRIPTOR;

struct web_descriptor {
  int fd;
  char request[MAXDATA*2];
  struct sockaddr_in their_addr;
  unsigned int sin_size;
  WEB_DESCRIPTOR *next;
  bool valid;
};

WEB_DESCRIPTOR *web_desc_free;

int send_color_buf(int fd, const char* buf);
int send_buf(int fd, const char* buf);
void handle_web_request(WEB_DESCRIPTOR *wdesc);
void handle_web_who_request(WEB_DESCRIPTOR *wdesc);
WEB_DESCRIPTOR *new_web_desc(void);
void free_web_desc(WEB_DESCRIPTOR *desc);

const char ENDREQUEST[5] = { 13, 10, 13, 10, 0 }; /* (CRLFCRLF) */
int top_web_desc;
WEB_DESCRIPTOR *web_descs;
int sockfd;

void init_web(int port) {
  struct sockaddr_in my_addr;
  char log_buf[MSL];

  web_descs = NULL;

  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("web-socket");
    return;
  }

  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(port);
  my_addr.sin_addr.s_addr = htons(INADDR_ANY);
  bzero(&(my_addr.sin_zero),8);

  if((bind(sockfd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr))) == -1)
  {
    perror("web-bind");
    return;
  }

  /* Only listen for 5 connects at once, do we really need more? */
  listen(sockfd, 5);

  mprintf(sizeof(log_buf),log_buf, "--Draknor webserver been started on port %d",port );
  log_string( log_buf );
}

struct timeval ZERO_TIME = { 0, 0 };

void handle_web(void) {
  int max_fd;
  WEB_DESCRIPTOR *current, *prev, *next;
  fd_set readfds;

  FD_ZERO(&readfds);
  FD_SET(sockfd, &readfds);

  /* it *will* be atleast sockfd */
  max_fd = sockfd;

  /* add in all the current web descriptors */
  for(current=web_descs; current != NULL; current = current->next) {
      FD_SET(current->fd, &readfds);
      if(max_fd < current->fd)
    max_fd = current->fd;
  }

  /* Wait for ONE descriptor to have activity */
  select(max_fd+1, &readfds, NULL, NULL, &ZERO_TIME);

  if(FD_ISSET(sockfd, &readfds)) {
            /* NEW CONNECTION -- INIT & ADD TO LIST */

      current = new_web_desc();
      current->sin_size  = sizeof(struct sockaddr_in);
      current->request[0] = '\0';

      if((current->fd = accept(sockfd, (struct sockaddr *)&(current->their_addr), &(current->sin_size))) == -1) {
        perror("web-accept");
        return;
      }

      current->next = web_descs;
      web_descs = current;

      /* END ADDING NEW DESC */
  }

  /* DATA IN! */
  for(current=web_descs; current != NULL; current = current->next) {
      if (FD_ISSET(current->fd, &readfds)) /* We Got Data! */
      {
        char buf[MAXDATA];
    int numbytes;

    if((numbytes=read(current->fd,buf,sizeof(buf))) == -1) {
        perror("web-read");
        return;
    }

    buf[numbytes] = '\0';

    strcat(current->request,buf);
      }
  } /* DONE WITH DATA IN */

  /* DATA OUT */
  for(current=web_descs; current != NULL; current = next ){
      next = current->next;

      if(strstr(current->request, "HTTP/1.") /* 1.x request (vernum on FIRST LINE) */
      && strstr(current->request, ENDREQUEST))
    handle_web_request(current);
      else if(!strstr(current->request, "HTTP/1.")
     &&  strchr(current->request, '\n')) /* HTTP/0.9 (no ver number) */
    handle_web_request(current);
      else {
    continue; /* Don't have full request yet! */
      }

      close(curr