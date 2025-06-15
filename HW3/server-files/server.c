#include "segel.h"
#include "request.h"
#include "log.h"

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
    // TODO: Can we get only "./server [port]"?
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    *port = atoi(argv[1]);
    *threads_size = atoi(argv[2]);
    *queue_size = atoi(argv[3]);

    // check if sizes are positive
    if (*threads_size <= 0 || *queue_size <= 0) {
        fprintf(stderr, "args must be positive\n", argv[0]);
        exit(1);
    }
}
// TODO: HW3 — Initialize thread pool and request queue
// This server currently handles all requests in the main thread.
// You must implement a thread pool (fixed number of worker threads)
// that process requests from a synchronized queue.

typedef struct {
    server_log* log;
    int connfd;
    threads_stats t;
    struct timeval arrival, dispatch;
} request_val;


// Define Fifo queue and all auxiliary functions
typedef struct {
    request_val *queue;   // dynamically allocated array
    int head;
    int tail;
    int count;
    int max_size;
} fifo_queue;

// initialize queue
void fifo_init(fifo_queue *fifo, int size) {
    fifo->queue = malloc(size * sizeof(request_val));
    fifo->head = 0;
    fifo->tail = 0;
    fifo->count = 0;
    fifo->max_size = size;
}

// Add to queue (to tail)
int fifo_enqueue(fifo_queue *fifo, int max_size, request_val value) {
    if (fifo->count == max_size)
        return -1; // queue full

    fifo->queue[fifo->tail] = value;
    fifo->tail = (fifo->tail + 1) % max_size;
    fifo->count++;
    return 0;
}

// Remove from queue (from head)
int fifo_dequeue(fifo_queue *fifo, int max_size, request_val *value) {
    if (fifo->count == 0)
        return -1; // queue empty

    *value = fifo->queue[fifo->head];
    fifo->head = (fifo->head + 1) % max_size;
    fifo->count--;
    return 0;
}



// arguments the thread will use
typedef struct {
    fifo_queue *queue;
    pthread_mutex_t *requests_lock;
    pthread_cond_t *requests_remove_allowed;
    pthread_cond_t *requests_add_allowed;
} thread_args_struct;

/**
 * the function every thread will run on its own.
 * @param arg - the arguments (queue, mutex and cond)
 * @return
 */
void* thread_functon(void* arg) {
    thread_args_struct *args = (thread_args_struct *)arg;

    while (1) { //TODO: when we will kill a thread?
        pthread_mutex_lock(args->requests_lock);
        while (args->queue->count == 0) {
            pthread_cond_wait(args->requests_remove_allowed, args->requests_lock);
        }
        //remove
        request_val request;
        int before_removing_count = args->queue->count;
        fifo_dequeue(args->queue, args->queue->max_size, &request);

        // signaling the main thread to allow more requests and updating count
        if (before_removing_count == args->queue->max_size) {
            pthread_cond_signal(args->requests_add_allowed);
        }
        pthread_mutex_unlock(args->requests_lock);

        request.t->id = (int)pthread_self(); // getting ID of current thread

        // Call the request handler (immediate in main thread — DEMO ONLY)
        gettimeofday(&request.dispatch, NULL);
        requestHandle(request.connfd, request.arrival, request.dispatch, request.t, *request.log);

        free(request.t); // Cleanup
        Close(request.connfd); // Close the connection
    }
}


int main(int argc, char *argv[])
{
    // Create the global server log
    server_log log = create_log();

    int listenfd, connfd, port, clientlen, threads_size, queue_size, total_requests;
    struct sockaddr_in clientaddr;
    pthread_cond_t requests_remove_allowed;
    pthread_cond_t requests_add_allowed;
    pthread_mutex_t requests_lock;

    getargs(&port, &threads_size, &queue_size, argc, argv);

    // Create the thread pool and request queue
    pthread_t threads[threads_size];
    fifo_queue queue; // queue of the connections
    fifo_init(&queue, queue_size);

    // Initialize locks and conds
    pthread_mutex_init(&requests_lock, NULL);
    pthread_cond_init(&requests_remove_allowed, NULL);
    pthread_cond_init(&requests_add_allowed, NULL);

    // arguments the thread will use
    thread_args_struct thread_args = {
        .queue = &queue,
        .requests_lock = &requests_lock,
        .requests_remove_allowed = &requests_remove_allowed,
        .requests_add_allowed = &requests_add_allowed
    };

    for (unsigned int i = 0; i < threads_size; i++)
        pthread_create(&threads[i], NULL, thread_functon, &thread_args);

    listenfd = Open_listenfd(port);
    total_requests = 0;
    while (1) {
        // wait if queue is full
        while (queue.count == queue.max_size) {
            pthread_cond_wait(&requests_add_allowed , &requests_lock);
        }

        // get new request
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

        total_requests += 1;

        // malloc place for stats
        threads_stats t = malloc(sizeof(struct Threads_stats));
        t->id = 0;             // Thread ID (placeholder)
        t->stat_req = 0;       // Static request count //TODO: static and dynamic request count
        t->dynm_req = 0;       // Dynamic request count
        t->total_req = total_requests;      // Total request count

        struct timeval arrival;
        gettimeofday(&arrival, NULL);

        request_val new_request = {
            .log = &log,
            .connfd = connfd,
            .t = t,
            .arrival = arrival,
            .dispatch = {0, 0}
        };

        pthread_mutex_lock(&requests_lock);
        // adding to queue the nw request
        fifo_enqueue(&queue, queue.max_size, new_request);

        // signaling all working threads that a new request came
        pthread_cond_signal(&requests_remove_allowed);

        pthread_mutex_unlock(&requests_lock);
    }

    // Clean up the server log before exiting
    destroy_log(log);

    // TODO: HW3 — Add cleanup code for thread pool and queue
    free(queue.queue);  // free allocated place in queue
    // destroy locks and conds
    pthread_mutex_destroy(&requests_lock);
    pthread_cond_destroy(&requests_remove_allowed);
    pthread_cond_destroy(&requests_add_allowed);
}
