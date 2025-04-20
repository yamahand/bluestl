#pragma once

#include <cstdio>
#include "../include/stl/allocator.h"

class TestAllocator : public bluestl::allocator {
public:
    TestAllocator(const char* test_name) : bluestl::allocator(test_name), allocate_count(0), deallocate_count(0) {
    }

    ~TestAllocator() {
        if (allocate_count == deallocate_count) {
            printf("[TestAllocator] メモリリークなし (allocate: %zu, deallocate: %zu)\n", allocate_count, deallocate_count);
        } else {
            printf("[TestAllocator] メモリリーク検出! (allocate: %zu, deallocate: %zu)\n", allocate_count, deallocate_count);
        }
    }

    void* allocate(size_t n) override {
        ++allocate_count;
        return ::operator new(n);
    }

    void deallocate(void* p, size_t) override {
        ++deallocate_count;
        ::operator delete(p);
    }

private:
    size_t allocate_count;
    size_t deallocate_count;
};
