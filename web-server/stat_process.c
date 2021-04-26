#include "helper.h"


void intHandler(int dummy) {
    // TODO: Delete shared memory object here
  exit(0);
}


int main(){
  signal(SIGINT, intHandler);	
  while(1){}

  return 0;
}



