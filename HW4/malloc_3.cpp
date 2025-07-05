#include <unistd.h> // for sbrk
#include <stddef.h> // for size_t
#include <bits/stdc++.h> // for memset


#define MAX_ORDER 10
#define KB (size_t)1 << 10
#define MB (size_t)1 << 20


struct MallocMetadata{
    size_t size = 0;  // requested size + meta-data structure size
    size_t real_size = 0;
    bool is_free = true;
    void* data = nullptr;  // pointer to the first byte of our allocated data
    MallocMetadata* next = nullptr;
    MallocMetadata* prev = nullptr;
};

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
    static int map_order(size_t value){
        value += sizeof(MallocMetadata);
        size_t last = 0;
        for (int i = 0; i <= MAX_ORDER; i++)
            if (value <= Orders_mapping::orders[i])
                return i;
        return MEGA;
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

    DataList tbl[MAX_ORDER];
    DataList mmaped;
} handler;

void _init_handler(){
    handler.init = true;

    const size_t ALLOCATE = 1 << 22; // 32 * 128KiB = 4MiB = 2^22 B

    // get current brk
    void* curr_brk = sbrk(0);
    if (curr_brk == (void*)-1){
        exit(1);
    }
    uintptr_t curr_addr = (uintptr_t)curr_brk;

    // allocate aligned memory and calculate alignment
    const uintptr_t align_size_mask = 4 * MB - 1;   // create alignment mask
    size_t alignemnt = ALLOCATE - align_size_mask & curr_addr;
    void* result = sbrk(alignemnt + ALLOCATE);
    if (result == (void*)-1){
        exit(1);
    }

    // initialize ORDER_10 blocks, allocate 32 mega blocks of size 128 KB each. 
    MallocMetadata* aligned_mem_start = (MallocMetadata*)((uintptr_t)result + curr_addr);
    MallocMetadata* temp = aligned_mem_start;
    int i = 0;
    handler.tbl[MAX_ORDER].first_data_list = aligned_mem_start;
    do{
        _init_block(temp, &handler.tbl[MAX_ORDER], 128 * KB - sizeof(MallocMetadata));
        temp += 128 * KB;
        i++;
    } while (i < 32);

    // assert(i == 32);
}

void _init_block(MallocMetadata* block, DataList* datalist, size_t size){
    block->is_free = true;
    block->next = nullptr;
    block->prev = datalist->last_data_list;
    block->size = size;
    block->real_size = size + sizeof(MallocMetadata);
    block->data = (void*)((char*)block + sizeof(MallocMetadata));

    if (datalist->last_data_list){  // if there is a last node
        datalist->last_data_list->next = block;
    }

    datalist->last_data_list = block;
    datalist->num_blocks++;
    datalist->num_free_blocks++;
}

void _populate_block(MallocMetadata* block, DataList* datalist, size_t size){
    // we don't really need this but maybe for fragmentation computation or something
    block->size = size + sizeof(MallocMetadata);
    block->is_free = false;
    datalist->num_free_blocks--;
}

// finds an empty block for allocation (else nullptr)
MallocMetadata* find_empty_block(DataList* list){
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
    // checks for invalid inputs
    if (size == 0 || size > 100000000){ // if equal to 0 or larger then 10^8
        return NULL;
    }

    if (!handler.init){
        _init_handler();
    }

    int order = Orders_mapping::map_order(size);
    if (order == Orders_mapping::MEGA){
        void* mapped = mmap(NULL, size + sizeof(MallocMetadata),
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (mapped == nullptr){
            // handle error
            return nullptr;
        }
        MallocMetadata* block = (MallocMetadata*)mapped;
        _init_block(block, &handler.mmaped, size);
        _populate_block(block, &handler.mmaped, size);
        return block->data;
    }

    // try to allocate in incrementing size. 
    DataList* datalist = &handler.tbl[order];
    bool perfect_fit = true;
    while (true){
        MallocMetadata* block = find_empty_block(datalist);
        if (block == nullptr){
            if (order < MAX_ORDER){
                order++;
                datalist = &handler.tbl[order];
                perfect_fit = false;
                continue;
            }
            else{
                // could not allocate
                return nullptr;
            }
        }

        // populate block
        if (perfect_fit){
            _populate_block(block, datalist, size);
            return block->data;
        }
        else{
            // TODO: split to buddies (should this be done recursivly?)
        }
    }

    return nullptr; // won't reach this
}


void* scalloc(size_t num, size_t size){
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
    if (p == nullptr)
        return;

    if (!handler.init){
        return;
    }

    MallocMetadata* metadata_p = (MallocMetadata*)((char*)p - sizeof(MallocMetadata));
    metadata_p->is_free = true;
    int order = Orders_mapping::map_order(metadata_p->real_size - sizeof(MallocMetadata));
    DataList* datalist;
    if (order == Orders_mapping::MEGA)
        datalist = &handler.mmaped;
    else
        datalist = &handler.tbl[order];
    
    datalist->num_free_blocks++;

    // TODO: merge with buddy 
    return;
}

// void* srealloc(void* oldp, size_t size){

//     // checks for invalid inputs
//     if (size == 0 || size > 100000000){ // if equal to 0 or larger then 10^8
//         return NULL;
//     }

//     if (oldp == nullptr)
//         return smalloc(size);

//     size_t metadata_size = _size_meta_data();
//     MallocMetadata* oldp_metadata = (MallocMetadata*)((char*)oldp - metadata_size);

//     // If ‘size’ is smaller than or equal to the current block’s size, reuses the same block.
//     if ((size + metadata_size) <= oldp_metadata->size){
//         return oldp;
//     }

//     void* result = smalloc(size);
//     if (result == nullptr)
//         return nullptr;

//     size_t old_data_size = oldp_metadata->size - _size_meta_data(); // if oldp is smaller from size
//     std::memmove(result, oldp, std::min(old_data_size, size));
//     sfree(oldp);
//     return result;
// }



