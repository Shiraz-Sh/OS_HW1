#include <iostream>
#include <cassert>
#include <cstring>
#include <unistd.h>
#include <cstdint>

// Include your malloc_2.cpp implementation
// Assuming the functions are declared as:
void* smalloc(size_t size);
void* scalloc(size_t num, size_t size);
void sfree(void* p);
void* srealloc(void* oldp, size_t size);
size_t _num_free_blocks();
size_t _num_free_bytes();
size_t _num_allocated_blocks();
size_t _num_allocated_bytes();
size_t _num_meta_data_bytes();
size_t _size_meta_data();

class Malloc2Tester {
private:
    int tests_passed = 0;
    int tests_failed = 0;

    void assert_test(bool condition, const char* test_name) {
        if (condition) {
            std::cout << "[PASS] " << test_name << std::endl;
            tests_passed++;
        } else {
            std::cout << "[FAIL] " << test_name << std::endl;
            tests_failed++;
        }
    }

    void print_stats(const char* context) {
        std::cout << "=== " << context << " ===" << std::endl;
        std::cout << "Free blocks: " << _num_free_blocks() << std::endl;
        std::cout << "Free bytes: " << _num_free_bytes() << std::endl;
        std::cout << "Allocated blocks: " << _num_allocated_blocks() << std::endl;
        std::cout << "Allocated bytes: " << _num_allocated_bytes() << std::endl;
        std::cout << "Metadata bytes: " << _num_meta_data_bytes() << std::endl;
        std::cout << "Metadata size: " << _size_meta_data() << std::endl;
        std::cout << std::endl;
    }

public:
    void test_smalloc_basic() {
        std::cout << "\n=== Testing smalloc Basic Functionality ===" << std::endl;

        // Test normal allocation
        void* ptr1 = smalloc(100);
        assert_test(ptr1 != nullptr, "smalloc(100) returns non-null");

        // Test zero size
        void* ptr2 = smalloc(0);
        assert_test(ptr2 == nullptr, "smalloc(0) returns null");

        // Test large size (> 10^8)
        void* ptr3 = smalloc(100000001);
        assert_test(ptr3 == nullptr, "smalloc(>10^8) returns null");

        // Test writing to allocated memory
        if (ptr1) {
            memset(ptr1, 0xAA, 100);
            unsigned char* bytes = (unsigned char*)ptr1;
            bool write_success = true;
            for (int i = 0; i < 100; i++) {
                if (bytes[i] != 0xAA) {
                    write_success = false;
                    break;
                }
            }
            assert_test(write_success, "Can write to allocated memory");
        }

        print_stats("After basic smalloc tests");
    }

    void test_scalloc_basic() {
        std::cout << "\n=== Testing scalloc Basic Functionality ===" << std::endl;

        // Test normal allocation
        void* ptr1 = scalloc(10, 20);
        assert_test(ptr1 != nullptr, "scalloc(10, 20) returns non-null");

        // Test zero num
        void* ptr2 = scalloc(0, 20);
        assert_test(ptr2 == nullptr, "scalloc(0, 20) returns null");

        // Test zero size
        void* ptr3 = scalloc(10, 0);
        assert_test(ptr3 == nullptr, "scalloc(10, 0) returns null");

        // Test large total size (> 10^8)
        void* ptr4 = scalloc(1000, 100001);
        assert_test(ptr4 == nullptr, "scalloc with total >10^8 returns null");

        // Test that memory is zeroed
        if (ptr1) {
            unsigned char* bytes = (unsigned char*)ptr1;
            bool is_zeroed = true;
            for (int i = 0; i < 200; i++) {
                if (bytes[i] != 0) {
                    is_zeroed = false;
                    break;
                }
            }
            assert_test(is_zeroed, "scalloc memory is zeroed");
        }

        print_stats("After basic scalloc tests");
    }

    void test_sfree_basic() {
        std::cout << "\n=== Testing sfree Basic Functionality ===" << std::endl;

        size_t initial_free_blocks = _num_free_blocks();
        size_t initial_allocated_blocks = _num_allocated_blocks();

        void* ptr1 = smalloc(50);
        assert_test(ptr1 != nullptr, "Allocation for free test successful");

        size_t after_alloc_free_blocks = _num_free_blocks();
        size_t after_alloc_allocated_blocks = _num_allocated_blocks();

        sfree(ptr1);

        size_t after_free_free_blocks = _num_free_blocks();
        size_t after_free_allocated_blocks = _num_allocated_blocks();

        assert_test(after_free_free_blocks > after_alloc_free_blocks,
                    "Free blocks increased after sfree");
        assert_test(after_free_allocated_blocks == after_alloc_allocated_blocks,
                    "Total allocated blocks unchanged after sfree");

        // Test freeing null pointer (should not crash)
        sfree(nullptr);
        assert_test(true, "sfree(nullptr) doesn't crash");

        print_stats("After basic sfree tests");
    }

    void test_srealloc_basic() {
        std::cout << "\n=== Testing srealloc Basic Functionality ===" << std::endl;

        // Test realloc with null pointer (should behave like malloc)
        void* ptr1 = srealloc(nullptr, 100);
        assert_test(ptr1 != nullptr, "srealloc(nullptr, 100) allocates memory");

        // Write some data
        if (ptr1) {
            strcpy((char*)ptr1, "Hello World");
        }

        // Test realloc to larger size
        void* ptr2 = srealloc(ptr1, 200);
        assert_test(ptr2 != nullptr, "srealloc to larger size succeeds");

        // Check data preservation
        if (ptr2) {
            bool data_preserved = (strcmp((char*)ptr2, "Hello World") == 0);
            assert_test(data_preserved, "Data preserved during realloc");
        }

        // Test realloc to smaller size (should reuse same block)
        void* ptr3 = srealloc(ptr2, 50);
        assert_test(ptr3 != nullptr, "srealloc to smaller size succeeds");

        // Test realloc with zero size
        void* ptr4 = srealloc(ptr3, 0);
        assert_test(ptr4 == nullptr, "srealloc with size 0 returns null");

        // Test realloc with size > 10^8
        void* ptr5 = smalloc(100);
        void* ptr6 = srealloc(ptr5, 100000001);
        assert_test(ptr6 == nullptr, "srealloc with size >10^8 returns null");

        // Original pointer should still be valid after failed realloc
        if (ptr5) {
            memset(ptr5, 0xBB, 100);
            assert_test(true, "Original pointer valid after failed realloc");
        }

        print_stats("After basic srealloc tests");
    }

    void test_memory_reuse() {
        std::cout << "\n=== Testing Memory Reuse ===" << std::endl;

        size_t initial_allocated_blocks = _num_allocated_blocks();

        // Allocate and free some blocks
        void* ptr1 = smalloc(100);
        void* ptr2 = smalloc(200);
        void* ptr3 = smalloc(150);

        size_t after_alloc_blocks = _num_allocated_blocks();
        assert_test(after_alloc_blocks == initial_allocated_blocks + 3,
                    "Three blocks allocated");

        sfree(ptr2); // Free middle block

        size_t after_free_blocks = _num_free_blocks();

        // Allocate a block that should fit in the freed space
        void* ptr4 = smalloc(50); // Smaller than 200, should reuse freed block

        size_t final_allocated_blocks = _num_allocated_blocks();
        assert_test(final_allocated_blocks == after_alloc_blocks,
                    "Block reused, total blocks unchanged");

        // Test that we can use the reused memory
        if (ptr4) {
            memset(ptr4, 0xCC, 50);
            assert_test(true, "Can write to reused memory");
        }

        print_stats("After memory reuse tests");
    }

    void test_ascending_order_allocation() {
        std::cout << "\n=== Testing Ascending Order Allocation ===" << std::endl;

        // Allocate several blocks
        void* ptr1 = smalloc(100);
        void* ptr2 = smalloc(100);
        void* ptr3 = smalloc(100);

        // Free them in reverse order
        sfree(ptr3);
        sfree(ptr1);
        sfree(ptr2);

        // Allocate new blocks - should get them in ascending address order
        void* new_ptr1 = smalloc(80);
        void* new_ptr2 = smalloc(80);
        void* new_ptr3 = smalloc(80);

        // Check that addresses are in ascending order
        // (This test assumes the implementation follows the spec)
        bool ascending = true;
        if (new_ptr1 && new_ptr2 && new_ptr3) {
            if (!((uintptr_t)new_ptr1 < (uintptr_t)new_ptr2 &&
                  (uintptr_t)new_ptr2 < (uintptr_t)new_ptr3)) {
                ascending = false;
            }
        }
        assert_test(ascending, "Blocks allocated in ascending address order");

        print_stats("After ascending order tests");
    }

    void test_statistics_functions() {
        std::cout << "\n=== Testing Statistics Functions ===" << std::endl;

        // Start fresh understanding of current state
        size_t initial_free_blocks = _num_free_blocks();
        size_t initial_free_bytes = _num_free_bytes();
        size_t initial_allocated_blocks = _num_allocated_blocks();
        size_t initial_allocated_bytes = _num_allocated_bytes();
        size_t metadata_size = _size_meta_data();

        assert_test(metadata_size > 0, "Metadata size is positive");

        // Allocate a known size
        void* ptr = smalloc(1000);

        size_t after_alloc_blocks = _num_allocated_blocks();
        size_t after_alloc_bytes = _num_allocated_bytes();
        size_t after_alloc_metadata = _num_meta_data_bytes();

        assert_test(after_alloc_blocks == initial_allocated_blocks + 1,
                    "Allocated blocks increased by 1");
        assert_test(after_alloc_bytes == initial_allocated_bytes + 1000,
                    "Allocated bytes increased by 1000");
        assert_test(after_alloc_metadata >= metadata_size,
                    "Metadata bytes accounts for new block");

        // Free the block
        sfree(ptr);

        size_t after_free_free_blocks = _num_free_blocks();
        size_t after_free_free_bytes = _num_free_bytes();
        size_t after_free_allocated_blocks = _num_allocated_blocks();
        size_t after_free_allocated_bytes = _num_allocated_bytes();

        assert_test(after_free_free_blocks > initial_free_blocks,
                    "Free blocks increased after freeing");
        assert_test(after_free_free_bytes >= 1000,
                    "Free bytes includes freed block");
        assert_test(after_free_allocated_blocks == after_alloc_blocks,
                    "Total allocated blocks unchanged after free");
        assert_test(after_free_allocated_bytes == after_alloc_bytes,
                    "Total allocated bytes unchanged after free");

        print_stats("After statistics tests");
    }

    void test_large_block_reuse() {
        std::cout << "\n=== Testing Large Block Reuse ===" << std::endl;

        // Allocate a large block
        void* large_ptr = smalloc(2000);
        assert_test(large_ptr != nullptr, "Large block allocation successful");

        sfree(large_ptr);

        // Allocate a smaller block - should reuse the large block
        void* small_ptr = smalloc(500);
        assert_test(small_ptr != nullptr, "Small block allocation successful");

        // The large block should now be marked as used entirely
        size_t free_bytes = _num_free_bytes();

        // Allocate another block that definitely won't fit in remaining space
        void* another_ptr = smalloc(1600); // Won't fit in remaining 1500 bytes
        assert_test(another_ptr != nullptr, "Another allocation successful");

        print_stats("After large block reuse tests");
    }

    void test_stress_allocation() {
        std::cout << "\n=== Stress Testing ===" << std::endl;

        const int NUM_ALLOCS = 100;
        void* ptrs[NUM_ALLOCS];

        // Allocate many small blocks
        for (int i = 0; i < NUM_ALLOCS; i++) {
            ptrs[i] = smalloc(i + 10); // Variable sizes
            assert_test(ptrs[i] != nullptr, "Stress allocation successful");

            // Write to each block
            if (ptrs[i]) {
                memset(ptrs[i], i % 256, i + 10);
            }
        }

        // Free every other block
        for (int i = 0; i < NUM_ALLOCS; i += 2) {
            sfree(ptrs[i]);
            ptrs[i] = nullptr;
        }

        // Verify remaining blocks still have correct data
        bool data_intact = true;
        for (int i = 1; i < NUM_ALLOCS; i += 2) {
            if (ptrs[i]) {
                unsigned char* bytes = (unsigned char*)ptrs[i];
                for (int j = 0; j < i + 10; j++) {
                    if (bytes[j] != (unsigned char)(i % 256)) {
                        data_intact = false;
                        break;
                    }
                }
                if (!data_intact) break;
            }
        }
        assert_test(data_intact, "Data integrity maintained during stress test");

        // Free remaining blocks
        for (int i = 1; i < NUM_ALLOCS; i += 2) {
            sfree(ptrs[i]);
        }

        print_stats("After stress test");
    }

    void run_all_tests() {
        std::cout << "Running malloc_2 Test Suite" << std::endl;
        std::cout << "============================" << std::endl;

        test_smalloc_basic();
        test_scalloc_basic();
        test_sfree_basic();
        test_srealloc_basic();
        test_memory_reuse();
        test_ascending_order_allocation();
        test_statistics_functions();
        test_large_block_reuse();
        test_stress_allocation();

        std::cout << "\n=== Test Summary ===" << std::endl;
        std::cout << "Tests passed: " << tests_passed << std::endl;
        std::cout << "Tests failed: " << tests_failed << std::endl;
        std::cout << "Success rate: " << (100.0 * tests_passed / (tests_passed + tests_failed)) << "%" << std::endl;

        if (tests_failed == 0) {
            std::cout << "🎉 All tests passed!" << std::endl;
        } else {
            std::cout << "⚠️  Some tests failed. Please review your implementation." << std::endl;
        }
    }
};

int main() {
    Malloc2Tester tester;
    tester.run_all_tests();
    return 0;
}

/*
 * Compilation and Usage Instructions:
 * 
 * 1. Make sure you have malloc_2.cpp implemented according to the homework spec
 * 2. Compile with: g++ -o test_malloc2 malloc_2.cpp test_malloc2.cpp
 * 3. Run with: ./test_malloc2
 * 
 * Note: You may need to adjust the includes or function declarations 
 * depending on how you structured your malloc_2.cpp file.
 * 
 * Additional Test Ideas:
 * - Test with different allocation patterns
 * - Test edge cases with maximum allowed size (close to 10^8)
 * - Test fragmentation scenarios
 * - Test memory alignment (though not required in part 2)
 * - Test with calloc using different num/size combinations
 * - Test realloc with overlapping memory scenarios
 */