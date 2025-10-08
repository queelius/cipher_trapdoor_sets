# Tutorial 2: Basic Trapdoor Operations

**Duration:** 60 minutes | **Level:** Beginner | **Prerequisites:** Tutorial 1

## Learning Objectives

By the end of this tutorial, you will:
- Create trapdoors from various data types (integers, strings, custom objects)
- Master trapdoor factories and key management patterns
- Perform efficient batch operations
- Understand approximate equality testing in depth
- Serialize and store trapdoor values

## Table of Contents

1. [Trapdoor Factory Pattern](#trapdoor-factory-pattern)
2. [Creating Trapdoors from Different Types](#creating-trapdoors-from-different-types)
3. [Batch Operations](#batch-operations)
4. [Advanced Equality Testing](#advanced-equality-testing)
5. [Serialization and Storage](#serialization-and-storage)
6. [Exercises](#exercises)
7. [Key Takeaways](#key-takeaways)

## Trapdoor Factory Pattern

### Understanding the Factory

The `trapdoor_factory` is your gateway to creating consistent trapdoors:

```cpp
#include <cipher_trapdoor_sets/cts.hpp>
#include <iostream>

using namespace cts;

int main() {
    // Factory encapsulates:
    // 1. Secret key for HMAC
    // 2. Hash algorithm configuration
    // 3. Key fingerprint for compatibility checking

    trapdoor_factory<32> factory("my-secret-key-2025");

    // Key fingerprint ensures trapdoor compatibility
    std::cout << "Key fingerprint: " << factory.key_fingerprint() << "\n";

    return 0;
}
```

### Key Management Best Practices

```cpp
#include <cipher_trapdoor_sets/cts.hpp>
#include <cstdlib>
#include <string>

// Production key management example
class SecureTrapdoorManager {
private:
    trapdoor_factory<32> factory_;

public:
    SecureTrapdoorManager()
        : factory_(load_key_from_environment()) {}

    static std::string load_key_from_environment() {
        // In production: Use HSM, KMS, or secure vault
        const char* key = std::getenv("TRAPDOOR_SECRET_KEY");
        if (!key) {
            throw std::runtime_error("Secret key not configured");
        }
        return std::string(key);
    }

    template<typename T>
    auto create_trapdoor(const T& value) {
        return factory_.create(value);
    }

    // Rotate keys periodically
    void rotate_key(const std::string& new_key) {
        // Archive old factory for backward compatibility
        factory_ = trapdoor_factory<32>(new_key);
    }
};
```

### Multiple Factories for Different Contexts

```cpp
// Different factories for different data domains
class MultiDomainSystem {
    trapdoor_factory<32> user_factory_;
    trapdoor_factory<32> product_factory_;
    trapdoor_factory<16> session_factory_;  // Smaller hash for temporary data

public:
    MultiDomainSystem()
        : user_factory_("user-key-2025")
        , product_factory_("product-key-2025")
        , session_factory_("session-key-2025") {}

    auto protect_user_id(const std::string& user_id) {
        return user_factory_.create(user_id);
    }

    auto protect_product_id(int product_id) {
        return product_factory_.create(product_id);
    }

    auto protect_session_token(const std::string& token) {
        return session_factory_.create(token);
    }
};
```

## Creating Trapdoors from Different Types

### Built-in Types

The library supports any type that's hashable with `std::hash`:

```cpp
#include <cipher_trapdoor_sets/cts.hpp>
#include <iostream>
#include <string>
#include <vector>

using namespace cts;

void demonstrate_builtin_types() {
    trapdoor_factory<32> factory("demo-key-2025");

    // Strings
    auto td_string = factory.create(std::string("hello"));

    // Integers
    auto td_int = factory.create(42);
    auto td_long = factory.create(9999999999L);

    // Floating point (be careful with precision!)
    auto td_double = factory.create(3.14159);

    // Characters
    auto td_char = factory.create('A');

    // Pointers (hashes the address, not the value!)
    int value = 100;
    auto td_ptr = factory.create(&value);

    std::cout << "Created trapdoors for various built-in types\n";
}
```

### Custom Types

Make your custom types trapdoor-compatible:

```cpp
#include <cipher_trapdoor_sets/cts.hpp>
#include <functional>
#include <string>

// Custom user type
struct User {
    std::string id;
    std::string email;
    int age;

    bool operator==(const User& other) const {
        return id == other.id && email == other.email && age == other.age;
    }
};

// Provide hash specialization
namespace std {
    template<>
    struct hash<User> {
        size_t operator()(const User& user) const {
            // Combine hashes of members
            size_t h1 = hash<string>{}(user.id);
            size_t h2 = hash<string>{}(user.email);
            size_t h3 = hash<int>{}(user.age);

            // Combine using boost::hash_combine algorithm
            h1 ^= h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2);
            h1 ^= h3 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2);
            return h1;
        }
    };
}

void use_custom_type() {
    using namespace cts;

    trapdoor_factory<32> factory("user-key-2025");

    User alice{"u001", "alice@example.com", 28};
    User bob{"u002", "bob@example.com", 32};

    auto td_alice = factory.create(alice);
    auto td_bob = factory.create(bob);

    auto result = (td_alice == td_bob);
    std::cout << "Users match: " << result.value() << "\n";
}
```

### Container Types

Working with collections:

```cpp
#include <cipher_trapdoor_sets/cts.hpp>
#include <vector>
#include <set>
#include <map>

void demonstrate_containers() {
    using namespace cts;

    trapdoor_factory<32> factory("container-key-2025");

    // Vectors - order matters!
    std::vector<int> vec1 = {1, 2, 3};
    std::vector<int> vec2 = {3, 2, 1};  // Different order

    // Need custom hash for vectors
    auto hash_vector = [](const std::vector<int>& v) {
        size_t seed = v.size();
        for (auto& i : v) {
            seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    };

    // Manual hashing then trapdoor
    size_t h1 = hash_vector(vec1);
    size_t h2 = hash_vector(vec2);

    auto td1 = factory.create(h1);
    auto td2 = factory.create(h2);

    std::cout << "Vectors equal: " << (td1 == td2).value() << "\n";
    // Will be false because order matters
}
```

## Batch Operations

### Efficient Batch Creation

```cpp
#include <cipher_trapdoor_sets/cts.hpp>
#include <cipher_trapdoor_sets/operations/batch_ops.hpp>
#include <vector>
#include <string>
#include <chrono>

void demonstrate_batch_operations() {
    using namespace cts;
    using namespace cts::operations;

    // Generate test data
    std::vector<std::string> emails;
    for (int i = 0; i < 10000; ++i) {
        emails.push_back("user" + std::to_string(i) + "@example.com");
    }

    // Method 1: Individual creation (slower)
    auto start = std::chrono::high_resolution_clock::now();

    trapdoor_factory<32> factory("batch-key-2025");
    std::vector<trapdoor<std::string, 32>> individual_trapdoors;
    for (const auto& email : emails) {
        individual_trapdoors.push_back(factory.create(email));
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto individual_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Method 2: Batch creation (optimized)
    start = std::chrono::high_resolution_clock::now();

    auto batch_trapdoors = batch_operations::batch_create_trapdoors(
        emails, "batch-key-2025");

    end = std::chrono::high_resolution_clock::now();
    auto batch_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Individual creation: " << individual_time.count() << "ms\n";
    std::cout << "Batch creation: " << batch_time.count() << "ms\n";
    std::cout << "Speedup: " << (float)individual_time.count() / batch_time.count() << "x\n";
}
```

### Batch Comparisons

```cpp
void batch_comparison_example() {
    using namespace cts;
    using namespace cts::operations;

    std::vector<std::string> dataset1 = {"Alice", "Bob", "Charlie", "Diana"};
    std::vector<std::string> dataset2 = {"Alice", "Bob", "Eve", "Diana"};

    // Create trapdoors for both datasets
    auto td_set1 = batch_operations::batch_create_trapdoors(dataset1, "key");
    auto td_set2 = batch_operations::batch_create_trapdoors(dataset2, "key");

    // Batch comparison
    auto results = batch_operations::batch_equals(td_set1, td_set2);

    // Analyze results
    int matches = 0;
    for (size_t i = 0; i < results.size(); ++i) {
        if (results[i].value()) {
            std::cout << "Position " << i << ": MATCH\n";
            matches++;
        } else {
            std::cout << "Position " << i << ": Different\n";
        }
    }

    std::cout << "Total matches: " << matches << "/" << results.size() << "\n";
}
```

### Parallel Processing

```cpp
#include <execution>
#include <algorithm>

void parallel_batch_operations() {
    using namespace cts;

    std::vector<int> ids(1000000);
    std::iota(ids.begin(), ids.end(), 0);

    trapdoor_factory<32> factory("parallel-key-2025");
    std::vector<trapdoor<int, 32>> trapdoors(ids.size());

    // Parallel transformation
    std::transform(std::execution::par_unseq,
                   ids.begin(), ids.end(),
                   trapdoors.begin(),
                   [&factory](int id) { return factory.create(id); });

    std::cout << "Created " << trapdoors.size() << " trapdoors in parallel\n";
}
```

## Advanced Equality Testing

### Understanding Approximate Equality

```cpp
#include <cipher_trapdoor_sets/cts.hpp>
#include <iostream>
#include <iomanip>

void demonstrate_approximate_equality() {
    using namespace cts;

    trapdoor_factory<32> factory("equality-key-2025");

    auto td1 = factory.create(std::string("Alice"));
    auto td2 = factory.create(std::string("Alice"));
    auto td3 = factory.create(std::string("Bob"));

    // Get detailed comparison results
    auto result_same = td1.equals(td2);
    auto result_diff = td1.equals(td3);

    std::cout << std::scientific << std::setprecision(2);

    std::cout << "Comparing identical values:\n";
    std::cout << "  Result: " << (result_same.value() ? "EQUAL" : "DIFFERENT") << "\n";
    std::cout << "  False Positive Rate: " << result_same.false_positive_rate() << "\n";
    std::cout << "  False Negative Rate: " << result_same.false_negative_rate() << "\n";
    std::cout << "  Confidence: " << (1.0 - result_same.false_positive_rate()) * 100 << "%\n";

    std::cout << "\nComparing different values:\n";
    std::cout << "  Result: " << (result_diff.value() ? "EQUAL" : "DIFFERENT") << "\n";
    std::cout << "  False Positive Rate: " << result_diff.false_positive_rate() << "\n";
    std::cout << "  False Negative Rate: " << result_diff.false_negative_rate() << "\n";
}
```

### Adjusting Hash Size for Error Rates

```cpp
template<size_t HashSize>
void analyze_hash_size() {
    using namespace cts;

    trapdoor_factory<HashSize> factory("analysis-key");

    // Create many trapdoors to test collision probability
    const int num_samples = 100000;
    std::vector<trapdoor<int, HashSize>> trapdoors;

    for (int i = 0; i < num_samples; ++i) {
        trapdoors.push_back(factory.create(i));
    }

    // Check for collisions
    int collisions = 0;
    for (int i = 0; i < num_samples; ++i) {
        for (int j = i + 1; j < std::min(i + 100, num_samples); ++j) {
            if ((trapdoors[i] == trapdoors[j]).value()) {
                collisions++;
            }
        }
    }

    std::cout << "Hash size: " << HashSize << " bytes (" << HashSize * 8 << " bits)\n";
    std::cout << "Theoretical FPR: " << std::pow(2.0, -(HashSize * 8)) << "\n";
    std::cout << "Observed collisions: " << collisions << "\n\n";
}

void compare_hash_sizes() {
    analyze_hash_size<8>();   // 64-bit
    analyze_hash_size<16>();  // 128-bit
    analyze_hash_size<32>();  // 256-bit
    analyze_hash_size<64>();  // 512-bit
}
```

## Serialization and Storage

### Binary Serialization

```cpp
#include <cipher_trapdoor_sets/serialization/binary_format.hpp>
#include <fstream>
#include <vector>

void demonstrate_serialization() {
    using namespace cts;
    using namespace cts::serialization;

    trapdoor_factory<32> factory("serial-key-2025");

    // Create trapdoors
    std::vector<trapdoor<std::string, 32>> trapdoors;
    trapdoors.push_back(factory.create(std::string("Alice")));
    trapdoors.push_back(factory.create(std::string("Bob")));
    trapdoors.push_back(factory.create(std::string("Charlie")));

    // Serialize to file
    {
        std::ofstream file("trapdoors.bin", std::ios::binary);
        binary_writer writer(file);

        // Write count
        writer.write(trapdoors.size());

        // Write each trapdoor
        for (const auto& td : trapdoors) {
            writer.write(td.hash());
            writer.write(td.key_fingerprint());
        }
    }

    // Deserialize from file
    std::vector<trapdoor<std::string, 32>> loaded_trapdoors;
    {
        std::ifstream file("trapdoors.bin", std::ios::binary);
        binary_reader reader(file);

        // Read count
        size_t count;
        reader.read(count);

        // Read trapdoors
        for (size_t i = 0; i < count; ++i) {
            core::hash_value<32> hash;
            size_t key_fingerprint;

            reader.read(hash);
            reader.read(key_fingerprint);

            loaded_trapdoors.emplace_back(hash, key_fingerprint);
        }
    }

    // Verify
    for (size_t i = 0; i < trapdoors.size(); ++i) {
        auto result = (trapdoors[i] == loaded_trapdoors[i]);
        std::cout << "Trapdoor " << i << " matches: " << result.value() << "\n";
    }
}
```

### Database Storage

```cpp
// Example schema for storing trapdoors in SQL database
/*
CREATE TABLE trapdoors (
    id SERIAL PRIMARY KEY,
    hash_value BYTEA NOT NULL,
    key_fingerprint BIGINT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_hash (hash_value),
    INDEX idx_fingerprint (key_fingerprint)
);
*/

#include <vector>
#include <string>

class TrapdoorDatabase {
public:
    void store_trapdoor(const trapdoor<std::string, 32>& td, const std::string& metadata) {
        // Convert hash to hex string for storage
        std::string hex_hash = to_hex(td.hash());

        // SQL: INSERT INTO trapdoors (hash_value, key_fingerprint, metadata)
        //      VALUES ($1, $2, $3)

        // In practice, use prepared statements
    }

    std::vector<trapdoor<std::string, 32>> find_matches(const trapdoor<std::string, 32>& query) {
        std::string hex_hash = to_hex(query.hash());

        // SQL: SELECT * FROM trapdoors
        //      WHERE hash_value = $1 AND key_fingerprint = $2

        std::vector<trapdoor<std::string, 32>> matches;
        // Load and return matches
        return matches;
    }

private:
    std::string to_hex(const core::hash_value<32>& hash) {
        std::string hex;
        for (size_t i = 0; i < 32; ++i) {
            char buf[3];
            sprintf(buf, "%02x", hash.data[i]);
            hex += buf;
        }
        return hex;
    }
};
```

## Complete Example: Privacy-Preserving User Matching

Let's build a complete system that matches users across services without revealing identities:

```cpp
#include <cipher_trapdoor_sets/cts.hpp>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>

using namespace cts;

class PrivacyPreservingUserMatcher {
private:
    trapdoor_factory<32> factory_;
    std::unordered_map<std::string, trapdoor<std::string, 32>> user_database_;

public:
    PrivacyPreservingUserMatcher(const std::string& secret_key)
        : factory_(secret_key) {}

    // Service A uploads their user emails as trapdoors
    void upload_service_a_users(const std::vector<std::string>& emails) {
        std::cout << "Service A uploading " << emails.size() << " users...\n";

        for (const auto& email : emails) {
            auto td = factory_.create(email);
            user_database_["service_a_" + std::to_string(user_database_.size())] = td;
        }
    }

    // Service B queries with their users to find matches
    std::vector<bool> find_matches_for_service_b(const std::vector<std::string>& emails) {
        std::cout << "Service B querying " << emails.size() << " users...\n";

        std::vector<bool> matches;

        for (const auto& email : emails) {
            auto query_td = factory_.create(email);
            bool found = false;

            for (const auto& [id, stored_td] : user_database_) {
                if ((query_td == stored_td).value()) {
                    found = true;
                    break;
                }
            }

            matches.push_back(found);
        }

        return matches;
    }

    void print_statistics() {
        std::cout << "\n=== System Statistics ===\n";
        std::cout << "Total stored trapdoors: " << user_database_.size() << "\n";
        std::cout << "Hash size: 256 bits\n";
        std::cout << "False positive rate: < 10^-77\n";
        std::cout << "Privacy guarantee: Original emails cannot be recovered\n";
    }
};

int main() {
    // Initialize system with secret key
    PrivacyPreservingUserMatcher matcher("production-key-2025");

    // Service A's users
    std::vector<std::string> service_a_users = {
        "alice@example.com",
        "bob@example.com",
        "charlie@example.com",
        "diana@example.com"
    };

    // Service B's users (some overlap)
    std::vector<std::string> service_b_users = {
        "alice@example.com",    // Match
        "eve@example.com",       // No match
        "charlie@example.com",   // Match
        "frank@example.com"      // No match
    };

    // Upload Service A users
    matcher.upload_service_a_users(service_a_users);

    // Find matches for Service B
    auto matches = matcher.find_matches_for_service_b(service_b_users);

    // Report results
    std::cout << "\n=== Match Results ===\n";
    for (size_t i = 0; i < service_b_users.size(); ++i) {
        std::cout << "User " << i << ": "
                  << (matches[i] ? "FOUND in Service A" : "Not found") << "\n";
        // Note: We never reveal the actual email addresses!
    }

    matcher.print_statistics();

    return 0;
}
```

## Exercises

### Exercise 1: Multi-Type Trapdoor Container

Create a container that can hold trapdoors of different types safely.

```cpp
// TODO: Design a heterogeneous trapdoor container
// Hint: Use std::variant or type erasure
```

[Solution](exercises/tutorial2_ex1_solution.cpp)

### Exercise 2: Trapdoor Cache with TTL

Implement a cache for trapdoors with time-to-live expiration.

```cpp
// TODO: Create TrapdoorCache class
// - Store trapdoors with timestamps
// - Automatic expiration after TTL
// - Thread-safe operations
```

[Solution](exercises/tutorial2_ex2_solution.cpp)

### Exercise 3: Bloom Filter Integration

Use trapdoors to build a space-efficient Bloom filter.

```cpp
// TODO: Implement PrivateBloomFilter
// - Use multiple hash functions from trapdoor
// - Approximate membership testing
// - Calculate optimal parameters
```

[Solution](exercises/tutorial2_ex3_solution.cpp)

## Common Pitfalls

### 1. Hash Size Mismatches

**Wrong:**
```cpp
trapdoor_factory<32> factory1("key");
trapdoor_factory<16> factory2("key");  // Different size!

auto td1 = factory1.create("value");
auto td2 = factory2.create("value");
// td1 and td2 are incompatible types
```

**Right:**
```cpp
constexpr size_t HASH_SIZE = 32;
trapdoor_factory<HASH_SIZE> factory1("key");
trapdoor_factory<HASH_SIZE> factory2("key");
```

### 2. Floating Point Precision Issues

**Wrong:**
```cpp
auto td1 = factory.create(0.1 + 0.2);  // 0.30000000000000004
auto td2 = factory.create(0.3);        // 0.3
// May not match due to floating point precision!
```

**Right:**
```cpp
// Round or use fixed precision
auto round_to_n = [](double x, int n) {
    return std::round(x * std::pow(10, n)) / std::pow(10, n);
};

auto td1 = factory.create(round_to_n(0.1 + 0.2, 10));
auto td2 = factory.create(round_to_n(0.3, 10));
```

### 3. Forgetting Key Fingerprint Checks

**Wrong:**
```cpp
auto result = (td1 == td2);  // May throw if incompatible!
```

**Right:**
```cpp
if (td1.compatible_with(td2)) {
    auto result = (td1 == td2);
    // Process result
} else {
    // Handle incompatible trapdoors
}
```

## Performance Tips

### 1. Batch Operations

Always prefer batch operations when processing multiple items:
- 10-50x faster for large datasets
- Better cache utilization
- Enables parallelization

### 2. Hash Size Selection

Choose hash size based on your needs:
- 16 bytes: High performance, suitable for non-critical matching
- 32 bytes: Standard security, good balance
- 64 bytes: Maximum security, when collisions are unacceptable

### 3. Caching Strategies

```cpp
// Cache frequently used trapdoors
std::unordered_map<std::string, trapdoor<std::string, 32>> cache;

auto get_or_create_trapdoor(const std::string& value) {
    auto it = cache.find(value);
    if (it != cache.end()) {
        return it->second;
    }

    auto td = factory.create(value);
    cache[value] = td;
    return td;
}
```

## Key Takeaways

✅ **Factory pattern ensures consistency**: All trapdoors from a factory use the same key

✅ **Type flexibility**: Can protect any hashable type

✅ **Batch operations are crucial**: Significant performance gains for bulk processing

✅ **Approximate equality is probabilistic**: Always consider false positive rates

✅ **Serialization enables persistence**: Store and retrieve trapdoors efficiently

## What's Next?

In [Tutorial 3: Boolean Set Operations](03_boolean_sets.md), we'll explore:
- Set union, intersection, and complement
- Privacy-preserving set operations
- Approximate membership testing
- Building oblivious data structures

---

*Continue to [Tutorial 3: Boolean Set Operations](03_boolean_sets.md) →*