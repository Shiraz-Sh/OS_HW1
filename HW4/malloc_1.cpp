
#include <unistd.h> // for sbrk
#include <stddef.h> // for size_t

void* smalloc(size_t size) {
    if (size == 0 || size > 100000000) { // if equal to 0 or larger then 10^8
        return NULL;
    }

    void* result = sbrk(size);
    if (result == (void*)-1) { // sbrk failed
        return NULL;
    }

    return result;
}