#ifndef FIFO_QUEUE_H
#define FIFO_QUEUE_H

#include "request.h"
#include "segel.h"

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
} fifo_queue;


void fifo_init(fifo_queue* fifo, int size);

// Add to queue (to tail)
int fifo_enqueue(fifo_queue* fifo, int max_size, request_val value);

// Remove from queue (from head)
int fifo_dequeue(fifo_queue* fifo, int max_size, request_val* value);

#endif
