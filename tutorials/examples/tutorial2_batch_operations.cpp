// Tutorial 2: Batch Operations and Performance
// Compile: g++ -std=c++17 -I../../include tutorial2_batch_operations.cpp -o batch_ops
// Run: ./batch_ops

#include <cipher_trapdoor_sets/cts.hpp>
#include <cipher_trapdoor_sets/operations/batch_ops.hpp>
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <iomanip>

using namespace cts;
using namespace cts::operations;

// Helper function to generate random strings
std::vector<std::string> generate_random_strings(size_t count, size_t length = 10) {
    std::vector<std::string> result;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis('a', 'z');

    for (size_t i = 0; i < count; ++i) {
        std::string str;
        for (size_t j = 0; j < length; ++j) {
            str += static_cast<char>(dis(gen));
        }
        result.push_back(str);
    }
    return result;
}

// Timer class for benchmarking
class Timer {
    std::chrono::high_resolution_clock::time_point start_;
public:
    Timer() : start_(std::chrono::high_resolution_clock::now()) {}

    double elapsed_ms() const {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - start_).count();
    }
};

void demonstrate_basic_batch() {
    std::cout << "═══════════════════════════════════════════════════\n";
    std::cout << "     BASIC BATCH OPERATIONS                       \n";
    std::cout << "═══════════════════════════════════════════════════\n\n";

    // Sample data
    std::vector<std::string> users = {
        "alice@example.com",
        "bob@example.com",
        "charlie@example.com",
        "diana@example.com",
        "eve@example.com"
    };

    std::cout << "Creating trapdoors for " << users.size() << " users...\n";

    // Batch creation
    auto batch_trapdoors = batch_operations::batch_create_trapdoors(
        users, "batch-demo-key-2025");

    std::cout << "✓ Created " << batch_trapdoors.size() << " trapdoors\n\n";

    // Display trapdoor info
    for (size_t i = 0; i < std::min(size_t(3), users.size()); ++i) {
        std::cout << "User " << i << ": " << users[i].substr(0, 5) << "***\n";
        std::cout << "  Hash (first 8 bytes): ";
        for (size_t j = 0; j < 8; ++j) {
            printf("%02x", batch_trapdoors[i].hash().data[j]);
        }
        std::cout << "...\n";
    }
    std::cout << "\n";
}

void benchmark_batch_vs_individual() {
    std::cout << "═══════════════════════════════════════════════════\n";
    std::cout << "     PERFORMANCE: BATCH VS INDIVIDUAL             \n";
    std::cout << "═══════════════════════════════════════════════════\n\n";

    std::vector<size_t> test_sizes = {100, 1000, 10000};

    for (size_t size : test_sizes) {
        std::cout << "Testing with " << size << " items:\n";
        std::cout << "─────────────────────────────\n";

        // Generate test data
        auto test_data = generate_random_strings(size);

        // Individual creation
        {
            Timer timer;
            trapdoor_factory<32> factory("perf-test-key");
            std::vector<trapdoor<std::string, 32>> individual;

            for (const auto& item : test_data) {
                individual.push_back(factory.create(item));
            }

            double time = timer.elapsed_ms();
            std::cout << "  Individual: " << std::fixed << std::setprecision(2)
                      << time << " ms";
            std::cout << " (" << (time / size * 1000) << " µs/item)\n";
        }

        // Batch creation
        {
            Timer timer;
            auto batch = batch_operations::batch_create_trapdoors(
                test_data, "perf-test-key");

            double time = timer.elapsed_ms();
            std::cout << "  Batch:      " << std::fixed << std::setprecision(2)
                      << time << " ms";
            std::cout << " (" << (time / size * 1000) << " µs/item)\n";
        }

        std::cout << "\n";
    }
}

void demonstrate_batch_comparison() {
    std::cout << "═══════════════════════════════════════════════════\n";
    std::cout << "     BATCH COMPARISON OPERATIONS                  \n";
    std::cout << "═══════════════════════════════════════════════════\n\n";

    // Two datasets with some overlap
    std::vector<std::string> dataset_a = {
        "item_001", "item_002", "item_003", "item_004", "item_005"
    };

    std::vector<std::string> dataset_b = {
        "item_001", "item_002", "item_006", "item_007", "item_005"
    };

    std::cout << "Dataset A: ";
    for (const auto& item : dataset_a) std::cout << item << " ";
    std::cout << "\n";

    std::cout << "Dataset B: ";
    for (const auto& item : dataset_b) std::cout << item << " ";
    std::cout << "\n\n";

    // Create trapdoors
    auto td_a = batch_operations::batch_create_trapdoors(dataset_a, "compare-key");
    auto td_b = batch_operations::batch_create_trapdoors(dataset_b, "compare-key");

    // Batch comparison
    auto results = batch_operations::batch_equals(td_a, td_b);

    std::cout << "Comparison Results:\n";
    std::cout << "─────────────────────\n";

    int matches = 0;
    for (size_t i = 0; i < results.size(); ++i) {
        std::cout << "  Position " << i << ": ";
        if (results[i].value()) {
            std::cout << "✓ MATCH";
            matches++;
        } else {
            std::cout << "✗ DIFFERENT";
        }
        std::cout << " (FPR: " << std::scientific << std::setprecision(2)
                  << results[i].false_positive_rate() << ")\n";
    }

    std::cout << "\nSummary: " << matches << "/" << results.size() << " matches\n\n";
}

void demonstrate_batch_operations_with_types() {
    std::cout << "═══════════════════════════════════════════════════\n";
    std::cout << "     BATCH OPERATIONS WITH DIFFERENT TYPES        \n";
    std::cout << "═══════════════════════════════════════════════════\n\n";

    trapdoor_factory<32> factory("type-demo-key");

    // Integer batch
    std::cout << "Integer batch:\n";
    std::vector<int> integers;
    for (int i = 0; i < 1000; ++i) {
        integers.push_back(i);
    }

    {
        Timer timer;
        auto int_batch = batch_operations::batch_create_trapdoors(integers, "type-demo-key");
        std::cout << "  Created " << int_batch.size() << " integer trapdoors in "
                  << timer.elapsed_ms() << " ms\n";
    }

    // Double batch (with precision handling)
    std::cout << "\nDouble batch (with rounding):\n";
    std::vector<double> doubles;
    for (int i = 0; i < 1000; ++i) {
        doubles.push_back(i * 3.14159);
    }

    {
        Timer timer;
        // Round to avoid floating point issues
        std::vector<double> rounded_doubles;
        for (auto d : doubles) {
            rounded_doubles.push_back(std::round(d * 1000000) / 1000000);
        }
        auto double_batch = batch_operations::batch_create_trapdoors(
            rounded_doubles, "type-demo-key");
        std::cout << "  Created " << double_batch.size() << " double trapdoors in "
                  << timer.elapsed_ms() << " ms\n";
    }

    // Custom struct batch
    struct UserRecord {
        int id;
        std::string name;

        bool operator==(const UserRecord& other) const {
            return id == other.id && name == other.name;
        }
    };

    // Provide hash for custom type
    struct UserRecordHash {
        size_t operator()(const UserRecord& user) const {
            return std::hash<int>{}(user.id) ^
                   (std::hash<std::string>{}(user.name) << 1);
        }
    };

    std::cout << "\nCustom struct batch:\n";
    std::cout << "  Note: Requires custom hash function\n";
    std::cout << "  See code for implementation details\n\n";
}

void demonstrate_parallel_batch() {
    std::cout << "═══════════════════════════════════════════════════\n";
    std::cout << "     PARALLEL BATCH PROCESSING                    \n";
    std::cout << "═══════════════════════════════════════════════════\n\n";

    const size_t large_size = 100000;
    auto large_dataset = generate_random_strings(large_size, 20);

    std::cout << "Processing " << large_size << " items...\n\n";

    // Sequential processing
    {
        Timer timer;
        auto sequential = batch_operations::batch_create_trapdoors(
            large_dataset, "parallel-key");
        std::cout << "Sequential: " << timer.elapsed_ms() << " ms\n";
    }

    // Simulated parallel processing (chunking)
    {
        Timer timer;
        const size_t num_chunks = 4;
        const size_t chunk_size = large_size / num_chunks;

        std::vector<std::vector<trapdoor<std::string, 32>>> chunks;

        for (size_t i = 0; i < num_chunks; ++i) {
            size_t start = i * chunk_size;
            size_t end = (i == num_chunks - 1) ? large_size : start + chunk_size;

            std::vector<std::string> chunk_data(
                large_dataset.begin() + start,
                large_dataset.begin() + end);

            chunks.push_back(batch_operations::batch_create_trapdoors(
                chunk_data, "parallel-key"));
        }

        // Merge results
        std::vector<trapdoor<std::string, 32>> parallel_result;
        for (const auto& chunk : chunks) {
            parallel_result.insert(parallel_result.end(),
                                   chunk.begin(), chunk.end());
        }

        std::cout << "Chunked (" << num_chunks << " chunks): "
                  << timer.elapsed_ms() << " ms\n";
        std::cout << "  Note: True parallelization would use std::async or threads\n";
    }

    std::cout << "\n";
}

void demonstrate_batch_statistics() {
    std::cout << "═══════════════════════════════════════════════════\n";
    std::cout << "     BATCH STATISTICS AND ANALYSIS                \n";
    std::cout << "═══════════════════════════════════════════════════\n\n";

    // Create dataset with intentional duplicates
    std::vector<std::string> dataset;
    for (int i = 0; i < 100; ++i) {
        dataset.push_back("unique_" + std::to_string(i));
    }
    // Add duplicates
    for (int i = 0; i < 20; ++i) {
        dataset.push_back("unique_" + std::to_string(i % 10));  // Duplicates
    }

    std::cout << "Dataset: " << dataset.size() << " total items\n";
    std::cout << "  Expected: 100 unique, 20 duplicates\n\n";

    // Create trapdoors
    auto trapdoors = batch_operations::batch_create_trapdoors(
        dataset, "stats-key");

    // Find unique trapdoors
    std::set<size_t> unique_hashes;
    for (const auto& td : trapdoors) {
        // Use first 8 bytes as simplified identifier
        size_t hash_id = 0;
        for (size_t i = 0; i < 8; ++i) {
            hash_id = (hash_id << 8) | td.hash().data[i];
        }
        unique_hashes.insert(hash_id);
    }

    std::cout << "Analysis Results:\n";
    std::cout << "─────────────────\n";
    std::cout << "  Total trapdoors: " << trapdoors.size() << "\n";
    std::cout << "  Unique patterns: " << unique_hashes.size() << "\n";
    std::cout << "  Duplicate count: " << (trapdoors.size() - unique_hashes.size()) << "\n";

    // Memory usage
    size_t memory_per_trapdoor = sizeof(trapdoor<std::string, 32>);
    size_t total_memory = memory_per_trapdoor * trapdoors.size();

    std::cout << "\nMemory Usage:\n";
    std::cout << "  Per trapdoor: " << memory_per_trapdoor << " bytes\n";
    std::cout << "  Total: " << total_memory << " bytes (";
    if (total_memory < 1024) {
        std::cout << total_memory << " B)\n";
    } else if (total_memory < 1024 * 1024) {
        std::cout << total_memory / 1024.0 << " KB)\n";
    } else {
        std::cout << total_memory / (1024.0 * 1024.0) << " MB)\n";
    }

    std::cout << "\n";
}

int main() {
    std::cout << "\n╔═══════════════════════════════════════════════════╗\n";
    std::cout << "║      BATCH OPERATIONS TUTORIAL                   ║\n";
    std::cout << "╚═══════════════════════════════════════════════════╝\n\n";

    // Run demonstrations
    demonstrate_basic_batch();
    benchmark_batch_vs_individual();
    demonstrate_batch_comparison();
    demonstrate_batch_operations_with_types();
    demonstrate_parallel_batch();
    demonstrate_batch_statistics();

    std::cout << "╔═══════════════════════════════════════════════════╗\n";
    std::cout << "║                 KEY TAKEAWAYS                    ║\n";
    std::cout << "╚═══════════════════════════════════════════════════╝\n\n";

    std::cout << "✓ Batch operations are significantly faster\n";
    std::cout << "✓ Support for various data types\n";
    std::cout << "✓ Efficient comparison operations\n";
    std::cout << "✓ Can be parallelized for large datasets\n";
    std::cout << "✓ Memory efficient with constant overhead\n\n";

    std::cout << "Next: Try Tutorial 3 for Boolean set operations\n\n";

    return 0;
}