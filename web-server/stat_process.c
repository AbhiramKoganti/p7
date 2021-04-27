#include "helper.h"
#include "shared-memory-slot.h"
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

int pagesize,shmfd;
slot_t *shm_ptr;
char* shm_name;
void intHandler(int dummy) {
    // TODO: Delete shared memory object here
    // fprintf(stdout,"lets go");
    int ret = munmap(shm_ptr, pagesize);
    if (ret != 0) {
        perror("munmap");
        exit(1);
    }

    // Delete the shared memory region.
    ret = shm_unlink(shm_name);
    if (ret != 0) {
        perror("shm_unlink");
        exit(1);
    }
  exit(0);
}

void getargs( char** shm_name,long * sleeptime_ms,int * num_threads, int argc, char *argv[])
{
  
  if (argc != 4) {
    fprintf(stderr, "Usage: is wrong");
    exit(1);
  }
  *shm_name = (char*) argv[1];
  *sleeptime_ms = atoi(argv[2]);
  *num_threads = atoi(argv[3]);
  // *buffers = atoi(argv[3]);
  
}



int main(int argc, char *argv[]){
  long sleeptime_ms;
  int num_threads;
  // struct timespec ts;
  
  pagesize = getpagesize();
  getargs( &shm_name,&sleeptime_ms,&num_threads, argc, argv);
    // Create the shared memory region.
  // /ts.tv_nsec=(sleeptime_ms)*100000;
  // ts.tv_sec=10;
  // fprintf(stdout,"here");
  shmfd = shm_open(shm_name, O_RDWR, S_IRUSR | S_IWUSR);
    if (shmfd < 0) {
        perror("shm_open");
        exit(1);
    }
  shm_ptr = mmap(NULL, pagesize, PROT_READ, MAP_SHARED, shmfd, 0);
   if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
  signal(SIGINT, intHandler);
  int count=1;	
  while(1){
    usleep(sleeptime_ms*1000);
    // /nanosleep(&ts,NULL);
    // fprintf(stdout,"%ld",sleeptime_ms);
    // 
    // sleep(1);
    // fprintf(stdout,"here\n");
    // break;
    for(int i=0;i<num_threads;i++){
      fprintf(stdout,"%d\n",count);
      fprintf(stdout,"%lu : %d %d %d\n",shm_ptr[i].thread_id,shm_ptr[i].requests,shm_ptr[i].static_requests,shm_ptr[i].dynamic_requests);

    }
    count++;
  }

  return 0;
}



