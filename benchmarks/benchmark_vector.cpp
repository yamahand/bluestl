/**
 * @file benchmark_vector.cpp
 * @brief bluestl::vector のベンチマークテスト
 * 
 * Google Benchmarkフレームワークを使用したパフォーマンステスト
 */

#include <benchmark/benchmark.h>
#include "bluestl/vector.h"
#include <vector>
#include <random>
#include <algorithm>

// ベンチマーク用のランダムデータ生成
static std::vector<int> generate_random_data(size_t size) {
    std::vector<int> data;
    data.reserve(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100000);
    
    for (size_t i = 0; i < size; ++i) {
        data.push_back(dis(gen));
    }
    return data;
}

// bluestl::vector vs std::vector: push_back性能比較
static void BM_BluestlVector_PushBack(benchmark::State& state) {
    const size_t N = state.range(0);
    auto data = generate_random_data(N);
    
    for (auto _ : state) {
        bluestl::vector<int> v;
        for (size_t i = 0; i < N; ++i) {
            v.push_back(data[i]);
        }
        benchmark::DoNotOptimize(v.data());
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_StdVector_PushBack(benchmark::State& state) {
    const size_t N = state.range(0);
    auto data = generate_random_data(N);
    
    for (auto _ : state) {
        std::vector<int> v;
        for (size_t i = 0; i < N; ++i) {
            v.push_back(data[i]);
        }
        benchmark::DoNotOptimize(v.data());
    }
    state.SetItemsProcessed(state.iterations() * N);
}

// reserve済みでのpush_back性能比較
static void BM_BluestlVector_PushBackReserved(benchmark::State& state) {
    const size_t N = state.range(0);
    auto data = generate_random_data(N);
    
    for (auto _ : state) {
        bluestl::vector<int> v;
        v.reserve(N);
        for (size_t i = 0; i < N; ++i) {
            v.push_back(data[i]);
        }
        benchmark::DoNotOptimize(v.data());
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_StdVector_PushBackReserved(benchmark::State& state) {
    const size_t N = state.range(0);
    auto data = generate_random_data(N);
    
    for (auto _ : state) {
        std::vector<int> v;
        v.reserve(N);
        for (size_t i = 0; i < N; ++i) {
            v.push_back(data[i]);
        }
        benchmark::DoNotOptimize(v.data());
    }
    state.SetItemsProcessed(state.iterations() * N);
}

// ランダムアクセス性能比較
static void BM_BluestlVector_RandomAccess(benchmark::State& state) {
    const size_t N = state.range(0);
    bluestl::vector<int> v;
    for (size_t i = 0; i < N; ++i) {
        v.push_back(static_cast<int>(i));
    }
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, static_cast<int>(N - 1));
    
    for (auto _ : state) {
        int sum = 0;
        for (int i = 0; i < 1000; ++i) {
            sum += v[dis(gen)];
        }
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * 1000);
}

static void BM_StdVector_RandomAccess(benchmark::State& state) {
    const size_t N = state.range(0);
    std::vector<int> v;
    for (size_t i = 0; i < N; ++i) {
        v.push_back(static_cast<int>(i));
    }
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, static_cast<int>(N - 1));
    
    for (auto _ : state) {
        int sum = 0;
        for (int i = 0; i < 1000; ++i) {
            sum += v[dis(gen)];
        }
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * 1000);
}

// イテレータ性能比較
static void BM_BluestlVector_Iterator(benchmark::State& state) {
    const size_t N = state.range(0);
    bluestl::vector<int> v;
    for (size_t i = 0; i < N; ++i) {
        v.push_back(static_cast<int>(i));
    }
    
    for (auto _ : state) {
        int sum = 0;
        for (auto it = v.begin(); it != v.end(); ++it) {
            sum += *it;
        }
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_StdVector_Iterator(benchmark::State& state) {
    const size_t N = state.range(0);
    std::vector<int> v;
    for (size_t i = 0; i < N; ++i) {
        v.push_back(static_cast<int>(i));
    }
    
    for (auto _ : state) {
        int sum = 0;
        for (auto it = v.begin(); it != v.end(); ++it) {
            sum += *it;
        }
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

// insert性能比較
static void BM_BluestlVector_Insert(benchmark::State& state) {
    const size_t N = state.range(0);
    
    for (auto _ : state) {
        bluestl::vector<int> v;
        for (size_t i = 0; i < N; ++i) {
            v.insert(v.begin(), static_cast<int>(i));
        }
        benchmark::DoNotOptimize(v.data());
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_StdVector_Insert(benchmark::State& state) {
    const size_t N = state.range(0);
    
    for (auto _ : state) {
        std::vector<int> v;
        for (size_t i = 0; i < N; ++i) {
            v.insert(v.begin(), static_cast<int>(i));
        }
        benchmark::DoNotOptimize(v.data());
    }
    state.SetItemsProcessed(state.iterations() * N);
}

// emplace_back vs push_back性能比較
static void BM_BluestlVector_EmplaceBack(benchmark::State& state) {
    const size_t N = state.range(0);
    
    for (auto _ : state) {
        bluestl::vector<std::string> v;
        v.reserve(N);
        for (size_t i = 0; i < N; ++i) {
            v.emplace_back("test_string_" + std::to_string(i));
        }
        benchmark::DoNotOptimize(v.data());
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_BluestlVector_PushBackString(benchmark::State& state) {
    const size_t N = state.range(0);
    
    for (auto _ : state) {
        bluestl::vector<std::string> v;
        v.reserve(N);
        for (size_t i = 0; i < N; ++i) {
            std::string s = "test_string_" + std::to_string(i);
            v.push_back(s);
        }
        benchmark::DoNotOptimize(v.data());
    }
    state.SetItemsProcessed(state.iterations() * N);
}

// メモリ使用量テスト（間接的）
static void BM_BluestlVector_MemoryUsage(benchmark::State& state) {
    const size_t N = state.range(0);
    
    for (auto _ : state) {
        bluestl::vector<int> v;
        for (size_t i = 0; i < N; ++i) {
            v.push_back(static_cast<int>(i));
        }
        // 容量をチェック
        benchmark::DoNotOptimize(v.capacity());
        benchmark::DoNotOptimize(v.size());
    }
}

// ベンチマーク登録とパラメータ設定
BENCHMARK(BM_BluestlVector_PushBack)->Range(1000, 100000);
BENCHMARK(BM_StdVector_PushBack)->Range(1000, 100000);

BENCHMARK(BM_BluestlVector_PushBackReserved)->Range(1000, 100000);
BENCHMARK(BM_StdVector_PushBackReserved)->Range(1000, 100000);

BENCHMARK(BM_BluestlVector_RandomAccess)->Range(1000, 100000);
BENCHMARK(BM_StdVector_RandomAccess)->Range(1000, 100000);

BENCHMARK(BM_BluestlVector_Iterator)->Range(1000, 100000);
BENCHMARK(BM_StdVector_Iterator)->Range(1000, 100000);

BENCHMARK(BM_BluestlVector_Insert)->Range(100, 1000);
BENCHMARK(BM_StdVector_Insert)->Range(100, 1000);

BENCHMARK(BM_BluestlVector_EmplaceBack)->Range(1000, 10000);
BENCHMARK(BM_BluestlVector_PushBackString)->Range(1000, 10000);

BENCHMARK(BM_BluestlVector_MemoryUsage)->Range(1000, 100000);

BENCHMARK_MAIN();