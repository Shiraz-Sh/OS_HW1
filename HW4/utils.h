#ifndef UTILS_H
#define UTILS_H

#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) \
    fprintf(stderr, "[DEBUG] %s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) // no-op

#endif

#define MALLOC_FAIL(size) fprintf(stderr, "error: malloc failed to allocate size %ld", size)

#endif 