#include "helper.h"
#include "request.h"
#include "shared-memory-slot.h"

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
slot_t *shm_ptr;
int * temp;

int buf_size;
int thread_count;

// Lock for the work buffer
pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
int size = 0;
int head = 0;
int tail = 0;

void addtobuffer(int connfd, void *arg){
  int* work_buffer = temp;
  //  printf("in add buffer\n");
  work_buffer[0]=0;
 
  pthread_mutex_lock(&buffer_mutex);
  while(size == buf_size)
    pthread_cond_wait(&buf_not_full, &buffer_mutex);
  work_buffer[tail] = connfd;
  size=size+1;
  tail = (tail + 1) % buf_size;
  
  pthread_cond_signal(&buf_not_empty);
  pthread_mutex_unlock(&buffer_mutex);//change made here
  // printf("adding\n");
}

static void * 
worker_func(void *arg) {
  int* work_buffer = temp;
  while(1) {
    
    pthread_mutex_lock(&buffer_mutex);
    while(size == 0){
      pthread_cond_wait(&buf_not_empty, &buffer_mutex);
    }
    size=size-1;
    int connfd = work_buffer[head];
    head = (head + 1) % buf_size;
    
    pthread_cond_signal(&buf_not_full);
    pthread_mutex_unlock(&buffer_mutex);// change made here
    int request_type=requestHandle(connfd);
    int thread_index=-1;
    for(int i=0;i<thread_count;i++){
      if(shm_ptr[i].thread_id==pthread_self()){
        thread_index=i;
        break;
      }
    }
    if(thread_index!=-1){
      shm_ptr[thread_index].requests=shm_ptr[thread_index].requests+1;
      if(request_type==1){
        shm_ptr[thread_index].dynamic_requests=shm_ptr[thread_index].dynamic_requests+1;
      }
      else if(request_type==0){
        shm_ptr[thread_index].static_requests=shm_ptr[thread_index].static_requests+1;
      }

    }
    else{
      shm_ptr[thread_count].thread_id=pthread_self();
      shm_ptr[thread_count].requests=shm_ptr[thread_count].requests+1;
      if(request_type==1){
        shm_ptr[thread_count].dynamic_requests=shm_ptr[thread_count].dynamic_requests+1;
        }
      else if(request_type==0){
        shm_ptr[thread_count].static_requests=shm_ptr[thread_count].static_requests+1;
        }
      thread_count++;
    }
    Close(connfd);
  }
  return NULL;  
}


// CS537: Parse the new arguments too
void getargs(int *port, int* threads, int* buffers, char** shm_name, int argc, char *argv[])
{
  if (argc != 5) {
    fprintf(stderr, "Usage: %s <port> other stuff to\n", argv[0]);
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
  int pagesize = getpagesize();

  getargs(&port, &threads, &buffers, &shm_name, argc, argv);
  int shm_fd = shm_open(shm_name, O_RDWR | O_CREAT, 0660);
  if (shm_fd < 0) {
        perror("shm_open");
        exit(1);
    }
  int ret = ftruncate(shm_fd, pagesize);
    if(ret!=0){
    exit(1);
  }
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
  // if(threads==8){
  //   exit(1);
  // }
  // if(buffers==5){
  //   exit(1);
  // }
  int work_buffer[buf_size];
  // printf("insallah\n");
  // work_buffer[0]=0;
  temp=work_buffer;
  // printf("allahoakbar\n");
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

  shm_ptr= mmap(NULL, pagesize, PROT_WRITE, MAP_SHARED, shm_fd, 0);
     if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
  // fprintf(stdout,"made it here");
  listenfd = Open_listenfd(port);
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
    // fprintf(stdout,"made it here\n");
    addtobuffer(connfd, &work_buffer);
    // 
    // CS537 (Part A): In general, don't handle the request in the main thread.
    // Save the relevant info in a buffer and have one of the worker threads 
    // do the work. Also let the worker thread close the connection.
    // 
  }
  // for(int i = 0; i < threads; ++i) {
  //   pthread_join(workers[i], NULL);
  // }
}
