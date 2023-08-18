#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <iostream>
#include <queue>
#include  <sys/wait.h>

#include <netinet/in.h>
#include "http_server.hh"
#include <pthread.h>

#define THREAD 16

static int threadno, quedrop;
pthread_t thread[THREAD];
queue<int> clientFD;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty1 = PTHREAD_COND_INITIALIZER, full = PTHREAD_COND_INITIALIZER;
int flag;

void error(char *msg)
{
  perror(msg);
  // delete msg;
  exit(1);
}

void newSigInt(int x){
  flag=1;
  pthread_cond_broadcast(&full);
  // cout<<"aa gya andr"; 
  // fflush(stdout);
  for (int i = 0; i < THREAD; i++)
  {
    // pthread_cancel(thread[i]);
    pthread_join(thread[i], NULL);
    // cout<<"JudGaya"; fflush(stdout);
  }
	exit(0);
}

void *clientProcessing(void *arg)
{
  int threadID=++threadno;
  while (true)
  {
    pthread_mutex_lock(&lock);
    while (clientFD.empty()){
      if(flag==1){
        pthread_mutex_unlock(&lock);
        pthread_exit(NULL);
      }
      pthread_cond_wait(&full, &lock);
    }
    int newsockfd = clientFD.front();
    clientFD.pop();
    pthread_cond_signal(&empty1);
    pthread_mutex_unlock(&lock);
    // cout<<"Thread#"<<threadID<<endl;
    int n;
    char buffer[1024];
    bzero(buffer, 1023);
    n = read(newsockfd, buffer, 1023);
    if (n == 0)
    {
       close(newsockfd);
   	   continue; 
       //pthread_exit(NULL);
    }
    if (n < 0){
      perror("ERROR reading from socket");
      continue;
    }
    // printf("%s", buffer);

    /* send reply to client */
    HTTP_Response *x = handle_request(buffer);
    string y = x->get_string();
    // cout << y;
    n = write(newsockfd, y.c_str(), y.size());
    if (n < 0){
      perror("ERROR writing to socket");
      continue;
    }
    delete x;
    close(newsockfd);
  }
  return NULL;
}

int main(int argc, char *argv[])
{
  // signal(SIGINT, newSigInt);
  struct sigaction sa={0};
  sa.sa_handler = newSigInt;
  sa.sa_flags =SA_RESTART;
  sigaction(SIGINT, &sa, NULL);
  int sockfd, newsockfd, portno;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;
  int i,n;

  if (argc < 2)
  {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }

  /* create socket */

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  // printf("%d", sockfd);
  if (sockfd < 0)
    error("ERROR opening socket");

  /* fill in port number to listen on. IP address can be anything (INADDR_ANY)
   */

  bzero((char *)&serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  // delete argv;

  /* bind socket to this port number on this machine */

  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");

  for (i = 0; i < THREAD; i++)
  {
    pthread_create(&thread[i], NULL, clientProcessing, NULL);
  }

  /* listen for incoming connection requests */
  listen(sockfd, 256);

  while (true)
  {
    clilen = sizeof(cli_addr);
    /* accept a new request, create a newsockfd */
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
      error("ERROR on accept");
    pthread_mutex_lock(&lock);
    while(clientFD.size()>=512){
      pthread_cond_wait(&empty1, &lock);
    }
    clientFD.push(newsockfd);
    pthread_cond_signal(&full);
    pthread_mutex_unlock(&lock);
  }
  return 0;
}
