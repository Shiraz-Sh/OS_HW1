#include <unistd.h> // for sbrk
#include <stddef.h> // for size_t
#include <bits/stdc++.h> // for memset

struct MallocMetadata{
    size_t size = 0;  // requested size + meta-data structure size
    size_t real_size = 0;
    bool is_free = true;
    void* data = nullptr;  // pointer to the first byte of our allocated data
    MallocMetadata* next;
    MallocMetadata* prev;
};

int main(){
    std::cout << sizeof(MallocMetadata) << std::endl;
}