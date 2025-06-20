#include <stdlib.h>
#include <string.h>

#include "log.h"
// #include "request.h"
#include "segel.h"
#include "rw_lock.h"
#include "utils.h"

// Opaque struct definition


/**
 *  Allocate and initialize internal log structure
 *  @return a pointer to the log
 */
server_log create_log(){
    readers_writers_init();
    server_log log = (server_log)malloc(sizeof(struct Server_Log));
    if (log == NULL){
        MALLOC_FAIL(sizeof(struct Server_Log));
        exit(1);
    }

    log->head = (log_entry)malloc(sizeof(struct Log_Entry));
    if (log->head == NULL){
        MALLOC_FAIL(sizeof(struct Log_Entry));
        free(log);
        exit(1);
    }

    log->head->next = NULL;
    log->head->empty = true;
    return log;
}

/**
 * Free all internal resources used by the log
 * @param log a pointer to the log
 */
void destroy_log(server_log log){
    reader_writer_destroy();

    log_entry temp = log->head;
    log_entry next = NULL;
    while (temp != NULL){
        next = temp->next;
        if (!temp->empty){
            free(temp->data);
        }
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
    DEBUG_PRINT("got reader lock");
    DEBUG_PRINT("log null ? (%d)", log == NULL);

    int total_log_len = 0;
    log_entry temp = log->head;

    int i = 0;
    while (temp != NULL && !temp->empty){
        i++;
        total_log_len += temp->data_len;
        temp = temp->next;
    }

    DEBUG_PRINT("log size =  %d", total_log_len);

    *dst = (char*)malloc(total_log_len + 1 + i);
    if (*dst != NULL){
        (*dst)[0] = '\0';
        DEBUG_PRINT("malloc successfull");

        temp = log->head;
        while (temp != NULL && !temp->empty){
            DEBUG_PRINT("read log");
            if (temp->empty){
                DEBUG_PRINT("\n\n>>>>>>> very very bad! <<<<<<<\n\n");
            }
            else{
                strcat(*dst, temp->data);
            }
            strcat(*dst, "\n");
            DEBUG_PRINT("strcat success");
            temp = temp->next;
        }
    }
    else{
        MALLOC_FAIL((size_t)total_log_len + 1 + i);
        reader_unlock();
        exit(1);
    }

    reader_unlock();
    DEBUG_PRINT("got log");
    return total_log_len + i;
}

/**
 *  Append the provided data to the log
 *  @param log a pointer to the log
 *  @param data the data to add in a new entry
 *  @param data_len the length of the new data
 */
void add_to_log(server_log log, const char* data, int data_len){
    DEBUG_PRINT("try writer lock");
    writer_lock();
    DEBUG_PRINT("got writer lock");

    log_entry temp = log->head;
    while (temp->next != NULL){
        temp = temp->next;
    }

    usleep(200000); // set this to 0.2 seconds (in microseconds)

    // instantiate an empty cell at the end
    temp->next = (log_entry)malloc(sizeof(struct Log_Entry));
    if (temp->next == NULL){
        MALLOC_FAIL(sizeof(struct Log_Entry));
        writer_unlock();
        exit(1);
    }

    temp->next->next = NULL;
    temp->next->empty = true;

    temp->data = (char*)malloc(data_len + 1);
    if (temp->data != NULL){
        strcpy(temp->data, data);
    }
    else{
        MALLOC_FAIL((size_t)data_len + 1);
        writer_unlock();
        exit(1);
    }
    temp->data_len = data_len;
    temp->empty = false;

    DEBUG_PRINT("added to log");

    writer_unlock();
    DEBUG_PRINT("writer unlocked");
}
