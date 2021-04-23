#include "helper.h"
#include "request.h"

// 
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// CS537: Parse the new arguments too
void getargs(int *port, int argc, char *argv[])
{
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    exit(1);
  }
  *port = atoi(argv[1]);
}


int main(int argc, char *argv[])
{
  int listenfd, connfd, port, clientlen;
  struct sockaddr_in clientaddr;

  getargs(&port, argc, argv);
  


  //
  // CS537 (Part B): Create & initialize the shared memory region...
  //
  
  // 
  // CS537 (Part A): Create some threads...
  //
  
  pthread_t workers[THREAD_CNT];
  for (int i = 0; i < THREAD_CNT; ++i) {
    pthread_create(&workers[i], NULL, worker_func, NULL);
  }

  for (int i = 0; i < THREAD_CNT; ++i) {
    pthread_join(workers[i], NULL);
  }




  listenfd = Open_listenfd(port);
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

    // 
    // CS537 (Part A): In general, don't handle the request in the main thread.
    // Save the relevant info in a buffer and have one of the worker threads 
    // do the work. Also let the worker thread close the connection.
    // 
    requestHandle(connfd);
    Close(connfd);
  }
}
