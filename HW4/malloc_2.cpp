#include <unistd.h> // for sbrk
#include <stddef.h> // for size_t
#include <bits/stdc++.h> // for memset

struct MallocMetadata {
    size_t size;  // requested size + meta-data structure size
    bool is_free;
    void* data;  // pointer to the first byte of our allocated data
    MallocMetadata* next;
    MallocMetadata* prev;
};

struct DataList {
    MallocMetadata* first_data_list;
    MallocMetadata* last_data_list;
    size_t num_blocks;
    size_t num_free_blocks;
};

DataList dataList = {
        nullptr,
        nullptr,
        0,
        0
};


size_t _size_meta_data() {
    return sizeof(MallocMetadata);
}

size_t _num_free_blocks() {
    return dataList.num_free_blocks;
}

// Returns the number of bytes in all allocated blocks in the heap that are currently free,
// excluding the bytes used by the meta-data structs
size_t _num_free_bytes() {
    MallocMetadata* data_list_temp = dataList.first_data_list;
    size_t metadata_size = _size_meta_data();
    size_t result = 0;

    // searches for a free blocks
    while (data_list_temp != nullptr) {
        if (data_list_temp->is_free) {
            result += data_list_temp->size - metadata_size;
        }
        data_list_temp = data_list_temp->next;
    }
    return result;
}

size_t _num_allocated_blocks() {
    return dataList.num_blocks;
}

size_t _num_allocated_bytes() {
    MallocMetadata* data_list_temp = dataList.first_data_list;
    size_t metadata_size = _size_meta_data();
    size_t result = 0;

    // go thruout all blocks
    while (data_list_temp != nullptr) {
        result += data_list_temp->size - metadata_size;
        data_list_temp = data_list_temp->next;
    }
    return result;
}

// Returns the overall number of meta-data bytes currently in the heap.
size_t _num_meta_data_bytes() {
    return _size_meta_data() * _num_allocated_blocks();
}

// """"""""""""""""""""""""""""""" mallocs """"""""""""""""""""""""""""""

void* smalloc(size_t size) {

    // checks for invalid inputs
    if (size == 0 || size > 100000000) { // if equal to 0 or larger then 10^8
        return NULL;
    }

    MallocMetadata* data_list_temp = dataList.first_data_list;
    size_t metadata_size = _size_meta_data();

    // searches for a free block with at least "size" bytes
    while (data_list_temp != nullptr) {
        if (data_list_temp->size >= (size + metadata_size) && data_list_temp->is_free) {
            data_list_temp->is_free = false;
            dataList.num_free_blocks--;
            return data_list_temp->data;
        }
        data_list_temp = data_list_temp->next;
    }

    void* result = sbrk(size + metadata_size);
    if (result == (void*)-1) { // sbrk failed
        return NULL;
    }
    // change fields in metadata
    auto* new_node = (MallocMetadata*)result;
    new_node->size = size + metadata_size;
    new_node->is_free = false;
    new_node->next = nullptr;
    new_node->prev = dataList.last_data_list;
    new_node->data = (void*)((char*)result + metadata_size);

    if (!dataList.first_data_list) {  // if first node in list is nullptr
        dataList.first_data_list = new_node;
    }
    if (dataList.last_data_list) {  // if there is a last node
        dataList.last_data_list->next = new_node;
    }

    dataList.last_data_list = new_node;
    dataList.num_blocks++;

    return new_node->data;
}


void* scalloc(size_t num, size_t size) {
    // checks for invalid inputs
    if (size == 0 || num == 0 || (size * num) > 100000000) { // if equal to 0 or larger then 10^8
        return NULL;
    }

    void* allocated_space = smalloc(num * size);
    if (allocated_space == nullptr) {
        return nullptr;
    }

    std::memset(allocated_space, 0, num * size);

    return allocated_space;
}


void sfree(void* p) {
    if (p == nullptr)
        return;

    MallocMetadata* metadata_p = (MallocMetadata*)((char*)p - _size_meta_data());
    metadata_p->is_free = true;
    dataList.num_free_blocks++;
    return;
}

void* srealloc(void* oldp, size_t size) {

    // checks for invalid inputs
    if (size == 0 || size > 100000000) { // if equal to 0 or larger then 10^8
        return NULL;
    }

    if (oldp == nullptr)
        return smalloc(size);

    size_t metadata_size = _size_meta_data();
    MallocMetadata* oldp_metadata = (MallocMetadata*)((char*)oldp - metadata_size);

    // If ‘size’ is smaller than or equal to the current block’s size, reuses the same block.
    if ((size + metadata_size) <= oldp_metadata->size) {
        return oldp;
    }

    void* result = smalloc(size);
    if (result == nullptr)
        return nullptr;

    size_t old_data_size = oldp_metadata->size - _size_meta_data(); // if oldp is smaller from size
    std::memmove(result, oldp, std::min(old_data_size, size));
    sfree(oldp);
    return result;
}



