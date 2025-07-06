#include <unistd.h> // for sbrk
#include <stddef.h> // for size_t
#include <bits/stdc++.h> // for memset
#include "header.h"

#include <iostream>
#include <cstring>
#include <cassert>

#define MINI_TEST(test) {assert(test); std::cout << "passed: [ " << #test << " ]" << std::endl;} 

// Function declarations
size_t _num_free_blocks();
size_t _num_free_bytes();
size_t _num_allocated_blocks();
size_t _num_allocated_bytes();
size_t _num_meta_data_bytes();
size_t _size_meta_data();

void test_smalloc_and_stats(){
    void* p = smalloc(1000);

    size_t initial_free_blocks = _num_free_blocks();
    std::cout << initial_free_blocks << std::endl;
    MINI_TEST(initial_free_blocks == 32);

    size_t initial_allocated_blocks = _num_allocated_blocks();
    MINI_TEST(p != nullptr);

    MINI_TEST(_num_free_blocks() == initial_free_blocks - 1 + 2);
    MINI_TEST(_num_allocated_blocks() == initial_allocated_blocks + 1);

    MINI_TEST(_num_allocated_bytes() == 64 * 1024);

    MINI_TEST(_num_meta_data_bytes() == _size_meta_data() * (initial_allocated_blocks + 1));

    MINI_TEST(_num_free_blocks() == initial_allocated_blocks);

    void* p2 = smalloc(500); // Should succeed
    MINI_TEST(p2 != nullptr);
    MINI_TEST(_num_allocated_blocks() == 2);
    MINI_TEST(_num_allocated_bytes() >= 1500);
}

void test_sfree_and_reuse() {
    void* p1 = smalloc(256); // Should succeed
    assert(p1 != nullptr);
    sfree(p1); // Free the block

    size_t free_blocks = _num_free_blocks();
    size_t free_bytes = _num_free_bytes();

    void* p2 = smalloc(200); // Should reuse the freed block if possible
    assert(p2 != nullptr);
    assert(_num_free_blocks() <= free_blocks); // May merge/split
    assert(_num_free_bytes() <= free_bytes);
}

void test_scalloc() {
    void* p = scalloc(10, 100); // 1000 bytes
    assert(p != nullptr);

    char* data = static_cast<char*>(p);
    for (int i = 0; i < 1000; ++i) {
        assert(data[i] == 0);
    }

    sfree(p);
}

void test_srealloc_shrink_and_expand() {
    void* p = smalloc(512);
    assert(p != nullptr);
    std::memset(p, 0xAB, 512);

    // Shrinking
    void* shrunk = srealloc(p, 200);
    assert(shrunk == p); // Should reuse the same block
    char* c = static_cast<char*>(shrunk);
    for (int i = 0; i < 200; ++i) {
        assert(c[i] == (char)0xAB);
    }

    // Expanding
    void* expanded = srealloc(shrunk, 2048);
    assert(expanded != nullptr);
    c = static_cast<char*>(expanded);
    for (int i = 0; i < 200; ++i) {
        assert(c[i] == (char)0xAB); // Should still contain previous data
    }

    sfree(expanded);
}

void test_large_allocation() {
    // Should go to mmap (128KB or more)
    void* big = smalloc(128 * 1024);
    assert(big != nullptr);

    std::memset(big, 1, 128 * 1024);

    void* resized = srealloc(big, 256 * 1024);
    assert(resized != nullptr);
    sfree(resized);
}

int main(){
    std::cout << "sizeof metadata is: " << sizeof(MallocMetadata) << std::endl;

    std::cout << "Testing smalloc and stats...\n";
    test_smalloc_and_stats();
    std::cout << "Passed.\n";

    std::cout << "Testing sfree and reuse...\n";
    test_sfree_and_reuse();
    std::cout << "Passed.\n";

    std::cout << "Testing scalloc...\n";
    test_scalloc();
    std::cout << "Passed.\n";

    std::cout << "Testing srealloc (shrink/expand)...\n";
    test_srealloc_shrink_and_expand();
    std::cout << "Passed.\n";

    std::cout << "Testing large mmap allocation...\n";
    test_large_allocation();
    std::cout << "Passed.\n";

    std::cout << "\nAll tests passed successfully!\n";
    return 0;
}