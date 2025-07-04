#include "fifo_queue.h"
#include <stdlib.h>
#include <pthread.h>

// initialize queue
void fifo_init(fifo_queue* fifo, int size){

    fifo->queue = malloc(size * sizeof(request_val));
    fifo->head = 0;
    fifo->tail = 0;
    fifo->count = 0;
    fifo->max_size = size;
    
    pthread_mutex_init(&fifo->lock, NULL);
    pthread_cond_init(&fifo->not_full, NULL);
    pthread_cond_init(&fifo->not_empty, NULL);
}

void fifo_destroy(fifo_queue* fifo){

    free(fifo->queue);

    pthread_mutex_destroy(&fifo->lock);
    pthread_cond_destroy(&fifo->not_full);
    pthread_cond_destroy(&fifo->not_empty);
}

// Add to queue (to tail), blocks if full
int fifo_enqueue(fifo_queue* fifo, request_val value){

    pthread_mutex_lock(&fifo->lock);
    while (fifo->count == fifo->max_size) {
        pthread_cond_wait(&fifo->not_full, &fifo->lock);
    }

    fifo->queue[fifo->tail] = value;
    fifo->tail = (fifo->tail + 1) % fifo->max_size;
    fifo->count++;

    pthread_cond_signal(&fifo->not_empty);
    pthread_mutex_unlock(&fifo->lock);

    return 0;
}

// Remove from queue (from head), blocks if empty
int fifo_dequeue(fifo_queue* fifo, request_val* value){

    pthread_mutex_lock(&fifo->lock);
    while (fifo->count == 0) {
        pthread_cond_wait(&fifo->not_empty, &fifo->lock);
    }

    *value = fifo->queue[fifo->head];
    fifo->head = (fifo->head + 1) % fifo->max_size;
    fifo->count--;

    pthread_cond_signal(&fifo->not_full);
    pthread_mutex_unlock(&fifo->lock);

    return 0;
}