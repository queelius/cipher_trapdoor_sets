#include <cipher_trapdoor_sets/cts.hpp>
#include <benchmark/benchmark.h>
#include <random>
#include <string>

using namespace cts;

static void BM_TrapdoorCreation(benchmark::State& state) {
    std::string_view secret = "benchmark-secret-key";
    trapdoor_factory<32> factory(secret);
    std::string value = "test-value-" + std::to_string(state.range(0));

    for (auto _ : state) {
        auto td = factory.create(value);
        benchmark::DoNotOptimize(td);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_TrapdoorCreation)->Range(1, 1000000);

static void BM_TrapdoorEquality(benchmark::State& state) {
    std::string_view secret = "benchmark-secret-key";
    trapdoor_factory<32> factory(secret);

    auto td1 = factory.create(std::string("value1"));
    auto td2 = factory.create(std::string("value2"));

    for (auto _ : state) {
        auto result = (td1 == td2);
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_TrapdoorEquality);

template <std::size_t HashSize>
static void BM_TrapdoorHashSize(benchmark::State& state) {
    std::string_view secret = "benchmark-secret-key";
    trapdoor_factory<HashSize> factory(secret);
    std::string value = "test-value";

    for (auto _ : state) {
        auto td = factory.create(value);
        benchmark::DoNotOptimize(td);
    }

    state.SetBytesProcessed(state.iterations() * HashSize);
}
BENCHMARK_TEMPLATE(BM_TrapdoorHashSize, 16);
BENCHMARK_TEMPLATE(BM_TrapdoorHashSize, 32);
BENCHMARK_TEMPLATE(BM_TrapdoorHashSize, 64);
BENCHMARK_TEMPLATE(BM_TrapdoorHashSize, 128);

static void BM_BatchTrapdoorCreation(benchmark::State& state) {
    std::string_view secret = "benchmark-secret-key";
    trapdoor_factory<32> factory(secret);

    std::vector<std::string> values;
    for (int i = 0; i < state.range(0); ++i) {
        values.push_back("value-" + std::to_string(i));
    }

    for (auto _ : state) {
        std::vector<trapdoor<std::string, 32>> trapdoors;
        trapdoors.reserve(values.size());
        for (const auto& v : values) {
            trapdoors.push_back(factory.create(v));
        }
        benchmark::DoNotOptimize(trapdoors);
    }

    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_BatchTrapdoorCreation)->Range(10, 10000);

BENCHMARK_MAIN();