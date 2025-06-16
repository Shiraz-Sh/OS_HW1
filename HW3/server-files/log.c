#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "request.h"
#include "segel.h"
#include "rw_lock.h"


// Opaque struct definition

struct Server_Log{
    // TODO: Implement internal log storage (e.g., dynamic buffer, linked list, etc.)
    struct Server_Log* next;
    char* data;
    int data_len;
};


/**
 *  Allocate and initialize internal log structure
 *  @return a pointer to the log
 */
server_log create_log(){
    readers_writers_init();
    server_log log = (server_log)malloc(sizeof(struct Server_Log));
    if (log != NULL)
        log->next = NULL;
    return log;
}

/**
 * Free all internal resources used by the log
 * @param log a pointer to the log
 */
void destroy_log(server_log log){
    reader_writer_destroy();

    server_log temp = log->next;
    server_log next = NULL;
    while (temp != NULL){
        next = temp->next;
        free(temp->data);
        free(temp);
        temp = next;
    }

    free(log);
}

/**
 * Return the full contents of the log as a dynamically allocated string
 * @param log a pointer to the log
 * @param dst destination buffer
 * @return length of data inserted to dst 
 */
int get_log(server_log log, char** dst){
    reader_lock();

    int total_log_len = 0;
    server_log temp = log->next;
    server_log next = NULL;

    while (temp != NULL){
        next = temp->next;
        total_log_len += temp->data_len;
        temp = next;
    }

    *dst = (char*)malloc(total_log_len + 1);
    if (*dst != NULL){
        temp = log->next;
        while (temp != NULL){
            strcat(*dst, temp->data);
        }
    }

    reader_unlock();

    return total_log_len;
}

/**
 *  Append the provided data to the log
 *  @param log a pointer to the log
 *  @param data the data to add in a new entry
 *  @param data_len the length of the new data
 */
void add_to_log(server_log log, const char* data, int data_len) {
    writer_lock();

    if (log->next == NULL){
        log->next = (server_log)malloc(sizeof(struct Sever_Log));
    }
    // if malloc failed
    if (log->next == NULL){
        writer_unlock();
        return;
    }
    server_log temp = log->next;
    while (temp->next != NULL){
        temp = temp->next;
    }
    temp->next = (server_log)malloc(sizeof(struct Sever_Log));
    if (temp->next != NULL){
        temp->next->data = (char*)malloc(data_len);
        if (temp->next->data != NULL){
            strcpy(temp->next->data, data);
        }
        temp->next->data_len = data_len;
        temp->next->next = NULL;
    }

    writer_unlock();
}
