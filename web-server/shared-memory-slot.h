#ifndef _SHARED_MEMORY_SLOT_H_
#define _SHARED_MEMORY_SLOT_H_


typedef struct slot {
    pthread_t thread_id;
    int num;
    int requests;
    int dynamic_requests;
    int static_requests;
} slot_t;


#endif /* _SHARED_MEMORY_SLOT_H_ */