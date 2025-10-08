# Tutorial 1: Introduction to Trapdoor Functions

**Duration:** 45 minutes | **Level:** Beginner | **Prerequisites:** Basic C++

## Learning Objectives

By the end of this tutorial, you will:
- Understand what cryptographic trapdoor functions are and why they matter
- Learn the security properties that make trapdoors useful for privacy
- Write your first program using trapdoor transformations
- Grasp the trade-offs between privacy and accuracy

## Table of Contents

1. [Why Privacy-Preserving Computation?](#why-privacy-preserving-computation)
2. [Understanding Trapdoor Functions](#understanding-trapdoor-functions)
3. [Security Properties](#security-properties)
4. [Your First Trapdoor Program](#your-first-trapdoor-program)
5. [Exercises](#exercises)
6. [Key Takeaways](#key-takeaways)

## Why Privacy-Preserving Computation?

### The Problem

Imagine these scenarios:

1. **Contact Discovery**: WhatsApp wants to find which of your contacts use their service without uploading your entire contact list to their servers.

2. **Medical Research**: Hospitals want to find patients with rare diseases across institutions without sharing patient records.

3. **Fraud Detection**: Banks need to share fraud patterns without revealing customer transaction details.

In each case, we need to compute on sensitive data without exposing the data itself. This is where cryptographic trapdoor functions come in.

### The Solution: Trapdoor Functions

A trapdoor function is like a one-way street in cryptography:
- **Easy to compute forward**: Given input `x`, computing `f(x)` is fast
- **Hard to reverse**: Given output `y`, finding `x` where `f(x) = y` is computationally infeasible
- **Enables private operations**: We can work with `f(x)` without knowing `x`

## Understanding Trapdoor Functions

### Conceptual Model

Think of a trapdoor function like a paper shredder that creates a unique pattern:

```
Original Document → [SHREDDER] → Unique Shred Pattern
    "Alice"      →     f()      →   0x3f2a8b9c...
```

Key properties:
- Same input always produces the same output (deterministic)
- Can't reconstruct the document from the shred pattern
- Different documents produce different patterns (with high probability)

### Mathematical Foundation

Our library uses cryptographic hash functions as the trapdoor mechanism:

```cpp
trapdoor(value, key) = HASH(key || value)
```

Where:
- `HASH` is a cryptographic hash function (SHA-256 by default)
- `key` is a secret that determines the specific trapdoor function
- `||` denotes concatenation

### Visual Representation

```
    INPUT SPACE                    OUTPUT SPACE

    "Alice" ─────┐
                 ├──→ Hash ──→ 0x3f2a8b9c7d6e5f4a...
    Secret Key ──┘

    "Bob" ───────┐
                 ├──→ Hash ──→ 0x9e8d7c6b5a4f3e2d...
    Secret Key ──┘
```

## Security Properties

### 1. Preimage Resistance

**Property**: Given hash `h`, cannot find input `x` where `hash(x) = h`

**Why it matters**: Protects the original data from being recovered

**In practice**:
```cpp
// Given only this hash, you cannot determine it came from "Alice"
hash_value<32> h = {0x3f, 0x2a, 0x8b, ...};
// Finding "Alice" would require ~2^256 operations
```

### 2. Collision Resistance

**Property**: Hard to find two different inputs that produce the same hash

**Why it matters**: Ensures different data items remain distinguishable

**Probability of collision**:
- With 256-bit hashes: ~1 in 2^256
- Birthday paradox: Need ~2^128 items for 50% collision chance

### 3. Key Dependency

**Property**: Different keys produce completely different trapdoors

**Why it matters**: Provides isolation between different applications/users

```cpp
trapdoor_factory<32> factory1("key1");
trapdoor_factory<32> factory2("key2");

auto td1 = factory1.create("Alice");  // Hash depends on key1
auto td2 = factory2.create("Alice");  // Completely different hash

// td1 and td2 are incompatible and unrelated
```

## Your First Trapdoor Program

Let's build a simple privacy-preserving name matcher:

### Step 1: Basic Setup

Create `tutorial1_example.cpp`:

```cpp
#include <cipher_trapdoor_sets/cts.hpp>
#include <iostream>
#include <string>
#include <vector>

using namespace cts;

int main() {
    std::cout << "=== Privacy-Preserving Name Matcher ===\n\n";

    // Step 1: Create a secret key
    // In production, this would be securely generated and stored
    std::string_view secret_key = "tutorial-secret-key-2025";

    // Step 2: Create a trapdoor factory
    // The factory encapsulates the key and hash parameters
    trapdoor_factory<32> factory(secret_key);

    std::cout << "Trapdoor factory initialized with secret key.\n";
    std::cout << "Hash size: 256 bits (32 bytes)\n\n";

    return 0;
}
```

### Step 2: Create Trapdoors

Add to your main function:

```cpp
    // Step 3: Transform sensitive data into trapdoors
    std::vector<std::string> names = {
        "Alice Johnson",
        "Bob Smith",
        "Charlie Brown",
        "Alice Johnson"  // Duplicate for testing
    };

    std::cout << "Creating trapdoors for " << names.size() << " names...\n";

    std::vector<trapdoor<std::string, 32>> trapdoors;
    for (const auto& name : names) {
        trapdoors.push_back(factory.create(name));
    }

    std::cout << "Trapdoors created. Original names are now protected.\n\n";
```

### Step 3: Compare Trapdoors

```cpp
    // Step 4: Compare trapdoors without revealing original values
    std::cout << "Comparing trapdoors (privacy-preserving):\n";
    std::cout << "----------------------------------------\n";

    for (size_t i = 0; i < trapdoors.size(); ++i) {
        for (size_t j = i + 1; j < trapdoors.size(); ++j) {
            auto result = (trapdoors[i] == trapdoors[j]);

            std::cout << "Trapdoor[" << i << "] vs Trapdoor[" << j << "]: ";

            if (result.value()) {
                std::cout << "MATCH (";
                // Only for demonstration - in real use, we wouldn't have names
                std::cout << names[i] << " = " << names[j];
                std::cout << ")\n";
            } else {
                std::cout << "Different\n";
            }

            std::cout << "  False positive rate: " << result.false_positive_rate() << "\n";
        }
    }
```

### Step 4: Demonstrate One-Way Property

```cpp
    // Step 5: Show that trapdoors cannot be reversed
    std::cout << "\n=== One-Way Property Demonstration ===\n";
    std::cout << "Original value: " << names[0] << "\n";
    std::cout << "Trapdoor hash (first 16 bytes): ";

    const auto& hash = trapdoors[0].hash();
    for (size_t i = 0; i < 16; ++i) {
        printf("%02x", hash.data[i]);
    }
    std::cout << "...\n";

    std::cout << "\nThis hash cannot be reversed to recover '"
              << names[0] << "'\n";
    std::cout << "Reversal would require ~2^256 operations (infeasible)\n";
```

### Complete Program

Compile and run:

```bash
g++ -std=c++17 -I/path/to/cipher_trapdoor_sets/include tutorial1_example.cpp -o tutorial1
./tutorial1
```

Expected output:
```
=== Privacy-Preserving Name Matcher ===

Trapdoor factory initialized with secret key.
Hash size: 256 bits (32 bytes)

Creating trapdoors for 4 names...
Trapdoors created. Original names are now protected.

Comparing trapdoors (privacy-preserving):
----------------------------------------
Trapdoor[0] vs Trapdoor[1]: Different
  False positive rate: 3.67e-78
Trapdoor[0] vs Trapdoor[2]: Different
  False positive rate: 3.67e-78
Trapdoor[0] vs Trapdoor[3]: MATCH (Alice Johnson = Alice Johnson)
  False positive rate: 3.67e-78
...
```

## Understanding False Positive Rates

### What is a False Positive?

A false positive occurs when two different values produce the same hash (collision).

```cpp
// If by extreme chance:
hash("Alice") == hash("Bob")  // False positive!
```

### Calculating the Rate

For N-bit hashes:
- False positive rate ≈ 1/2^N
- 256-bit hash: ≈ 1/2^256 ≈ 10^-78

### Trade-offs

Larger hashes:
- ✅ Lower false positive rate
- ❌ More memory usage
- ❌ Slower operations

Smaller hashes:
- ✅ Less memory
- ✅ Faster operations
- ❌ Higher false positive rate

## Exercises

### Exercise 1: Key Independence

Demonstrate that different keys produce independent trapdoors.

```cpp
// TODO: Create two factories with different keys
// Compare trapdoors of the same value with different keys
// Verify they are completely different
```

[Solution](exercises/tutorial1_ex1_solution.cpp)

### Exercise 2: Batch Processing

Create trapdoors for 1000 random strings and find duplicates.

```cpp
// TODO: Generate 1000 random strings (some duplicates)
// Create trapdoors for all strings
// Find and count matching trapdoors
// Calculate actual vs expected false positive rate
```

[Solution](exercises/tutorial1_ex2_solution.cpp)

### Exercise 3: Different Hash Sizes

Compare false positive rates with different hash sizes.

```cpp
// TODO: Create factories with 16, 32, and 64 byte hashes
// Compare the same values with each
// Measure and compare false positive rates
```

[Solution](exercises/tutorial1_ex3_solution.cpp)

## Common Pitfalls

### 1. Reusing Keys Across Applications

**Wrong:**
```cpp
// DON'T: Same key for different purposes
trapdoor_factory<32> user_factory("my-key");
trapdoor_factory<32> product_factory("my-key");  // Security breach!
```

**Right:**
```cpp
// DO: Application-specific keys
trapdoor_factory<32> user_factory("user-data-key-2025");
trapdoor_factory<32> product_factory("product-data-key-2025");
```

### 2. Comparing Incompatible Trapdoors

**Wrong:**
```cpp
auto td1 = factory1.create("Alice");
auto td2 = factory2.create("Alice");
auto result = (td1 == td2);  // Exception! Different keys
```

**Right:**
```cpp
if (td1.compatible_with(td2)) {
    auto result = (td1 == td2);
} else {
    std::cout << "Trapdoors use different keys\n";
}
```

### 3. Assuming Zero False Positives

**Wrong:**
```cpp
if (td1 == td2) {
    // Definitely the same value - WRONG!
    do_critical_operation();
}
```

**Right:**
```cpp
auto result = (td1 == td2);
if (result.value() && result.false_positive_rate() < threshold) {
    // Probably the same (check FPR for your use case)
    do_operation_with_probability_awareness();
}
```

## Key Takeaways

✅ **Trapdoor functions are one-way**: Easy to compute, hard to reverse

✅ **Privacy through transformation**: Work with encrypted values without decryption

✅ **Deterministic but secure**: Same input → same output, but output reveals nothing

✅ **False positives are rare but possible**: Design systems to handle them

✅ **Key management is critical**: Different keys for different contexts

## What's Next?

In [Tutorial 2: Basic Trapdoor Operations](02_basic_operations.md), we'll explore:
- Creating trapdoors from different data types
- Batch operations for efficiency
- Advanced equality testing
- Serialization and storage

## Additional Resources

- [Cryptographic Hash Functions (Wikipedia)](https://en.wikipedia.org/wiki/Cryptographic_hash_function)
- [One-way Functions in Cryptography](https://en.wikipedia.org/wiki/One-way_function)
- [Birthday Problem and Hash Collisions](https://en.wikipedia.org/wiki/Birthday_problem)

---

*Continue to [Tutorial 2: Basic Trapdoor Operations](02_basic_operations.md) →*