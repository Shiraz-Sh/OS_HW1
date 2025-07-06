#include <unistd.h> // for sbrk
#include <stddef.h> // for size_t
#include <bits/stdc++.h> // for memset
#include <sys/mman.h> // for mmap, munmap
#include <cassert>
#include <iostream>

#include "header.h" // TODO: remove this before submit!!!!!!!


#define MAX_ORDER 10
#define BYTE_OFFSET(ptr, offset) ((void*)((char*)(ptr) + (offset)))

const uintptr_t KB = 1 << 10;
const uintptr_t MB = 1 << 20;

#define DEBUG
#ifdef DEBUG
#define DEBUG_ASSERT(test) {assert(test); std::cout << "passed: [ " << #test << " ]" << std::endl;} 
#define DEBUG_PRINT(fmt, ...) \
    fprintf(stderr, "[DEBUG] %s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) // no-op
#define DEBUG_ASSERT(test) {} 

#endif

// TODO: uncomment this before submit
// struct MallocMetadata{
//     size_t size = 0;  // requested size + meta-data structure size
//     size_t real_size = 0;
//     bool is_free = true;
//     void* data = nullptr;  // pointer to the first byte of our allocated data
//     MallocMetadata* next = nullptr;
//     MallocMetadata* prev = nullptr;
// };

struct DataList{
    MallocMetadata* first_data_list = nullptr;
    MallocMetadata* last_data_list = nullptr;
    size_t num_blocks = 0;
    size_t num_free_blocks = 0;
};

struct Orders_mapping{
    enum Orders{
        ORD_0 = 0,
        ORD_1, ORD_2, ORD_3, ORD_4,
        ORD_5, ORD_6, ORD_7, ORD_8,
        ORD_9, ORD_10,
        MEGA = -1
    };

    static const size_t orders[MAX_ORDER + 1];

    // given a size return of which order is it.
    static int size_to_order(size_t size, bool is_real_size){
        if (!is_real_size)
            size += sizeof(MallocMetadata);
        for (int i = 0; i <= MAX_ORDER; i++)
            if (size <= Orders_mapping::orders[i])
                return i;
        return MEGA;
    }

    static size_t order_to_size(int order){
        if (order == MEGA){
            return -1;
        }
        return 1 << (7 + order);
    }
};

const size_t Orders_mapping::orders[MAX_ORDER + 1] = {
    1 << 7,
    1 << 8,
    1 << 9,
    1 << 10,
    1 << 11,
    1 << 12,
    1 << 13,
    1 << 14,
    1 << 15,
    1 << 16,
    1 << 17
};

struct Handle{
    bool init = false;
    DataList tbl[MAX_ORDER + 1];
    DataList mmaped;
} handler;


void _init_handler();
void _init_block(MallocMetadata* block, DataList* datalist, size_t size, bool is_real_size);
void _remove_block(MallocMetadata* block, DataList* datalist);
MallocMetadata* _merge_blocks(MallocMetadata* block, DataList* org_datalist, DataList* dst_datalist, int order, size_t request_size);
bool _check_merge_blocks(MallocMetadata* block, int request_size, int order);
MallocMetadata* _split_block(MallocMetadata* block, DataList* org_datalist, DataList* dst_datalist, size_t size);
void _populate_block(MallocMetadata* block, DataList* datalist, size_t size);
MallocMetadata* find_empty_block(DataList* list);

size_t _num_free_blocks();
size_t _num_free_bytes();
size_t _num_allocated_blocks();
size_t _num_allocated_bytes();
size_t _num_meta_data_bytes();
size_t _size_meta_data();

size_t _num_free_blocks(){
    DEBUG_PRINT();
    size_t free_blocks = 0;
    for (int i = 0; i <= MAX_ORDER; i++){
        free_blocks += handler.tbl[i].num_free_blocks;
    }

    free_blocks += handler.mmaped.num_free_blocks;
    return free_blocks;
}

size_t _num_free_bytes(){
    DEBUG_PRINT();
    int free_bytes = 0;
    for (int i = 0; i <= MAX_ORDER; i++){
        MallocMetadata* curr = handler.tbl[i].first_data_list;
        while (curr != nullptr){
            if (curr->is_free){
                free_bytes += curr->size;
            }
            curr = curr->next;
        }
    }

    MallocMetadata* curr = handler.mmaped.first_data_list;
    while (curr != nullptr){
        if (curr->is_free){
            free_bytes += curr->size;
        }
        curr = curr->next;
    }
    return free_bytes;
}

size_t _num_allocated_blocks(){
    int free_blocks = 0;
    for (int i = 0; i <= MAX_ORDER; i++){
        free_blocks += handler.tbl[i].num_blocks - handler.tbl[i].num_free_blocks;
    }

    free_blocks += handler.mmaped.num_blocks - handler.mmaped.num_free_blocks;
    return free_blocks;
}

size_t _num_allocated_bytes(){
    int alloc_bytes = 0;
    for (int i = 0; i <= MAX_ORDER; i++){
        MallocMetadata* curr = handler.tbl[i].first_data_list;
        while (curr != nullptr){
            if (!curr->is_free){
                alloc_bytes += curr->size;
            }
            curr = curr->next;
        }
    }
    MallocMetadata* curr = handler.mmaped.first_data_list;
    while (curr != nullptr){
        if (!curr->is_free){
            alloc_bytes += curr->size;
        }
        curr = curr->next;
    }
    return alloc_bytes;
}

size_t _num_meta_data_bytes(){
    return _num_allocated_blocks() * _size_meta_data();
}

size_t _size_meta_data(){
    return sizeof(MallocMetadata);
}

void _init_handler(){
    DEBUG_PRINT();

    handler.init = true;
    for (int i = 0; i <= MAX_ORDER; i++){
        handler.tbl[i] = { nullptr, nullptr, 0, 0 };
        handler.tbl[i].num_free_blocks = 0;
    }
    handler.mmaped = { nullptr, nullptr, 0, 0 };
    handler.mmaped.num_free_blocks = 0;

    const uintptr_t ALLOCATE = 4 * MB; // 32 * 128KiB = 4MiB = 2^22 B

    // get current brk
    void* curr_brk = sbrk(0);
    if (curr_brk == (void*)-1){
        exit(1);
    }
    uintptr_t curr_addr = (uintptr_t)curr_brk;

    // allocate aligned memory and calculate alignment
    uintptr_t misalignment = ALLOCATE - (curr_addr % ALLOCATE);

    void* result = sbrk(misalignment + ALLOCATE);
    if (result == (void*)-1){
        exit(1);
    }

    // initialize ORDER_10 blocks, allocate 32 mega blocks of size 128 KB each. 
    MallocMetadata* aligned_mem_start = (MallocMetadata*)(misalignment + curr_addr);
    DEBUG_ASSERT((size_t)aligned_mem_start % (4 * MB) == 0);

    MallocMetadata* temp = aligned_mem_start;
    int i = 0;
    handler.tbl[MAX_ORDER].first_data_list = aligned_mem_start;
    do{
        _init_block(temp, &handler.tbl[MAX_ORDER], 128 * KB, true);
        temp = (MallocMetadata*)BYTE_OFFSET(temp, 128 * KB);
        i++;
    } while (i < 32);
}

// static int t = 0;
void _init_block(
    MallocMetadata* block,
    DataList* datalist,
    size_t size,
    bool is_real_size = false
){
    DEBUG_PRINT();
    block->is_free = true;
    block->next = nullptr;
    block->prev = datalist->last_data_list;
    block->size = size - ((is_real_size) ? sizeof(MallocMetadata) : 0);
    block->real_size = size + ((is_real_size) ? 0 : sizeof(MallocMetadata));
    block->data = (void*)((char*)block + sizeof(MallocMetadata));

    if (datalist->last_data_list){  // if there is a last node
        datalist->last_data_list->next = block;
    }

    datalist->last_data_list = block;
    datalist->num_blocks++;
    datalist->num_free_blocks++;
}

// remove a block from a list but deos not free it
void _remove_block(MallocMetadata* block, DataList* datalist){
    DEBUG_PRINT();
    if (block->prev)
        block->prev->next = block->next;
    if (block->next)
        block->next->prev = block->prev;
    if (block->is_free)
        datalist->num_free_blocks--;
    
    datalist->num_blocks--;

    block->next = nullptr;
    block->prev = nullptr;
}

// merge blocks recursively
bool _check_merge_blocks(
    MallocMetadata* block,
    size_t request_size,
    int order
){
    DEBUG_PRINT();
    if (request_size > Orders_mapping::order_to_size(MAX_ORDER))
        return false;
    
    if (Orders_mapping::order_to_size(order) >= request_size)
        return true;
    
    // check if buddy is also freed
    MallocMetadata* buddy = (MallocMetadata*)((size_t)block ^ Orders_mapping::order_to_size(order));
    if (!buddy->is_free){
        return false;
    }

    MallocMetadata* comb_block = ((size_t)block < (size_t)buddy) ? block : buddy;
    // if (order + 1 < MAX_ORDER)
    return _check_merge_blocks(comb_block, request_size, order + 1);
}

MallocMetadata* _merge_blocks(
    MallocMetadata* block,
    DataList* org_datalist,
    DataList* dst_datalist,
    int order,
    size_t request_size = 0 // real size
){
    DEBUG_PRINT();
    if (request_size != 0 && request_size <= Orders_mapping::order_to_size(order)){
        _populate_block(block, org_datalist, request_size);
        return block;
    }

    if (order == MAX_ORDER)
        return block;

    // check if buddy is also freed
    MallocMetadata* buddy = (MallocMetadata*)((size_t)block ^ block->real_size);
    if (!buddy->is_free){
        return block;
    }

    _remove_block(block, org_datalist);
    _remove_block(buddy, org_datalist);

    MallocMetadata* comb_block = ((size_t)block < (size_t)buddy) ? block : buddy;
    _init_block(comb_block, dst_datalist, comb_block->real_size * 2);

    if (order + 1 < MAX_ORDER)
        _merge_blocks(comb_block, &handler.tbl[order + 1], &handler.tbl[order + 2], order + 1, request_size);
    return comb_block;
}

MallocMetadata* _split_block(
    MallocMetadata* block,
    DataList* org_datalist,
    DataList* dst_datalist,
    size_t size
){
    DEBUG_PRINT();
    _remove_block(block, org_datalist);
    _init_block(block, dst_datalist, size, true);

    MallocMetadata* b_block = (MallocMetadata*)((char*)block + size);
    _init_block(b_block, dst_datalist, size, true);
    return b_block;
}

void _populate_block(
    MallocMetadata* block,
    DataList* datalist,
    size_t size
){
    DEBUG_PRINT();
    // we don't really need this but maybe for fragmentation computation or something
    block->size = size;
    block->is_free = false;
    datalist->num_free_blocks--;
}

// finds an empty block for allocation (else nullptr)
MallocMetadata* find_empty_block(DataList* list){
    DEBUG_PRINT();
    if (list->num_free_blocks == 0){
        return nullptr;
    }
    MallocMetadata* current = list->first_data_list;
    while (current != nullptr){
        if (current->is_free)
            return current;
        current = current->next;
    }
    return nullptr;
}


// """"""""""""""""""""""""""""""" mallocs """"""""""""""""""""""""""""""

void* smalloc(size_t size){
    DEBUG_PRINT();

    // checks for invalid inputs
    if (size == 0 || size > 100000000){ // if equal to 0 or larger then 10^8
        return NULL;
    }

    if (!handler.init){
        _init_handler();
    }

    int order = Orders_mapping::size_to_order(size, false);
    if (order == Orders_mapping::MEGA){
        void* mapped = mmap(NULL, size + sizeof(MallocMetadata),
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (mapped == nullptr){
            // handle error
            return nullptr;
        }
        MallocMetadata* block = (MallocMetadata*)mapped;
        _init_block(block, &handler.mmaped, size, false);
        _populate_block(block, &handler.mmaped, size);
        return block->data;
    }

    // try to allocate in incrementing size. 
    DataList* datalist = &handler.tbl[order];
    bool perfect_fit = true;
    while (true){
        datalist = &handler.tbl[order];
        MallocMetadata* block = find_empty_block(datalist);

        if (block == nullptr){
            if (order < MAX_ORDER){
                order++;
                perfect_fit = false;
                continue;
            }
            else{
                // could not allocate
                return nullptr;
            }
        }

        // TODO: split to buddies (should this be done recursivly to fine grain?)
        size_t shrink_to_size = Orders_mapping::order_to_size(order - 1);
        if (order > 0 && size + sizeof(MallocMetadata) <= shrink_to_size && !perfect_fit){
            datalist = &handler.tbl[order - 1];
            _split_block(block, &handler.tbl[order], datalist, shrink_to_size);
        }
        _populate_block(block, datalist, size);
        return block->data;
    }

    return nullptr; // won't reach this
}

void* scalloc(size_t num, size_t size){
    DEBUG_PRINT();
    // checks for invalid inputs
    if (size == 0 || num == 0 || (size * num) > 100000000){ // if equal to 0 or larger then 10^8
        return NULL;
    }

    void* allocated_space = smalloc(num * size);
    if (allocated_space == nullptr){
        return nullptr;
    }

    std::memset(allocated_space, 0, num * size);

    return allocated_space;
}


void sfree(void* p){
    DEBUG_PRINT();
    if (p == nullptr)
        return;

    if (!handler.init){
        return;
    }

    MallocMetadata* metadata_p = (MallocMetadata*)((char*)p - sizeof(MallocMetadata));
    metadata_p->is_free = true;
    int order = Orders_mapping::size_to_order(metadata_p->real_size, true);
    DataList* datalist;
    if (order == Orders_mapping::MEGA)
        datalist = &handler.mmaped;
    else
        datalist = &handler.tbl[order];
    
    datalist->num_free_blocks++;

    // unmap
    if (order == Orders_mapping::MEGA){
        munmap(metadata_p, metadata_p->real_size);
    }
    else if (order < MAX_ORDER){
        _merge_blocks(metadata_p, datalist, &handler.tbl[order + 1], order);    // merge with buddy if buddy is also free recursively
    }
    metadata_p->is_free = true;
    return;
}

void* srealloc(void* oldp, size_t size){
    DEBUG_PRINT();
    // checks for invalid inputs
    if (size == 0 || size > 100000000){ // if equal to 0 or larger then 10^8
        return NULL;
    }

    // check if must allocate
    if (oldp == nullptr)
        return smalloc(size);

    size_t metadata_size = sizeof(MallocMetadata);
    MallocMetadata* oldp_metadata = (MallocMetadata*)((char*)oldp - metadata_size);
    int order = Orders_mapping::size_to_order(oldp_metadata->real_size, true);

    // try to reuse
    if (order == Orders_mapping::MEGA){
        if (size + metadata_size == oldp_metadata->real_size)
            return oldp;
    }
    else if (size + metadata_size <= oldp_metadata->real_size){
        return oldp;
    }

    void* result;
    bool merged = false;

    // try using merge
    if (_check_merge_blocks(oldp_metadata, size, order)){
        // itrative merge until no need to merge
        result = (void*)_merge_blocks(oldp_metadata, &handler.tbl[order], &handler.tbl[order + 1], order, size);
        merged = true;
    }
    else{
        // allocate new
        result = smalloc(size);
        if (result == nullptr)
            return nullptr;
    }

    size_t old_data_size = oldp_metadata->size; // if oldp is smaller from size
    std::memmove(result, oldp, std::min(old_data_size, size));
    if (!merged)
        sfree(oldp);
    return result;
}



