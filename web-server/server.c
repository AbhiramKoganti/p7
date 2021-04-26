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

// Condition variables for buffer
pthread_cond_t buf_not_full;
pthread_condattr_t buf_not_full_attr;
pthread_cond_t buf_not_empty;
pthread_condattr_t buf_not_empty_attr;

int buf_size;


// Lock for the work buffer
pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
int size = 0;

void addtobuffer(int connfd, void *arg){
  int* work_buffer = *(int**) arg;
  pthread_mutex_lock(&buffer_mutex);
  while(size == buf_size)
    pthread_cond_wait(&buf_not_full, &buffer_mutex);
  work_buffer[size++] = connfd;
  pthread_mutex_unlock(&buffer_mutex);
  pthread_cond_signal(&buf_not_empty);
}

static void * 
worker_func(void *arg) {
  int* work_buffer = *(int**) arg;
  while(1) {
    pthread_mutex_lock(&buffer_mutex);
    while(size == 0){
      pthread_cond_wait(&buf_not_empty, &buffer_mutex);
    }
    int connfd = work_buffer[--size];
    pthread_mutex_unlock(&buffer_mutex);
    pthread_cond_signal(&buf_not_full);

    requestHandle(connfd);
    Close(connfd);
  }
  return NULL;  
}


// CS537: Parse the new arguments too
void getargs(int *port, int* threads, int* buffers, char** shm_name, int argc, char *argv[])
{
  if (argc != 5) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    exit(1);
  }
  *port = atoi(argv[1]);
  *threads = atoi(argv[2]);
  *buffers = atoi(argv[3]);
  *shm_name = (char*) argv[4];
}


int main(int argc, char *argv[])
{
  int listenfd, connfd, port, threads, buffers, clientlen;
  char* shm_name;
  struct sockaddr_in clientaddr;
 
  getargs(&port, &threads, &buffers, &shm_name, argc, argv);
  buf_size = buffers;


  pthread_condattr_init(&buf_not_full_attr);
  pthread_cond_init(&buf_not_full, &buf_not_full_attr);
  pthread_condattr_init(&buf_not_empty_attr);
  pthread_cond_init(&buf_not_empty, &buf_not_empty_attr);
  
  if(buffers > MAXBUF)
    exit(1);
  if(buffers < 0)
    exit(1);
  if(threads < 0)
    exit(1);
  if(port < 0)
    exit(1);
  int work_buffer[buffers];
  //
  // CS537 (Part B): Create & initialize the shared memory region...
  //
  // 
  // CS537 (Part A): Create some threads...
  //
  pthread_t workers[threads];
  for (int i = 0; i < threads; ++i) {
    pthread_create(&workers[i], NULL, worker_func, &work_buffer);
  }


  listenfd = Open_listenfd(port);
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
    
    addtobuffer(connfd, &work_buffer);
    // 
    // CS537 (Part A): In general, don't handle the request in the main thread.
    // Save the relevant info in a buffer and have one of the worker threads 
    // do the work. Also let the worker thread close the connection.
    // 
  }
}
