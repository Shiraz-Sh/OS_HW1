#include "segel.h"
#include "request.h"
#include "log.h"

#include "fifo_queue.h"

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


// arguments the thread will use
typedef struct{
    fifo_queue *queue;
    pthread_mutex_t *requests_lock;
    pthread_cond_t *requests_remove_allowed;
    pthread_cond_t* requests_add_allowed;
    int thread_id;
} thread_args_struct;

/**
 * the function every thread will run on its own.
 * @param arg - the arguments (queue, mutex and cond)
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

    //TODO: when we will kill a thread?
    while (1){
        pthread_mutex_lock(args->requests_lock);
        
        while (args->queue->count == 0){
            pthread_cond_wait(args->requests_remove_allowed, args->requests_lock);
        }

        request_val request;
        int before_removing_count = args->queue->count;
        fifo_dequeue(args->queue, args->queue->max_size, &request);

        // signaling the main thread to allow more requests
        if (before_removing_count == args->queue->max_size) {
            pthread_cond_signal(args->requests_add_allowed);
        }
        pthread_mutex_unlock(args->requests_lock);

        // Call the request handler (immediate in main thread — DEMO ONLY)
        gettimeofday(&request.dispatch, NULL);
        requestHandle(request.connfd, request.arrival, request.dispatch, t_stats, *request.log);

        Close(request.connfd);  // Close the connection
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
    fifo_queue queue;                   // queue of the connections
    fifo_init(&queue, queue_size);

    // Initialize locks and conds
    pthread_mutex_init(&requests_lock, NULL);
    pthread_cond_init(&requests_remove_allowed, NULL);
    pthread_cond_init(&requests_add_allowed, NULL);

    // arguments the thread will use
    for (unsigned int i = 0; i < threads_size; i++){
        thread_args_struct thread_args = {
            .queue = &queue,
            .requests_lock = &requests_lock,
            .requests_remove_allowed = &requests_remove_allowed,
            .requests_add_allowed = &requests_add_allowed,
            .thread_id = i
        };
        pthread_create(&threads[i], NULL, thread_functon, &thread_args);
    }

    listenfd = Open_listenfd(port);
    while (1) {
        // wait if queue is full
        while (queue.count == queue.max_size)
            pthread_cond_wait(&requests_add_allowed , &requests_lock);

        // get new request
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

        pthread_mutex_lock(&requests_lock);

        fifo_enqueue(&queue, queue.max_size, new_request);      // adding to queue the new request
        pthread_cond_signal(&requests_remove_allowed);          // signaling all working threads that a new request came

        pthread_mutex_unlock(&requests_lock);
    }


    /*
    // ------------------------------------------------------------------------------
    // I am not sure if this is needed but might be useful later
    // ------------------------------------------------------------------------------
    // kill all threads using SIGKILL while making sure they are not in mid run using mutex
    pthread_mutex_lock(&requests_lock);
    for (unsigned int i = 0; i < threads_size; i++){
        pthread_kill(&threads[i], 9); // send SIGKILL to each thread 
    }
    */

    destroy_log(log);   // Clean up the server log before exiting
    free(queue.queue);  // free allocated place in queue

    // destroy locks and conds
    pthread_mutex_destroy(&requests_lock);
    pthread_cond_destroy(&requests_remove_allowed);
    pthread_cond_destroy(&requests_add_allowed);
}
