#pragma once

#include <cstdio>
#if __has_include(<print>)
    #include <print>
    #define BLUESTL_USE_STD_PRINT 1
#else
    #define BLUESTL_USE_STD_PRINT 0
#endif
#include "../include/bluestl/allocator.h"
#include "../include/bluestl/fixed_hash_map.h"

class TestAllocator : public bluestl::allocator {
public:
    TestAllocator(const char* test_name) : bluestl::allocator(test_name), allocate_count(0), deallocate_count(0) {
    }

    ~TestAllocator() {
#if BLUESTL_USE_STD_PRINT
        if (allocate_count == deallocate_count) {
            std::print("[TestAllocator] {} メモリリークなし (allocate: {}, deallocate: {})\n", get_name(), allocate_count, deallocate_count);
        } else {
            std::print("[TestAllocator] {} メモリリーク検出! (allocate: {}, deallocate: {})\n", get_name(), allocate_count, deallocate_count);
        }
#else
        if(allocate_count == deallocate_count) {
            std::printf("[TestAllocator] %s メモリリークなし (allocate: %zu, deallocate: %zu)\n", get_name(), allocate_count, deallocate_count);
        } else {
            std::printf("[TestAllocator] %s メモリリーク検出! (allocate: %zu, deallocate: %zu)\n", get_name(), allocate_count, deallocate_count);
        }
#endif
    }

    void* allocate(size_t n) override {
        ++allocate_count;
		void* p = ::operator new(n);
		allocations.insert(p, n);
        return p;
    }

    void deallocate(void* p, size_t) override {
        ++deallocate_count;
		auto it = allocations.find(p);
		if (it) {
			allocations.erase(it);
		}
		else {
#if BLUESTL_USE_STD_PRINT
            std::print("[TestAllocator] 不明なポインタの解放: {}\n", p);
#else
            std::printf("[TestAllocator] 不明なポインタの解放: %p\n", p);
#endif
		}
        ::operator delete(p);
    }

private:
    size_t allocate_count;
    size_t deallocate_count;
	bluestl::fixed_hash_map<void*, size_t, 8> allocations;

};
