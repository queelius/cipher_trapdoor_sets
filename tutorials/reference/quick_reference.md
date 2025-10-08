# Cipher Trapdoor Sets - Quick Reference Guide

## Include Headers

```cpp
#include <cipher_trapdoor_sets/cts.hpp>                    // Core functionality
#include <cipher_trapdoor_sets/sets/boolean_set.hpp>       // Boolean set operations
#include <cipher_trapdoor_sets/operations/batch_ops.hpp>   // Batch operations
#include <cipher_trapdoor_sets/operations/cardinality.hpp> // Cardinality estimation
#include <cipher_trapdoor_sets/operations/similarity.hpp>  // Similarity metrics
```

## Core Types

### Trapdoor Factory

```cpp
// Create factory with secret key
trapdoor_factory<32> factory("secret-key");         // 32 bytes = 256-bit hash
trapdoor_factory<16> small_factory("key");          // 16 bytes = 128-bit hash
trapdoor_factory<64> large_factory("key");          // 64 bytes = 512-bit hash

// Get key fingerprint
size_t fingerprint = factory.key_fingerprint();
```

### Creating Trapdoors

```cpp
// From various types
auto td_string = factory.create(std::string("data"));
auto td_int = factory.create(42);
auto td_double = factory.create(3.14159);

// Custom types (must have std::hash specialization)
struct MyType { int id; std::string name; };
auto td_custom = factory.create(my_instance);
```

### Trapdoor Operations

```cpp
// Comparison
auto result = (td1 == td2);                        // Returns approximate_bool
bool matches = result.value();                     // Get boolean result
double fpr = result.false_positive_rate();         // Get error rate

// Compatibility check
if (td1.compatible_with(td2)) {                    // Check same key
    auto result = td1.equals(td2);
}

// Access hash
auto hash = td.hash();                             // Get hash_value<N>
size_t key_fp = td.key_fingerprint();              // Get key fingerprint
```

## Boolean Set Operations

### Creating Sets

```cpp
using namespace cts::sets;

// Empty set
boolean_set<std::string, 32> empty_set;

// From trapdoor
auto td = factory.create("element");
boolean_set<std::string, 32> single_set(td.hash(), td.key_fingerprint());

// Building from multiple elements
boolean_set<std::string, 32> multi_set;
for (const auto& elem : elements) {
    auto td = factory.create(elem);
    boolean_set<std::string, 32> elem_set(td.hash(), td.key_fingerprint());
    multi_set = multi_set | elem_set;              // Union to add
}
```

### Set Operations

```cpp
// Basic operations
auto union_set = set1 | set2;                      // Union (OR)
auto intersect = set1 & set2;                      // Intersection (AND)
auto diff = set1 - set2;                           // Difference
auto sym_diff = set1 ^ set2;                       // Symmetric difference (XOR)
auto complement = ~set1;                           // Complement (NOT)

// Compound operations
auto result = (set1 | set2) & set3;                // (A ∪ B) ∩ C
auto exclusive = set1 - (set2 | set3);             // A - (B ∪ C)
```

### Membership Testing

```cpp
// Test if element in set
auto td = factory.create("test_element");
auto contains = set.contains(td);                  // Returns approximate_bool

if (contains.value()) {
    std::cout << "Probably in set (FPR: " << contains.false_positive_rate() << ")\n";
}

// Subset testing
auto is_subset = set1.subset_of(set2);             // A ⊆ B
auto is_superset = set2.subset_of(set1);           // B ⊆ A (reverse for superset)
```

## Batch Operations

```cpp
using namespace cts::operations;

// Batch creation
std::vector<std::string> data = {"a", "b", "c", "d"};
auto batch = batch_operations::batch_create_trapdoors(data, "key");

// Batch comparison
auto batch1 = batch_operations::batch_create_trapdoors(data1, "key");
auto batch2 = batch_operations::batch_create_trapdoors(data2, "key");
auto results = batch_operations::batch_equals(batch1, batch2);

// Process results
for (size_t i = 0; i < results.size(); ++i) {
    if (results[i].value()) {
        std::cout << "Position " << i << " matches\n";
    }
}
```

## Cardinality Estimation

```cpp
using namespace cts::operations;

// Estimate set size
cardinality_estimator<32> estimator;
auto estimate = estimator.estimate(boolean_set);

size_t estimated_size = estimate.value();
double error_margin = estimate.error_margin();

std::cout << "Estimated elements: " << estimated_size
          << " ± " << error_margin << "\n";
```

## Similarity Metrics

```cpp
using namespace cts::operations;

// Jaccard similarity
similarity_calculator<32> calc;
auto jaccard = calc.jaccard_similarity(set1, set2);

double similarity = jaccard.value();               // 0.0 to 1.0
double error = jaccard.error_margin();

// Cosine similarity
auto cosine = calc.cosine_similarity(set1, set2);

// Dice coefficient
auto dice = calc.dice_coefficient(set1, set2);
```

## Common Patterns

### Privacy-Preserving Set Intersection

```cpp
template<typename T>
std::vector<T> private_intersection(
    const std::vector<T>& data1,
    const std::vector<T>& data2,
    const std::string& key) {

    trapdoor_factory<32> factory(key);

    // Build sets
    boolean_set<T, 32> set1, set2;
    for (const auto& elem : data1) {
        auto td = factory.create(elem);
        boolean_set<T, 32> e(td.hash(), td.key_fingerprint());
        set1 = set1 | e;
    }
    for (const auto& elem : data2) {
        auto td = factory.create(elem);
        boolean_set<T, 32> e(td.hash(), td.key_fingerprint());
        set2 = set2 | e;
    }

    // Intersection
    auto common = set1 & set2;

    // Find actual elements
    std::vector<T> result;
    for (const auto& elem : data1) {
        auto td = factory.create(elem);
        if (common.contains(td).value() &&
            std::find(data2.begin(), data2.end(), elem) != data2.end()) {
            result.push_back(elem);
        }
    }

    return result;
}
```

### Bloom Filter Pattern

```cpp
class PrivateBloomFilter {
    std::vector<boolean_set<std::string, 32>> hashes_;
    trapdoor_factory<32> factory_;
    size_t k_;  // Number of hash functions

public:
    PrivateBloomFilter(const std::string& key, size_t k = 3)
        : factory_(key), k_(k) {
        hashes_.resize(k);
    }

    void add(const std::string& elem) {
        for (size_t i = 0; i < k_; ++i) {
            auto salted = elem + "_" + std::to_string(i);
            auto td = factory_.create(salted);
            boolean_set<std::string, 32> e(td.hash(), td.key_fingerprint());
            hashes_[i] = hashes_[i] | e;
        }
    }

    bool probably_contains(const std::string& elem) {
        for (size_t i = 0; i < k_; ++i) {
            auto salted = elem + "_" + std::to_string(i);
            auto td = factory_.create(salted);
            if (!hashes_[i].contains(td).value()) {
                return false;  // Definitely not present
            }
        }
        return true;  // Probably present
    }
};
```

### Secure Multi-Party Computation

```cpp
class SecureMPC {
    trapdoor_factory<32> factory_;

public:
    SecureMPC(const std::string& shared_key) : factory_(shared_key) {}

    // Each party computes their set locally
    boolean_set<std::string, 32> prepare_party_data(
        const std::vector<std::string>& data) {

        boolean_set<std::string, 32> party_set;
        for (const auto& elem : data) {
            auto td = factory_.create(elem);
            boolean_set<std::string, 32> e(td.hash(), td.key_fingerprint());
            party_set = party_set | e;
        }
        return party_set;
    }

    // Combine results from multiple parties
    boolean_set<std::string, 32> secure_union(
        const std::vector<boolean_set<std::string, 32>>& party_sets) {

        boolean_set<std::string, 32> result;
        for (const auto& set : party_sets) {
            result = result | set;
        }
        return result;
    }
};
```

## Hash Size Selection Guide

| Hash Size | Security Level | False Positive Rate | Use Case |
|-----------|---------------|-------------------|----------|
| 8 bytes   | 64-bit        | ~10^-19          | Testing, non-critical |
| 16 bytes  | 128-bit       | ~10^-38          | Standard applications |
| 32 bytes  | 256-bit       | ~10^-77          | High security (default) |
| 64 bytes  | 512-bit       | ~10^-154         | Maximum security |

## Error Handling

```cpp
// Check compatibility before operations
if (!td1.compatible_with(td2)) {
    throw std::invalid_argument("Incompatible trapdoor keys");
}

// Handle approximate results
auto result = set.contains(td);
if (result.false_positive_rate() > acceptable_error) {
    // Handle high error rate
    std::cerr << "Warning: High false positive rate\n";
}

// Validate hash sizes match
static_assert(sizeof(trapdoor<T, 32>) == sizeof(trapdoor<U, 32>),
              "Hash sizes must match");
```

## Performance Tips

```cpp
// 1. Use batch operations for multiple items
auto batch = batch_operations::batch_create_trapdoors(items, key);

// 2. Reuse factory instances
class TrapdoorManager {
    trapdoor_factory<32> factory_;  // Reuse this
public:
    TrapdoorManager(const std::string& key) : factory_(key) {}
};

// 3. Prefer smaller hash sizes when appropriate
trapdoor_factory<16> fast_factory("key");  // Faster, less secure

// 4. Cache frequently used trapdoors
std::unordered_map<std::string, trapdoor<std::string, 32>> cache;

// 5. Use move semantics
auto td = std::move(expensive_trapdoor);

// 6. Parallelize independent operations
#pragma omp parallel for
for (size_t i = 0; i < items.size(); ++i) {
    results[i] = factory.create(items[i]);
}
```

## Compilation

```bash
# Basic compilation
g++ -std=c++17 -I/path/to/include program.cpp -o program

# With optimizations
g++ -std=c++17 -O3 -march=native -I/path/to/include program.cpp -o program

# With OpenMP for parallelization
g++ -std=c++17 -O3 -fopenmp -I/path/to/include program.cpp -o program

# Debug build
g++ -std=c++17 -g -DDEBUG -I/path/to/include program.cpp -o program
```

## Common Includes

```cpp
// Standard library
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>

// Cipher trapdoor sets
#include <cipher_trapdoor_sets/cts.hpp>
#include <cipher_trapdoor_sets/sets/boolean_set.hpp>
#include <cipher_trapdoor_sets/operations/batch_ops.hpp>

// Namespaces
using namespace cts;
using namespace cts::sets;
using namespace cts::operations;
```

---

*For detailed explanations and examples, see the full [Tutorial Series](../README.md)*