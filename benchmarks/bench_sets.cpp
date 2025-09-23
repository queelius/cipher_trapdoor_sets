#include <cipher_trapdoor_sets/cts.hpp>
#include <benchmark/benchmark.h>
#include <random>
#include <string>
#include <vector>

using namespace cts;

static void BM_BooleanSetCreation(benchmark::State& state) {
    std::string_view secret = "benchmark-secret-key";
    sets::boolean_set_factory<std::string> factory(secret);

    std::vector<std::string> items;
    for (int i = 0; i < state.range(0); ++i) {
        items.push_back("item-" + std::to_string(i));
    }

    for (auto _ : state) {
        auto set = factory.from_collection(items);
        benchmark::DoNotOptimize(set);
    }

    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_BooleanSetCreation)->Range(10, 10000);

static void BM_BooleanSetUnion(benchmark::State& state) {
    std::string_view secret = "benchmark-secret-key";
    sets::boolean_set_factory<std::string> factory(secret);

    std::vector<std::string> items1, items2;
    for (int i = 0; i < state.range(0); ++i) {
        items1.push_back("set1-" + std::to_string(i));
        items2.push_back("set2-" + std::to_string(i));
    }

    auto set1 = factory.from_collection(items1);
    auto set2 = factory.from_collection(items2);

    for (auto _ : state) {
        auto union_set = set1 | set2;
        benchmark::DoNotOptimize(union_set);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_BooleanSetUnion)->Range(10, 10000);

static void BM_BooleanSetIntersection(benchmark::State& state) {
    std::string_view secret = "benchmark-secret-key";
    sets::boolean_set_factory<std::string> factory(secret);

    std::vector<std::string> items1, items2;
    for (int i = 0; i < state.range(0); ++i) {
        items1.push_back("item-" + std::to_string(i));
        items2.push_back("item-" + std::to_string(i / 2)); // 50% overlap
    }

    auto set1 = factory.from_collection(items1);
    auto set2 = factory.from_collection(items2);

    for (auto _ : state) {
        auto intersection = set1 & set2;
        benchmark::DoNotOptimize(intersection);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_BooleanSetIntersection)->Range(10, 10000);

static void BM_BooleanSetMembership(benchmark::State& state) {
    std::string_view secret = "benchmark-secret-key";
    sets::boolean_set_factory<std::string> factory(secret);
    trapdoor_factory<32> td_factory(secret);

    std::vector<std::string> items;
    for (int i = 0; i < state.range(0); ++i) {
        items.push_back("item-" + std::to_string(i));
    }

    auto set = factory.from_collection(items);
    auto td = td_factory.create(std::string("item-50"));

    for (auto _ : state) {
        auto contains = set.contains(td);
        benchmark::DoNotOptimize(contains);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_BooleanSetMembership)->Range(100, 100000);

static void BM_SymmetricDifferenceSet(benchmark::State& state) {
    std::string_view secret = "benchmark-secret-key";
    sets::symmetric_difference_set_factory<std::string> factory(secret);

    std::vector<std::string> items1, items2;
    for (int i = 0; i < state.range(0); ++i) {
        items1.push_back("set1-" + std::to_string(i));
        items2.push_back("set2-" + std::to_string(i));
    }

    auto set1 = factory.from_unique(items1);
    auto set2 = factory.from_unique(items2);

    for (auto _ : state) {
        auto sym_diff = set1 ^ set2;
        benchmark::DoNotOptimize(sym_diff);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_SymmetricDifferenceSet)->Range(10, 10000);

BENCHMARK_MAIN();