/**
 * @file benchmark_hash_map.cpp
 * @brief bluestl::hash_map のベンチマークテスト
 * 
 * Google Benchmarkフレームワークを使用したパフォーマンステスト
 */

#include <benchmark/benchmark.h>
#include "bluestl/hash_map.h"
#include "bluestl/allocator.h"
#include <unordered_map>
#include <random>
#include <string>

// ベンチマーク用のランダムキー生成
static std::vector<int> generate_random_keys(size_t size) {
    std::vector<int> keys;
    keys.reserve(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1000000);
    
    for (size_t i = 0; i < size; ++i) {
        keys.push_back(dis(gen));
    }
    return keys;
}

static std::vector<std::string> generate_string_keys(size_t size) {
    std::vector<std::string> keys;
    keys.reserve(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(100000, 999999);
    
    for (size_t i = 0; i < size; ++i) {
        keys.push_back("key_" + std::to_string(dis(gen)));
    }
    return keys;
}

// 挿入性能比較
static void BM_BluestlHashMap_Insert(benchmark::State& state) {
    const size_t N = state.range(0);
    auto keys = generate_random_keys(N);
    
    for (auto _ : state) {
        bluestl::allocator<bluestl::pair<int, int>> alloc;
        bluestl::hash_map<int, int, bluestl::allocator<bluestl::pair<int, int>>> map(alloc);
        
        for (size_t i = 0; i < N; ++i) {
            map.insert({keys[i], static_cast<int>(i)});
        }
        benchmark::DoNotOptimize(map.size());
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_StdUnorderedMap_Insert(benchmark::State& state) {
    const size_t N = state.range(0);
    auto keys = generate_random_keys(N);
    
    for (auto _ : state) {
        std::unordered_map<int, int> map;
        
        for (size_t i = 0; i < N; ++i) {
            map.insert({keys[i], static_cast<int>(i)});
        }
        benchmark::DoNotOptimize(map.size());
    }
    state.SetItemsProcessed(state.iterations() * N);
}

// 検索性能比較
static void BM_BluestlHashMap_Find(benchmark::State& state) {
    const size_t N = state.range(0);
    auto keys = generate_random_keys(N);
    
    bluestl::allocator<bluestl::pair<int, int>> alloc;
    bluestl::hash_map<int, int, bluestl::allocator<bluestl::pair<int, int>>> map(alloc);
    
    // データの事前挿入
    for (size_t i = 0; i < N; ++i) {
        map.insert({keys[i], static_cast<int>(i)});
    }
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, static_cast<int>(N - 1));
    
    for (auto _ : state) {
        int found_count = 0;
        for (int i = 0; i < 1000; ++i) {
            auto it = map.find(keys[dis(gen)]);
            if (it != map.end()) {
                found_count++;
            }
        }
        benchmark::DoNotOptimize(found_count);
    }
    state.SetItemsProcessed(state.iterations() * 1000);
}

static void BM_StdUnorderedMap_Find(benchmark::State& state) {
    const size_t N = state.range(0);
    auto keys = generate_random_keys(N);
    
    std::unordered_map<int, int> map;
    
    // データの事前挿入
    for (size_t i = 0; i < N; ++i) {
        map.insert({keys[i], static_cast<int>(i)});
    }
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, static_cast<int>(N - 1));
    
    for (auto _ : state) {
        int found_count = 0;
        for (int i = 0; i < 1000; ++i) {
            auto it = map.find(keys[dis(gen)]);
            if (it != map.end()) {
                found_count++;
            }
        }
        benchmark::DoNotOptimize(found_count);
    }
    state.SetItemsProcessed(state.iterations() * 1000);
}

// operator[]性能比較
static void BM_BluestlHashMap_OperatorAccess(benchmark::State& state) {
    const size_t N = state.range(0);
    auto keys = generate_random_keys(N);
    
    for (auto _ : state) {
        bluestl::allocator<bluestl::pair<int, int>> alloc;
        bluestl::hash_map<int, int, bluestl::allocator<bluestl::pair<int, int>>> map(alloc);
        
        for (size_t i = 0; i < N; ++i) {
            map[keys[i]] = static_cast<int>(i);
        }
        benchmark::DoNotOptimize(map.size());
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_StdUnorderedMap_OperatorAccess(benchmark::State& state) {
    const size_t N = state.range(0);
    auto keys = generate_random_keys(N);
    
    for (auto _ : state) {
        std::unordered_map<int, int> map;
        
        for (size_t i = 0; i < N; ++i) {
            map[keys[i]] = static_cast<int>(i);
        }
        benchmark::DoNotOptimize(map.size());
    }
    state.SetItemsProcessed(state.iterations() * N);
}

// 削除性能比較
static void BM_BluestlHashMap_Erase(benchmark::State& state) {
    const size_t N = state.range(0);
    auto keys = generate_random_keys(N);
    
    for (auto _ : state) {
        bluestl::allocator<bluestl::pair<int, int>> alloc;
        bluestl::hash_map<int, int, bluestl::allocator<bluestl::pair<int, int>>> map(alloc);
        
        // データの挿入
        for (size_t i = 0; i < N; ++i) {
            map.insert({keys[i], static_cast<int>(i)});
        }
        
        // 半分を削除
        for (size_t i = 0; i < N / 2; ++i) {
            map.erase(keys[i]);
        }
        benchmark::DoNotOptimize(map.size());
    }
    state.SetItemsProcessed(state.iterations() * (N / 2));
}

static void BM_StdUnorderedMap_Erase(benchmark::State& state) {
    const size_t N = state.range(0);
    auto keys = generate_random_keys(N);
    
    for (auto _ : state) {
        std::unordered_map<int, int> map;
        
        // データの挿入
        for (size_t i = 0; i < N; ++i) {
            map.insert({keys[i], static_cast<int>(i)});
        }
        
        // 半分を削除
        for (size_t i = 0; i < N / 2; ++i) {
            map.erase(keys[i]);
        }
        benchmark::DoNotOptimize(map.size());
    }
    state.SetItemsProcessed(state.iterations() * (N / 2));
}

// イテレータ性能比較
static void BM_BluestlHashMap_Iterator(benchmark::State& state) {
    const size_t N = state.range(0);
    auto keys = generate_random_keys(N);
    
    bluestl::allocator<bluestl::pair<int, int>> alloc;
    bluestl::hash_map<int, int, bluestl::allocator<bluestl::pair<int, int>>> map(alloc);
    
    // データの事前挿入
    for (size_t i = 0; i < N; ++i) {
        map.insert({keys[i], static_cast<int>(i)});
    }
    
    for (auto _ : state) {
        int sum = 0;
        for (auto it = map.begin(); it != map.end(); ++it) {
            sum += it->second;
        }
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_StdUnorderedMap_Iterator(benchmark::State& state) {
    const size_t N = state.range(0);
    auto keys = generate_random_keys(N);
    
    std::unordered_map<int, int> map;
    
    // データの事前挿入
    for (size_t i = 0; i < N; ++i) {
        map.insert({keys[i], static_cast<int>(i)});
    }
    
    for (auto _ : state) {
        int sum = 0;
        for (auto it = map.begin(); it != map.end(); ++it) {
            sum += it->second;
        }
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

// 文字列キーでの性能比較
static void BM_BluestlHashMap_StringKeys(benchmark::State& state) {
    const size_t N = state.range(0);
    auto keys = generate_string_keys(N);
    
    for (auto _ : state) {
        bluestl::allocator<bluestl::pair<std::string, int>> alloc;
        bluestl::hash_map<std::string, int, bluestl::allocator<bluestl::pair<std::string, int>>> map(alloc);
        
        for (size_t i = 0; i < N; ++i) {
            map.insert({keys[i], static_cast<int>(i)});
        }
        benchmark::DoNotOptimize(map.size());
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_StdUnorderedMap_StringKeys(benchmark::State& state) {
    const size_t N = state.range(0);
    auto keys = generate_string_keys(N);
    
    for (auto _ : state) {
        std::unordered_map<std::string, int> map;
        
        for (size_t i = 0; i < N; ++i) {
            map.insert({keys[i], static_cast<int>(i)});
        }
        benchmark::DoNotOptimize(map.size());
    }
    state.SetItemsProcessed(state.iterations() * N);
}

// 負荷率の影響テスト
static void BM_BluestlHashMap_LoadFactor(benchmark::State& state) {
    const size_t N = state.range(0);
    auto keys = generate_random_keys(N);
    
    for (auto _ : state) {
        bluestl::allocator<bluestl::pair<int, int>> alloc;
        bluestl::hash_map<int, int, bluestl::allocator<bluestl::pair<int, int>>> map(alloc);
        
        // 予約なしで挿入（負荷率が高くなる）
        for (size_t i = 0; i < N; ++i) {
            map.insert({keys[i], static_cast<int>(i)});
        }
        
        // 負荷率を確認
        float load_factor = map.load_factor();
        benchmark::DoNotOptimize(load_factor);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

// reserve済みでの性能
static void BM_BluestlHashMap_WithReserve(benchmark::State& state) {
    const size_t N = state.range(0);
    auto keys = generate_random_keys(N);
    
    for (auto _ : state) {
        bluestl::allocator<bluestl::pair<int, int>> alloc;
        bluestl::hash_map<int, int, bluestl::allocator<bluestl::pair<int, int>>> map(alloc);
        
        map.reserve(N);  // 事前に容量を確保
        
        for (size_t i = 0; i < N; ++i) {
            map.insert({keys[i], static_cast<int>(i)});
        }
        benchmark::DoNotOptimize(map.size());
    }
    state.SetItemsProcessed(state.iterations() * N);
}

// ベンチマーク登録
BENCHMARK(BM_BluestlHashMap_Insert)->Range(1000, 100000);
BENCHMARK(BM_StdUnorderedMap_Insert)->Range(1000, 100000);

BENCHMARK(BM_BluestlHashMap_Find)->Range(1000, 100000);
BENCHMARK(BM_StdUnorderedMap_Find)->Range(1000, 100000);

BENCHMARK(BM_BluestlHashMap_OperatorAccess)->Range(1000, 100000);
BENCHMARK(BM_StdUnorderedMap_OperatorAccess)->Range(1000, 100000);

BENCHMARK(BM_BluestlHashMap_Erase)->Range(1000, 50000);
BENCHMARK(BM_StdUnorderedMap_Erase)->Range(1000, 50000);

BENCHMARK(BM_BluestlHashMap_Iterator)->Range(1000, 100000);
BENCHMARK(BM_StdUnorderedMap_Iterator)->Range(1000, 100000);

BENCHMARK(BM_BluestlHashMap_StringKeys)->Range(1000, 50000);
BENCHMARK(BM_StdUnorderedMap_StringKeys)->Range(1000, 50000);

BENCHMARK(BM_BluestlHashMap_LoadFactor)->Range(1000, 100000);
BENCHMARK(BM_BluestlHashMap_WithReserve)->Range(1000, 100000);

BENCHMARK_MAIN();