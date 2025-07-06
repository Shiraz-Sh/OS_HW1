#pragma once

#include <stddef.h> // for size_t


void* srealloc(void* oldp, size_t size);
void sfree(void* p);
void* scalloc(size_t num, size_t size);
void* smalloc(size_t size);

struct MallocMetadata{
    size_t size = 0;  // requested size + meta-data structure size
    size_t real_size = 0;
    bool is_free = true;
    void* data = nullptr;  // pointer to the first byte of our allocated data
    MallocMetadata* next = nullptr;
    MallocMetadata* prev = nullptr;
};