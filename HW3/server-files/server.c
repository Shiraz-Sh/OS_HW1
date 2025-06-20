#include "segel.h"
#include "fifo_queue.h"
#include "request.h"
#include "log.h"

#include <stdio.h>


//
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// Parses command-line arguments
void getargs(int *port, int *threads_size, int *queue_size, int argc, char *argv[])
{
    if (argc < 4) {
        fprintf(stderr, "Usage: %s [portnum] [threads] [queue_size]\n", argv[0]);
        exit(1);
    }
    // TODO: check these are numbers
    *port = atoi(argv[1]);
    *threads_size = atoi(argv[2]);
    *queue_size = atoi(argv[3]);

    // check if sizes are positive
    if (*threads_size <= 0 || *queue_size <= 0) {
        fprintf(stderr, "args must be positive\n");
        exit(1);
    }
}


// arguments the thread will use
typedef struct{
    fifo_queue* queue;
    int thread_id;
} thread_args_struct;

/**
 * the function every thread will run on its own.
 * @param arg - the arguments (queue, thread_id)
 * @return
 */
void* thread_functon(void* arg) {
    thread_args_struct* args = (thread_args_struct*)arg;

    threads_stats t_stats = (threads_stats)malloc(sizeof(struct Threads_stats));
    t_stats->id = args->thread_id;
    t_stats->dynm_req = 0;
    t_stats->post_req = 0;
    t_stats->stat_req = 0;
    t_stats->total_req = 0;

    while (1){
        request_val request;
        fifo_dequeue(args->queue, &request);

        struct timeval current, dispatch;
        gettimeofday(&current, NULL);
        timersub(&current, &request.arrival, &dispatch);
        request.dispatch = dispatch;

        requestHandle(request.connfd, request.arrival, request.dispatch, t_stats, *request.log);
        
        Close(request.connfd);
        fifo_decrease_count(args->queue);
    }
    free(t_stats);
    free(args);
}


int main(int argc, char *argv[])
{
    // Create the global server log
    server_log log = create_log();

    int listenfd, connfd, port, clientlen, threads_size, queue_size;
    struct sockaddr_in clientaddr;

    getargs(&port, &threads_size, &queue_size, argc, argv);

    threads_size = (threads_size < queue_size) ? threads_size : queue_size; // threads cannot be more than queue size

    // Create the thread pool and request queue
    pthread_t threads[threads_size];
    fifo_queue queue;                   // queue of the connections
    fifo_init(&queue, queue_size, threads_size);


    // arguments the thread will use
    for (unsigned int i = 0; i < threads_size; i++){
        thread_args_struct* thread_args = (thread_args_struct*)malloc(sizeof(thread_args_struct));
        thread_args->queue = &queue;
        thread_args->thread_id = i + 1;
        pthread_create(&threads[i], NULL, thread_functon, thread_args);
    }

    listenfd = Open_listenfd(port);
    while (1){
        // Wait until there is space in the queue
        pthread_mutex_lock(&queue.lock);
        while (queue.queue_size + queue.active_count >= queue.max_size){
            pthread_cond_wait(&queue.not_full, &queue.lock);
        }
        pthread_mutex_unlock(&queue.lock);

        // Wait for a connection
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

        struct timeval arrival;
        gettimeofday(&arrival, NULL);

        request_val new_request = {
            .log = &log,
            .connfd = connfd,
            .arrival = arrival,
            .dispatch = {0, 0}
        };

        fifo_enqueue(&queue, new_request);      // thread-safe enqueue
    }

    destroy_log(log);   // Clean up the server log before exiting

    fifo_destroy(&queue);  // free allocated place in queue and destroy locks/conds
}
