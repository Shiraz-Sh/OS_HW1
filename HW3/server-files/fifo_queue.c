#include "fifo_queue.h"

// initialize queue
void fifo_init(fifo_queue* fifo, int size){
    fifo->queue = malloc(size * sizeof(request_val));
    fifo->head = 0;
    fifo->tail = 0;
    fifo->count = 0;
    fifo->max_size = size;
}

// Add to queue (to tail)
int fifo_enqueue(fifo_queue* fifo, int max_size, request_val value){
    if (fifo->count == max_size)
        return -1; // queue full

    fifo->queue[fifo->tail] = value;
    fifo->tail = (fifo->tail + 1) % max_size;
    fifo->count++;
    return 0;
}

// Remove from queue (from head)
int fifo_dequeue(fifo_queue* fifo, int max_size, request_val* value){
    if (fifo->count == 0)
        return -1; // queue empty

    *value = fifo->queue[fifo->head];
    fifo->head = (fifo->head + 1) % max_size;
    fifo->count--;
    return 0;
}