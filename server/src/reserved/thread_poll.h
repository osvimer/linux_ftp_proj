#ifndef __THTEAD_POLL__
#define __THTEAD_POLL__
#include "head.h"

typedef void * (* pthread_entry)(void *);
typedef struct fd_queue_node{
    int fd;
    struct fd_node_t * pnext;
}fd_node_t, * pfd_node_t;

typedef struct fd_queue{
    pfd_node_t queue_head;
    pfd_node_t queue_tail;
    pthread_mutex_t mutex;
    int size;
    int queue_capacity;
}fd_queue_t, * pfd_queue_t;

typedef struct thread_poll{
    pthread_t thid;
    pthread_cond_t cond;
    pthread_entry entry;
    fd_queue_t fd_queue;
    int start_status;
    int thread_num;
}factory_t, * pfactory_t;

void factory_init(pfactory_t, int, int, pthread_entry);
void factory_start(pfactory_t);
void * thread_handle(void *);

void factory_queue_init(pfd_queue_t, int);
void factory_queue_set(pfd_queue_t, pfd_node_t);
void factory_queque_get(pfd_queue_t, pfd_node_t *);
int factory_queue_empty(pfd_queue_t);
#endif
