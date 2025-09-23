# Cipher Trapdoor Sets (CTS)

A modern C++20 header-only library for privacy-preserving set operations using cryptographic trapdoor functions. This library enables secure computation on encrypted sets without revealing the underlying data.

## Features

- **Privacy-Preserving**: Perform set operations without exposing actual values
- **One-Way Trapdoors**: Cryptographic hash-based transformations that cannot be reversed
- **Approximate Operations**: All operations include explicit error rates
- **Composable Design**: Simple components that combine to create powerful applications
- **Modern C++20**: Uses concepts, ranges, and parallel STL for performance
- **Header-Only**: Easy integration with no compilation required

## Quick Start

```cpp
#include <cipher_trapdoor_sets/cts.hpp>
#include <iostream>
#include <vector>

using namespace cts;

int main() {
    // Secret key for trapdoor generation
    std::string_view secret = "my-secret-key";

    // Create trapdoor factory
    trapdoor_factory<32> factory(secret);

    // Create trapdoors from sensitive data
    auto td1 = factory.create(std::string("Alice"));
    auto td2 = factory.create(std::string("Bob"));

    // Compare without revealing actual values
    auto equal = (td1 == td2);
    std::cout << "Equal: " << equal.value()
              << " (FPR: " << equal.false_positive_rate() << ")\n";

    // Create privacy-preserving sets
    sets::boolean_set_factory<std::string> set_factory(secret);
    std::vector<std::string> documents = {"doc1", "doc2", "doc3"};
    auto private_set = set_factory.from_collection(documents);

    // Perform set operations without exposing data
    auto td_doc = factory.create(std::string("doc2"));
    auto contains = private_set.contains(td_doc);
    std::cout << "Contains doc2: " << contains.value() << "\n";

    return 0;
}
```

## Building

### Requirements
- C++20 compatible compiler (GCC 11+, Clang 13+, MSVC 2022+)
- CMake 3.20+
- Optional: Intel TBB for parallel operations

### Build Instructions

```bash
mkdir build && cd build
cmake ..
cmake --build .

# Run tests
ctest --output-on-failure

# Run examples
./examples/basic_trapdoor
./examples/set_operations
```

### CMake Integration

```cmake
find_package(cipher_trapdoor_sets REQUIRED)
target_link_libraries(your_target PRIVATE cts::cts)
```

Or using FetchContent:

```cmake
include(FetchContent)
FetchContent_Declare(
    cipher_trapdoor_sets
    GIT_REPOSITORY https://github.com/queelius/cipher_trapdoor_sets.git
    GIT_TAG main
)
FetchContent_MakeAvailable(cipher_trapdoor_sets)
target_link_libraries(your_target PRIVATE cts::cts)
```

## Core Concepts

### Trapdoors
One-way cryptographic transformations that preserve equality testing while hiding actual values:

```cpp
trapdoor_factory<32> factory(secret_key);
auto td = factory.create(sensitive_value);
```

### Approximate Values
All operations return approximate results with explicit error rates:

```cpp
auto result = operation();
if (result.value() && result.false_positive_rate() < 0.001) {
    // High confidence in positive result
}
```

### Set Types

#### Symmetric Difference Sets
Support XOR-based operations, ideal for disjoint set unions:

```cpp
sets::symmetric_difference_set_factory<T> factory(secret);
auto set1 = factory.from_unique(collection1);
auto set2 = factory.from_unique(collection2);
auto union_set = set1 ^ set2;  // XOR for disjoint union
```

#### Boolean Sets
Full Boolean algebra operations with membership testing:

```cpp
sets::boolean_set_factory<T> factory(secret);
auto set = factory.from_collection(items);
auto intersection = set1 & set2;
auto union_set = set1 | set2;
auto complement = ~set1;
```

## API Design Principles

1. **Explicit Approximation**: Error rates are always visible
2. **Type Safety**: Strong types prevent mixing incompatible operations
3. **Composability**: Operations combine naturally
4. **Zero-Cost Abstractions**: Template-based design with no runtime overhead
5. **Fail-Fast**: Incompatible operations throw immediately

## Applications

- **Private Set Intersection**: Find common elements without revealing sets
- **Secure Deduplication**: Identify duplicates in encrypted data
- **Privacy-Preserving Analytics**: Compute statistics on sensitive data
- **Encrypted Search**: Query encrypted databases
- **Federated Learning**: Aggregate models without exposing training data

## Performance Considerations

- Hash size affects security and collision probability
- Parallel STL used when available (link with TBB)
- Batch operations provided for efficiency
- Header-only design enables full optimization

## Security Notes

- Use cryptographically secure keys
- Larger hash sizes (256+ bits) recommended for production
- Error rates depend on hash size: FPR ≈ 2^(-bits)
- Keys should never be shared between untrusted parties

## Contributing

Contributions are welcome! Please ensure:
- Code follows modern C++ best practices
- All tests pass
- New features include tests and examples
- Documentation is updated

## License

MIT License - See LICENSE file for details

## References

- Cryptographic Hash Functions
- Bloom Filters and Approximate Data Structures
- Private Set Intersection Protocols
- Homomorphic Encryption Techniques
