#include <cipher_trapdoor_sets/core/hash_types.hpp>
#include <cassert>
#include <iostream>
#include <random>
#include <set>
#include <unordered_set>

using namespace cts::core;

// Test basic hash_value construction and properties
void test_hash_value_construction() {
    std::cout << "Testing hash_value construction...";

    // Test default construction (all zeros)
    {
        hash_value<32> h;
        assert(h.is_zero());
        assert(!h.is_ones());

        for (size_t i = 0; i < 32; ++i) {
            assert(h.data[i] == 0);
        }
    }

    // Test explicit construction
    {
        std::array<uint8_t, 16> data;
        for (size_t i = 0; i < 16; ++i) {
            data[i] = static_cast<uint8_t>(i);
        }

        hash_value<16> h(data);
        for (size_t i = 0; i < 16; ++i) {
            assert(h.data[i] == i);
        }
        assert(!h.is_zero());
        assert(!h.is_ones());
    }

    // Test all ones
    {
        hash_value<8> h;
        for (auto& byte : h.data) {
            byte = 0xFF;
        }
        assert(!h.is_zero());
        assert(h.is_ones());
    }

    std::cout << " PASSED\n";
}

// Test bitwise operations exhaustively
void test_bitwise_operations() {
    std::cout << "Testing bitwise operations...";

    // Test XOR
    {
        hash_value<4> h1, h2;
        h1.data = {0x0F, 0xF0, 0xAA, 0x55};
        h2.data = {0xF0, 0x0F, 0x55, 0xAA};

        auto result = h1 ^ h2;
        assert(result.data[0] == 0xFF);
        assert(result.data[1] == 0xFF);
        assert(result.data[2] == 0xFF);
        assert(result.data[3] == 0xFF);
        assert(result.is_ones());

        // XOR with self should be zero
        auto zero = h1 ^ h1;
        assert(zero.is_zero());
    }

    // Test AND
    {
        hash_value<4> h1, h2;
        h1.data = {0xFF, 0x00, 0xF0, 0x0F};
        h2.data = {0xAA, 0xAA, 0x0F, 0xF0};

        auto result = h1 & h2;
        assert(result.data[0] == 0xAA);
        assert(result.data[1] == 0x00);
        assert(result.data[2] == 0x00);
        assert(result.data[3] == 0x00);

        // AND with zero should be zero
        hash_value<4> zero;
        auto zero_result = h1 & zero;
        assert(zero_result.is_zero());

        // AND with ones should be self
        hash_value<4> ones;
        for (auto& b : ones.data) b = 0xFF;
        auto self_result = h1 & ones;
        assert(self_result == h1);
    }

    // Test OR
    {
        hash_value<4> h1, h2;
        h1.data = {0x0F, 0x00, 0xF0, 0x0F};
        h2.data = {0xF0, 0xFF, 0x0F, 0xF0};

        auto result = h1 | h2;
        assert(result.data[0] == 0xFF);
        assert(result.data[1] == 0xFF);
        assert(result.data[2] == 0xFF);
        assert(result.data[3] == 0xFF);
        assert(result.is_ones());

        // OR with zero should be self
        hash_value<4> zero;
        auto self_result = h1 | zero;
        assert(self_result == h1);
    }

    // Test NOT
    {
        hash_value<4> h;
        h.data = {0x00, 0xFF, 0xAA, 0x55};

        auto result = ~h;
        assert(result.data[0] == 0xFF);
        assert(result.data[1] == 0x00);
        assert(result.data[2] == 0x55);
        assert(result.data[3] == 0xAA);

        // Double NOT should be identity
        auto double_not = ~(~h);
        assert(double_not == h);

        // NOT of zero should be ones
        hash_value<4> zero;
        auto ones = ~zero;
        assert(ones.is_ones());
    }

    std::cout << " PASSED\n";
}

// Test equality and inequality operators
void test_equality() {
    std::cout << "Testing equality operators...";

    hash_value<8> h1, h2, h3;

    // Test equality of default-constructed values
    assert(h1 == h2);
    assert(!(h1 != h2));

    // Test inequality after modification
    h2.data[0] = 1;
    assert(h1 != h2);
    assert(!(h1 == h2));

    // Test copy equality
    h3 = h2;
    assert(h2 == h3);
    assert(!(h2 != h3));

    // Test self-equality
    assert(h1 == h1);
    assert(!(h1 != h1));

    std::cout << " PASSED\n";
}

// Test different hash sizes
void test_different_sizes() {
    std::cout << "Testing different hash sizes...";

    // Test type aliases
    {
        hash_128 h128;
        assert(h128.byte_size == 16);
        assert(h128.bit_size == 128);

        hash_256 h256;
        assert(h256.byte_size == 32);
        assert(h256.bit_size == 256);

        hash_512 h512;
        assert(h512.byte_size == 64);
        assert(h512.bit_size == 512);
    }

    // Test operations with different sizes
    {
        hash_value<1> tiny;
        tiny.data[0] = 0xAB;

        auto not_tiny = ~tiny;
        assert(not_tiny.data[0] == 0x54);

        hash_value<128> large;
        assert(large.is_zero());
        for (auto& b : large.data) b = 0xFF;
        assert(large.is_ones());
    }

    std::cout << " PASSED\n";
}

// Test key derivation
void test_key_derivation() {
    std::cout << "Testing key derivation...";

    // Test basic derivation
    {
        key_derivation kd("master_key");

        auto h1 = kd.derive<32>(100);
        auto h2 = kd.derive<32>(100);
        auto h3 = kd.derive<32>(200);

        // Same input should produce same hash
        assert(h1 == h2);

        // Different input should produce different hash
        assert(h1 != h3);

        // Test key fingerprint consistency
        assert(kd.key_fingerprint() == kd.key_fingerprint());
    }

    // Test different master keys
    {
        key_derivation kd1("key1");
        key_derivation kd2("key2");

        auto h1 = kd1.derive<32>(std::string("data"));
        auto h2 = kd2.derive<32>(std::string("data"));

        // Different keys should produce different hashes for same data
        assert(h1 != h2);

        // Different key fingerprints
        assert(kd1.key_fingerprint() != kd2.key_fingerprint());
    }

    // Test various data types
    {
        key_derivation kd("test");

        auto h_int = kd.derive<16>(42);
        auto h_double = kd.derive<16>(3.14);
        auto h_string = kd.derive<16>(std::string("hello"));

        // All should produce valid hashes
        assert(!h_int.is_zero());
        assert(!h_double.is_zero());
        assert(!h_string.is_zero());

        // All should be different
        assert(h_int != h_double);
        assert(h_int != h_string);
        assert(h_double != h_string);
    }

    // Test different output sizes
    {
        key_derivation kd("size_test");

        auto h16 = kd.derive<16>(std::string("data"));
        auto h32 = kd.derive<32>(std::string("data"));
        auto h64 = kd.derive<64>(std::string("data"));

        assert(h16.byte_size == 16);
        assert(h32.byte_size == 32);
        assert(h64.byte_size == 64);

        // First 16 bytes might not match due to size-dependent derivation
        // but all should be non-zero
        assert(!h16.is_zero());
        assert(!h32.is_zero());
        assert(!h64.is_zero());
    }

    std::cout << " PASSED\n";
}

// Test edge cases and boundary conditions
void test_edge_cases() {
    std::cout << "Testing edge cases...";

    // Test with minimum size
    {
        hash_value<1> mini;
        mini.data[0] = 0x7F;

        auto doubled = mini | mini;
        assert(doubled == mini);

        auto inverted = ~mini;
        assert(inverted.data[0] == 0x80);
    }

    // Test operations preserve size
    {
        hash_value<7> odd_size;  // Odd size
        for (size_t i = 0; i < 7; ++i) {
            odd_size.data[i] = static_cast<uint8_t>(i * 37);
        }

        auto result = odd_size ^ odd_size;
        assert(result.byte_size == 7);
        assert(result.is_zero());
    }

    // Test boundary bit patterns
    {
        hash_value<8> h;

        // Alternating bits
        for (size_t i = 0; i < 8; ++i) {
            h.data[i] = (i % 2) ? 0xAA : 0x55;
        }

        auto inverted = ~h;
        for (size_t i = 0; i < 8; ++i) {
            assert(inverted.data[i] == ((i % 2) ? 0x55 : 0xAA));
        }
    }

    // Test associativity and commutativity
    {
        hash_value<4> a, b, c;
        a.data = {0x12, 0x34, 0x56, 0x78};
        b.data = {0xAB, 0xCD, 0xEF, 0x01};
        c.data = {0x23, 0x45, 0x67, 0x89};

        // XOR associativity: (a ^ b) ^ c == a ^ (b ^ c)
        auto left = (a ^ b) ^ c;
        auto right = a ^ (b ^ c);
        assert(left == right);

        // XOR commutativity: a ^ b == b ^ a
        assert((a ^ b) == (b ^ a));

        // AND associativity
        left = (a & b) & c;
        right = a & (b & c);
        assert(left == right);

        // OR associativity
        left = (a | b) | c;
        right = a | (b | c);
        assert(left == right);
    }

    // Test De Morgan's laws
    {
        hash_value<4> a, b;
        a.data = {0x0F, 0xF0, 0x33, 0xCC};
        b.data = {0xAA, 0x55, 0x66, 0x99};

        // ~(a & b) == (~a | ~b)
        auto left = ~(a & b);
        auto right = (~a) | (~b);
        assert(left == right);

        // ~(a | b) == (~a & ~b)
        left = ~(a | b);
        right = (~a) & (~b);
        assert(left == right);
    }

    std::cout << " PASSED\n";
}

// Test collision resistance properties
void test_collision_properties() {
    std::cout << "Testing collision properties...";

    key_derivation kd("collision_test");
    std::unordered_set<hash_value<16>, std::hash<hash_value<16>>> unique_hashes;

    // Generate many hashes and check for collisions
    const size_t num_samples = 1000;
    for (size_t i = 0; i < num_samples; ++i) {
        auto h = kd.derive<16>(i);
        unique_hashes.insert(h);
    }

    // Should have very few collisions (ideally none for small sample)
    assert(unique_hashes.size() == num_samples);

    // Test avalanche effect (small input change -> large output change)
    {
        auto h1 = kd.derive<32>(std::string("test"));
        auto h2 = kd.derive<32>(std::string("Test"));  // Only first letter capitalized

        // Count differing bits
        size_t diff_bits = 0;
        for (size_t i = 0; i < 32; ++i) {
            uint8_t xor_byte = h1.data[i] ^ h2.data[i];
            while (xor_byte) {
                diff_bits += xor_byte & 1;
                xor_byte >>= 1;
            }
        }

        // Should have many differing bits (avalanche effect)
        // Expect roughly 50% of bits to differ
        assert(diff_bits > 32 * 8 / 4);  // At least 25% different
    }

    std::cout << " PASSED\n";
}

// Test performance characteristics
void test_performance() {
    std::cout << "Testing performance characteristics...";

    // Test that operations are efficient
    const size_t iterations = 10000;

    // XOR performance
    {
        hash_value<32> h1, h2;
        for (auto& b : h1.data) b = rand() % 256;
        for (auto& b : h2.data) b = rand() % 256;

        for (size_t i = 0; i < iterations; ++i) {
            auto result = h1 ^ h2;
            (void)result;
        }
    }

    // Key derivation performance
    {
        key_derivation kd("perf_test");
        for (size_t i = 0; i < iterations; ++i) {
            auto h = kd.derive<32>(i);
            (void)h;
        }
    }

    std::cout << " PASSED\n";
}

int main() {
    std::cout << "\n=== Running Comprehensive Hash Types Tests ===\n\n";

    test_hash_value_construction();
    test_bitwise_operations();
    test_equality();
    test_different_sizes();
    test_key_derivation();
    test_edge_cases();
    test_collision_properties();
    test_performance();

    std::cout << "\nAll hash types tests passed!\n\n";
    return 0;
}