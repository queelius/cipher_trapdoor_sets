#include <cipher_trapdoor_sets/cts.hpp>
#include <cassert>
#include <iostream>
#include <vector>
#include <random>
#include <sstream>
#include <limits>
#include <cstring>
#include <set>
#include <iomanip>

using namespace std::string_view_literals;

using namespace cts;

// Test hash_value operations comprehensively
void test_hash_value_operations() {
    std::cout << "Testing hash_value operations comprehensively...";

    // Test different hash sizes
    {
        core::hash_value<16> h16;
        core::hash_value<32> h32;
        core::hash_value<64> h64;

        // Test zero initialization
        assert(h16.is_zero());
        assert(h32.is_zero());
        assert(h64.is_zero());

        // Test non-zero detection
        h16.data[0] = 1;
        assert(!h16.is_zero());

        h32.data[31] = 255;
        assert(!h32.is_zero());
    }

    // Test bitwise operations exhaustively
    {
        core::hash_value<8> a, b;
        for (size_t i = 0; i < 8; ++i) {
            a.data[i] = i;
            b.data[i] = (i * 3) % 256;
        }

        // Test XOR
        auto xor_result = a ^ b;
        for (size_t i = 0; i < 8; ++i) {
            assert(xor_result.data[i] == (a.data[i] ^ b.data[i]));
        }

        // Test AND
        auto and_result = a & b;
        for (size_t i = 0; i < 8; ++i) {
            assert(and_result.data[i] == (a.data[i] & b.data[i]));
        }

        // Test OR
        auto or_result = a | b;
        for (size_t i = 0; i < 8; ++i) {
            assert(or_result.data[i] == (a.data[i] | b.data[i]));
        }

        // Test NOT
        auto not_result = ~a;
        for (size_t i = 0; i < 8; ++i) {
            assert(not_result.data[i] == static_cast<uint8_t>(~a.data[i]));
        }
    }

    // Test equality and inequality
    {
        core::hash_value<16> h1, h2, h3;

        // Equal hashes
        for (size_t i = 0; i < 16; ++i) {
            h1.data[i] = i * 2;
            h2.data[i] = i * 2;
            h3.data[i] = i * 3;
        }

        assert(h1 == h2);
        assert(!(h1 != h2));
        assert(h1 != h3);
        assert(!(h1 == h3));
    }

    // Test is_ones
    {
        core::hash_value<4> h;
        assert(!h.is_ones());  // Default is zeros

        for (size_t i = 0; i < 4; ++i) {
            h.data[i] = 0xFF;
        }
        assert(h.is_ones());

        h.data[2] = 0xFE;  // One bit not set
        assert(!h.is_ones());
    }

    std::cout << " PASSED\n";
}

// Test approximate_value operations comprehensively
void test_approximate_value_operations() {
    std::cout << "Testing approximate_value operations comprehensively...";

    // Test boolean approximate values
    {
        core::approximate_bool true_exact(true, 0.0, 0.0);
        core::approximate_bool false_exact(false, 0.0, 0.0);
        core::approximate_bool true_approx(true, 0.1, 0.05);
        core::approximate_bool false_approx(false, 0.2, 0.15);

        // Test exactness
        assert(true_exact.is_exact());
        assert(false_exact.is_exact());
        assert(!true_approx.is_exact());
        assert(!false_approx.is_exact());

        // Test error rates
        assert(true_exact.error_rate() == 0.0);
        assert(true_approx.error_rate() == 0.1);
        assert(false_approx.error_rate() == 0.2);

        // Test AND operations
        auto and1 = true_exact && false_exact;
        assert(and1.value() == false);
        assert(and1.is_exact());

        auto and2 = true_approx && true_approx;
        assert(and2.value() == true);
        assert(!and2.is_exact());
        assert(and2.error_rate() > true_approx.error_rate());

        auto and3 = true_approx && false_approx;
        assert(and3.value() == false);
        assert(!and3.is_exact());

        // Test OR operations
        auto or1 = true_exact || false_exact;
        assert(or1.value() == true);
        assert(or1.is_exact());

        auto or2 = false_approx || false_approx;
        assert(or2.value() == false);
        assert(!or2.is_exact());

        // Test NOT operations
        auto not1 = !true_exact;
        assert(not1.value() == false);
        assert(not1.is_exact());

        auto not2 = !true_approx;
        assert(not2.value() == false);
        assert(!not2.is_exact());
        assert(not2.false_positive_rate() == true_approx.false_negative_rate());
        assert(not2.false_negative_rate() == true_approx.false_positive_rate());

        // Test combining error rates in complex expressions
        auto complex = (true_approx && false_approx) || true_exact;
        assert(complex.value() == true);

        // Test chained operations
        auto chained = !(!true_approx);
        assert(chained.value() == true);
        assert(!chained.is_exact());
    }

    // Test numeric approximate values
    {
        core::approximate_value<int> exact_10(10, 0.0);
        core::approximate_value<int> approx_20(20, 0.1);
        core::approximate_value<double> exact_pi(3.14159, 0.0);
        core::approximate_value<double> approx_e(2.71828, 0.05);

        assert(exact_10.is_exact());
        assert(!approx_20.is_exact());
        assert(exact_pi.is_exact());
        assert(!approx_e.is_exact());

        assert(exact_10.value() == 10);
        assert(approx_20.value() == 20);
        assert(exact_pi.value() == 3.14159);

        // Test combining error rates
        core::approximate_value<int> combined(30, 0.0);
        combined = core::approximate_value<int>(
            exact_10.value() + approx_20.value(),
            std::max(exact_10.error_rate(), approx_20.error_rate())
        );
        assert(combined.value() == 30);
        assert(combined.error_rate() == 0.1);
    }

    // Test edge cases
    {
        // Maximum error rates
        core::approximate_bool max_error(true, 1.0, 1.0);
        assert(max_error.error_rate() == 1.0);

        // Zero values
        core::approximate_value<int> zero(0, 0.0);
        assert(zero.value() == 0);
        assert(zero.is_exact());

        // Negative values
        core::approximate_value<int> negative(-100, 0.3);
        assert(negative.value() == -100);
        assert(negative.error_rate() == 0.3);
    }

    std::cout << " PASSED\n";
}

// Test hash collision properties
void test_hash_collisions() {
    std::cout << "Testing hash collision properties...";

    trapdoor_factory<32> factory("test-key");

    // Test that different inputs produce different hashes with high probability
    std::set<std::string> hash_set;
    const int num_samples = 1000;
    int collisions = 0;

    for (int i = 0; i < num_samples; ++i) {
        auto td = factory.create(i);
        // Convert hash to string for comparison
        std::stringstream ss;
        for (size_t j = 0; j < 32; ++j) {
            ss << std::hex << std::setw(2) << std::setfill('0')
               << static_cast<int>(td.hash().data[j]);
        }
        std::string hash_str = ss.str();

        if (hash_set.find(hash_str) != hash_set.end()) {
            collisions++;
        }
        hash_set.insert(hash_str);
    }

    // Should have very few collisions for small sample size
    assert(collisions < num_samples * 0.01);  // Less than 1% collision rate

    std::cout << " PASSED\n";
}

// Test edge cases and boundary conditions
void test_edge_cases() {
    std::cout << "Testing edge cases and boundary conditions...";

    // Test with empty strings
    {
        trapdoor_factory<32> factory("");
        auto td1 = factory.create(std::string(""));
        auto td2 = factory.create(std::string(""));
        assert(td1.hash() == td2.hash());
    }

    // Test with very long keys
    {
        std::string long_key(10000, 'x');
        trapdoor_factory<32> factory(long_key);
        auto td = factory.create(42);
        // Should not crash
        (void)td.hash();
    }

    // Test with special characters
    {
        trapdoor_factory<32> factory("\0\n\r\t\x01\xff"sv);
        auto td = factory.create(std::string("\0\n\r\t", 4));
        // Should handle binary data
        (void)td.hash();
    }

    // Test maximum values
    {
        trapdoor_factory<32> factory("test");
        auto td_max_int = factory.create(std::numeric_limits<int>::max());
        auto td_min_int = factory.create(std::numeric_limits<int>::min());
        assert(td_max_int.hash() != td_min_int.hash());

        auto td_max_size = factory.create(std::numeric_limits<size_t>::max());
        (void)td_max_size.hash();
    }

    // Test floating point edge cases
    {
        trapdoor_factory<32> factory("test");
        auto td_nan = factory.create(std::numeric_limits<double>::quiet_NaN());
        auto td_inf = factory.create(std::numeric_limits<double>::infinity());
        auto td_neg_inf = factory.create(-std::numeric_limits<double>::infinity());

        // These should all produce valid hashes
        (void)td_nan.hash();
        (void)td_inf.hash();
        (void)td_neg_inf.hash();
    }

    std::cout << " PASSED\n";
}

// Test hash distribution properties
void test_hash_distribution() {
    std::cout << "Testing hash distribution properties...";

    trapdoor_factory<32> factory("test-key");

    // Test that different inputs produce different hashes
    const int num_samples = 100;
    std::set<std::string> unique_hashes;

    for (int i = 0; i < num_samples; ++i) {
        auto td = factory.create(i);

        // Convert hash to string
        std::stringstream ss;
        for (size_t j = 0; j < 32; ++j) {
            ss << std::hex << std::setw(2) << std::setfill('0')
               << static_cast<int>(td.hash().data[j]);
        }
        unique_hashes.insert(ss.str());
    }

    // Should have mostly unique hashes
    assert(unique_hashes.size() > num_samples * 0.95);  // At least 95% unique

    // Test that hashes span the value space
    bool has_low_byte = false;
    bool has_high_byte = false;

    for (int i = 0; i < num_samples; ++i) {
        auto td = factory.create(i * 1000);  // Spread out the inputs
        if (td.hash().data[0] < 128) has_low_byte = true;
        if (td.hash().data[0] >= 128) has_high_byte = true;
    }

    assert(has_low_byte && has_high_byte);  // Should use full byte range

    std::cout << " PASSED\n";
}

// Test various data types
void test_various_data_types() {
    std::cout << "Testing various data types...";

    trapdoor_factory<32> factory("test-key");

    // Test with different integer types
    {
        auto td_i8 = factory.create(int8_t(-128));
        auto td_u8 = factory.create(uint8_t(255));
        auto td_i16 = factory.create(int16_t(-32768));
        auto td_u16 = factory.create(uint16_t(65535));
        auto td_i32 = factory.create(int32_t(-2147483648));
        auto td_u32 = factory.create(uint32_t(4294967295));
        auto td_i64 = factory.create(int64_t(std::numeric_limits<int64_t>::min()));
        auto td_u64 = factory.create(uint64_t(18446744073709551615ULL));

        // All should produce different hashes
        std::set<std::string> hashes;
        // Helper to convert hash to string
        auto hash_to_str = [](auto const& td) {
            std::stringstream ss;
            for (size_t j = 0; j < 32; ++j) {
                ss << std::hex << std::setw(2) << std::setfill('0')
                   << static_cast<int>(td.hash().data[j]);
            }
            return ss.str();
        };

        hashes.insert(hash_to_str(td_i8));
        hashes.insert(hash_to_str(td_u8));
        hashes.insert(hash_to_str(td_i16));
        hashes.insert(hash_to_str(td_u16));
        hashes.insert(hash_to_str(td_i32));
        hashes.insert(hash_to_str(td_u32));
        hashes.insert(hash_to_str(td_i64));
        hashes.insert(hash_to_str(td_u64));

        assert(hashes.size() == 8);
    }

    // Test with different single values
    {
        auto td1 = factory.create(12345);
        auto td2 = factory.create(12346);

        assert(td1.hash() != td2.hash());
    }

    // Test with custom types (using string representation)
    {
        struct Point {
            int x, y;
            operator std::string() const {
                return std::to_string(x) + "," + std::to_string(y);
            }
        };

        Point p1{10, 20};
        Point p2{10, 21};

        auto td_p1 = factory.create(std::string(p1));
        auto td_p2 = factory.create(std::string(p2));

        assert(td_p1.hash() != td_p2.hash());
    }

    std::cout << " PASSED\n";
}

int main() {
    std::cout << "\n=== Running Comprehensive Core Tests ===\n\n";

    test_hash_value_operations();
    test_approximate_value_operations();
    test_hash_collisions();
    test_edge_cases();
    test_hash_distribution();
    test_various_data_types();

    std::cout << "\nAll core tests passed!\n\n";
    return 0;
}