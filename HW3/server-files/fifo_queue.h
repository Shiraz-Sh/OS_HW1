#ifndef FIFO_QUEUE_H
#define FIFO_QUEUE_H

#include "request.h"
#include "segel.h"
#include <pthread.h>

typedef struct{
    server_log* log;
    int connfd;
    struct timeval arrival, dispatch;
} request_val;


// Define Fifo queue and all auxiliary functions
typedef struct{
    request_val* queue;   // dynamically allocated array
    int head;
    int tail;
    int count;
    int max_size;
    pthread_mutex_t lock;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
} fifo_queue;


void fifo_init(fifo_queue* fifo, int size);

void fifo_destroy(fifo_queue* fifo);

// Add to queue (to tail), blocks if full
int fifo_enqueue(fifo_queue* fifo, request_val value);

// Remove from queue (from head), blocks if empty
int fifo_dequeue(fifo_queue* fifo, request_val* value);

#endif
