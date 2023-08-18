/* run using ./server <port> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <iostream>
#include <queue>

#include <netinet/in.h>
#include "http_server.hh"
#include <pthread.h>

queue<int> clientFD;

void error(char *msg) {
  perror(msg);
  exit(1);
}

void *clientProcessing(void *arg) {
  int newsockfd = *((int *) arg);
  int n;
  char buffer[1024];
  /* read message from client */
  // while(true){ 
    bzero(buffer, 1023);
    n = read(newsockfd, buffer, 1023);
    if(n==0){
      close(newsockfd);
      pthread_exit(NULL);
    }
    if (n < 0)
      error("ERROR reading from socket");
    printf("%s", buffer);

    /* send reply to client */
    HTTP_Response *x=handle_request(buffer);
    string y = x->get_string();
    cout<<y;
    n = write(newsockfd, y.c_str(), y.size());
    if (n < 0)
      error("ERROR writing to socket");
  // }
  close(newsockfd);
  return NULL;
}

int main(int argc, char *argv[]) {
  int sockfd, newsockfd, portno;
  socklen_t clilen;
  char buffer[256];
  struct sockaddr_in serv_addr, cli_addr;
  int n;
  


  if (argc < 2) {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }

  /* create socket */

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  printf("%d",sockfd);
  if (sockfd < 0)
    error("ERROR opening socket");

  /* fill in port number to listen on. IP address can be anything (INADDR_ANY)
   */

  bzero((char *)&serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  /* bind socket to this port number on this machine */

  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");

  /* listen for incoming connection requests */

  listen(sockfd, 5);
  //while(true){
    clilen = sizeof(cli_addr);

    /* accept a new request, create a newsockfd */
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    printf("%d",newsockfd);
    clientFD.push(newsockfd);
    if (newsockfd < 0)
      error("ERROR on accept");
    pthread_t p1;
    pthread_create(&p1, NULL, &clientProcessing, &newsockfd);
    //pthread_join(p1, NULL);
    //close(newsockfd); 
  // }
  return 0;
}
