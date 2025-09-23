#include <cipher_trapdoor_sets/cts.hpp>
#include <benchmark/benchmark.h>
#include <random>
#include <string>
#include <vector>

using namespace cts;

// Benchmark cardinality estimation
static void BM_CardinalityEstimation(benchmark::State& state) {
    std::string_view secret = "benchmark-secret-key";
    operations::cardinality::hyperloglog_estimator<32> estimator;
    trapdoor_factory<32> factory(secret);

    std::vector<trapdoor<std::string, 32>> trapdoors;
    for (int i = 0; i < state.range(0); ++i) {
        trapdoors.push_back(factory.create("value-" + std::to_string(i)));
    }

    for (auto _ : state) {
        operations::cardinality::hyperloglog_estimator<32> local_estimator;
        for (const auto& td : trapdoors) {
            local_estimator.add(td);
        }
        auto estimate = local_estimator.estimate();
        benchmark::DoNotOptimize(estimate);
    }

    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_CardinalityEstimation)->Range(100, 100000);

// Benchmark homomorphic addition
static void BM_HomomorphicAddition(benchmark::State& state) {
    std::string_view secret = "benchmark-secret-key";
    operations::homomorphic::additive_trapdoor<int> trapdoor1(100, secret);
    operations::homomorphic::additive_trapdoor<int> trapdoor2(200, secret);

    for (auto _ : state) {
        auto sum = trapdoor1 + trapdoor2;
        benchmark::DoNotOptimize(sum);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_HomomorphicAddition);

// Benchmark secure aggregation
static void BM_SecureAggregation(benchmark::State& state) {
    std::string_view secret = "benchmark-secret-key";
    operations::homomorphic::secure_aggregator<double> aggregator(secret);

    std::vector<double> values;
    for (int i = 0; i < state.range(0); ++i) {
        values.push_back(i * 1.5);
    }

    for (auto _ : state) {
        operations::homomorphic::secure_aggregator<double> local_agg(secret);
        for (double v : values) {
            local_agg.add(v);
        }
        auto sum = local_agg.sum();
        benchmark::DoNotOptimize(sum);
    }

    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_SecureAggregation)->Range(10, 10000);

// Benchmark Jaccard similarity
static void BM_JaccardSimilarity(benchmark::State& state) {
    std::string_view secret = "benchmark-secret-key";
    sets::boolean_set_factory<std::string> factory(secret);

    std::vector<std::string> items1, items2;
    for (int i = 0; i < state.range(0); ++i) {
        items1.push_back("item-" + std::to_string(i));
        items2.push_back("item-" + std::to_string(i * 2)); // Some overlap
    }

    auto set1 = factory.from_collection(items1);
    auto set2 = factory.from_collection(items2);

    for (auto _ : state) {
        auto similarity = operations::similarity::jaccard_similarity(set1, set2);
        benchmark::DoNotOptimize(similarity);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_JaccardSimilarity)->Range(100, 10000);

// Benchmark MinHash signature generation
static void BM_MinHashSignature(benchmark::State& state) {
    std::string_view secret = "benchmark-secret-key";
    trapdoor_factory<32> factory(secret);

    std::vector<trapdoor<std::string, 32>> items;
    for (int i = 0; i < state.range(0); ++i) {
        items.push_back(factory.create("item-" + std::to_string(i)));
    }

    for (auto _ : state) {
        auto signature = operations::similarity::minhash_signature(items);
        benchmark::DoNotOptimize(signature);
    }

    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_MinHashSignature)->Range(100, 10000);

// Benchmark LSH indexing
static void BM_LSHIndexing(benchmark::State& state) {
    std::string_view secret = "benchmark-secret-key";
    trapdoor_factory<32> factory(secret);
    operations::similarity::lsh_index<std::string> index(4, 128);

    std::vector<std::string> items;
    for (int i = 0; i < state.range(0); ++i) {
        items.push_back("item-" + std::to_string(i));
    }

    for (auto _ : state) {
        operations::similarity::lsh_index<std::string> local_index(4, 128);
        for (const auto& item : items) {
            auto td = factory.create(item);
            local_index.add(item, td);
        }
        benchmark::DoNotOptimize(local_index);
    }

    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_LSHIndexing)->Range(100, 10000);

// Benchmark differential privacy noise
static void BM_DifferentialPrivacy(benchmark::State& state) {
    std::string_view secret = "benchmark-secret-key";
    operations::analytics::frequency_analyzer<std::string> analyzer(secret, 1.0);

    std::vector<std::string> items;
    for (int i = 0; i < state.range(0); ++i) {
        items.push_back("item-" + std::to_string(i % 10)); // 10 unique items
    }

    for (auto _ : state) {
        operations::analytics::frequency_analyzer<std::string> local_analyzer(secret, 1.0);
        for (const auto& item : items) {
            local_analyzer.add(item);
        }
        auto freq = local_analyzer.frequency_of("item-5");
        benchmark::DoNotOptimize(freq);
    }

    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_DifferentialPrivacy)->Range(100, 10000);

// Benchmark key derivation
static void BM_KeyDerivation(benchmark::State& state) {
    std::string password = "benchmark-password";
    std::string salt = "benchmark-salt";

    for (auto _ : state) {
        auto key = key_management::derive_key(password, salt, state.range(0));
        benchmark::DoNotOptimize(key);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_KeyDerivation)->Arg(1000)->Arg(10000)->Arg(100000);

// Benchmark threshold secret sharing
static void BM_ThresholdSecretSharing(benchmark::State& state) {
    std::string_view secret = "benchmark-secret-key";
    trapdoor_factory<32> factory(secret);
    auto td = factory.create(std::string("test-value"));

    for (auto _ : state) {
        auto shares = operations::homomorphic::threshold_share(td, 3, 5);
        benchmark::DoNotOptimize(shares);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_ThresholdSecretSharing);

BENCHMARK_MAIN();