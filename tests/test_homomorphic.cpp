#include <cipher_trapdoor_sets/cts.hpp>
#include <cassert>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>

using namespace cts;
using namespace cts::operations;

// Test homomorphic operations
void test_additive_trapdoor() {
    std::cout << "Testing additive trapdoor operations...";

    trapdoor_factory<32> factory("test-key");

    // Test basic construction
    {
        auto td = factory.create(10);
        additive_trapdoor<int> at(td, 10);

        assert(at.hash() == td.hash());
        assert(at.key_fingerprint() == td.key_fingerprint());
    }

    // Test addition
    {
        auto td1 = factory.create(10);
        auto td2 = factory.create(20);

        additive_trapdoor<int> at1(td1, 10);
        additive_trapdoor<int> at2(td2, 20);

        auto sum = at1 + at2;
        assert(sum.key_fingerprint() == at1.key_fingerprint());
    }

    // Test scalar multiplication
    {
        auto td = factory.create(5);
        additive_trapdoor<int> at(td, 5);

        auto scaled = at * 3;
        assert(scaled.key_fingerprint() == at.key_fingerprint());

        auto neg_scaled = at * -2;
        assert(neg_scaled.key_fingerprint() == at.key_fingerprint());
    }

    // Test with double values
    {
        auto td1 = factory.create(1.5);
        auto td2 = factory.create(2.5);

        additive_trapdoor<double> at1(td1, 1.5);
        additive_trapdoor<double> at2(td2, 2.5);

        auto sum = at1 + at2;
        (void)sum;
    }

    // Test incompatible keys error
    {
        trapdoor_factory<32> factory2("different-key");

        auto td1 = factory.create(5);
        auto td2 = factory2.create(5);

        additive_trapdoor<int> at1(td1, 5);
        additive_trapdoor<int> at2(td2, 5);

        bool threw = false;
        try {
            auto bad_sum = at1 + at2;
            (void)bad_sum;
        } catch (std::invalid_argument const&) {
            threw = true;
        }
        assert(threw);
    }

    std::cout << " PASSED\n";
}

// Test secure aggregator
void test_secure_aggregator() {
    std::cout << "Testing secure aggregator...";

    std::string_view secret = "test-secret";

    // Test sum aggregation
    {
        secure_aggregator<int> aggregator(secret);

        std::vector<int> values = {1, 2, 3, 4, 5};
        auto sum_result = aggregator.sum(values);

        assert(sum_result.value() == 15);  // 1+2+3+4+5 = 15
        assert(sum_result.error_rate() > 0);  // Should have some uncertainty
    }

    // Test average calculation
    {
        secure_aggregator<double> aggregator(secret);

        std::vector<double> values = {1.0, 2.0, 3.0, 4.0, 5.0};
        auto avg_result = aggregator.average(values);

        assert(std::abs(avg_result.value() - 3.0) < 0.001);  // Average should be 3.0
        assert(avg_result.error_rate() > 0);
    }

    // Test with empty container
    {
        secure_aggregator<int> aggregator(secret);

        std::vector<int> empty_values;
        auto sum_result = aggregator.sum(empty_values);

        assert(sum_result.value() == 0);
        assert(sum_result.error_rate() == 0.0);

        secure_aggregator<double> double_aggregator(secret);
        auto avg_result = double_aggregator.average(empty_values);
        assert(avg_result.value() == 0.0);
    }

    // Test with large values
    {
        secure_aggregator<long long> aggregator(secret);

        std::vector<long long> values;
        for (int i = 0; i < 100; ++i) {
            values.push_back(i * 1000000LL);
        }

        auto sum_result = aggregator.sum(values);
        long long expected = 0;
        for (auto v : values) expected += v;
        assert(sum_result.value() == expected);
    }

    std::cout << " PASSED\n";
}

// Test threshold scheme
void test_threshold_scheme() {
    std::cout << "Testing threshold scheme...";

    trapdoor_factory<32> factory("test-key");

    // Test basic threshold scheme
    {
        threshold_scheme<32> scheme(3, 5);  // 3-of-5 threshold

        auto td = factory.create(42);
        auto shares = scheme.create_shares(td);

        assert(shares.size() == 5);

        // Test reconstruction with exactly k shares
        std::vector<core::hash_value<32>> subset = {shares[0], shares[1], shares[2]};
        auto reconstructed = scheme.reconstruct<int>(subset, td.key_fingerprint());

        assert(reconstructed.key_fingerprint() == td.key_fingerprint());
    }

    // Test invalid threshold
    {
        bool threw = false;
        try {
            threshold_scheme<32> invalid_scheme(5, 3);  // k > n
        } catch (std::invalid_argument const&) {
            threw = true;
        }
        assert(threw);
    }

    // Test insufficient shares
    {
        threshold_scheme<32> scheme(3, 5);
        auto td = factory.create(100);
        auto shares = scheme.create_shares(td);

        std::vector<core::hash_value<32>> insufficient = {shares[0], shares[1]};

        bool threw = false;
        try {
            auto bad_reconstruct = scheme.reconstruct<int>(insufficient, td.key_fingerprint());
            (void)bad_reconstruct;
        } catch (std::invalid_argument const&) {
            threw = true;
        }
        assert(threw);
    }

    // Test threshold operation
    {
        threshold_scheme<32> scheme(2, 3);

        std::vector<trapdoor<int, 32>> participants;
        for (int i = 0; i < 3; ++i) {
            participants.push_back(factory.create(i));
        }

        auto operation = [](trapdoor<int, 32> const& td) {
            return td.hash().data[0] % 2 == 0;  // Simple predicate
        };

        auto result = scheme.threshold_operation(participants, operation);
        (void)result.value();
        assert(result.error_rate() >= 0.0);
    }

    std::cout << " PASSED\n";
}

// Test compound trapdoor
void test_compound_trapdoor() {
    std::cout << "Testing compound trapdoor...";

    trapdoor_factory<32> factory("test-key");

    // Test basic compound operations
    {
        compound_trapdoor<int> compound(factory.create(0).key_fingerprint());

        for (int i = 0; i < 5; ++i) {
            compound.add_component(factory.create(i));
        }

        auto multiplied = compound.multiply();
        assert(multiplied.key_fingerprint() == factory.create(0).key_fingerprint());
    }

    // Test incompatible key error
    {
        trapdoor_factory<32> factory2("different-key");

        compound_trapdoor<int> compound(factory.create(0).key_fingerprint());
        compound.add_component(factory.create(1));

        bool threw = false;
        try {
            compound.add_component(factory2.create(2));
        } catch (std::invalid_argument const&) {
            threw = true;
        }
        assert(threw);
    }

    // Test empty compound error
    {
        compound_trapdoor<int> compound(factory.create(0).key_fingerprint());

        bool threw = false;
        try {
            auto bad_multiply = compound.multiply();
            (void)bad_multiply;
        } catch (std::runtime_error const&) {
            threw = true;
        }
        assert(threw);
    }

    // Test all_satisfy predicate
    {
        compound_trapdoor<int> compound(factory.create(0).key_fingerprint());

        for (int i = 0; i < 3; ++i) {
            compound.add_component(factory.create(i * 2));  // Even numbers
        }

        auto predicate = [](trapdoor<int, 32> const& td) {
            return td.hash().data[0] != 0;  // Non-zero first byte
        };

        auto result = compound.all_satisfy(predicate);
        (void)result.value();
        assert(result.error_rate() > 0.0);
    }

    std::cout << " PASSED\n";
}

// Test edge cases
void test_homomorphic_edge_cases() {
    std::cout << "Testing homomorphic edge cases...";

    trapdoor_factory<32> factory("test-key");

    // Test with zero values
    {
        auto td = factory.create(0);
        additive_trapdoor<int> at(td, 0);

        auto scaled = at * 0;
        (void)scaled;
    }

    // Test with negative values
    {
        auto td1 = factory.create(-10);
        auto td2 = factory.create(-20);

        additive_trapdoor<int> at1(td1, -10);
        additive_trapdoor<int> at2(td2, -20);

        auto sum = at1 + at2;
        (void)sum;
    }

    // Test with maximum values
    {
        auto td = factory.create(std::numeric_limits<int>::max());
        additive_trapdoor<int> at(td, std::numeric_limits<int>::max());

        // Note: This might overflow in real arithmetic
        auto scaled = at * 1;
        (void)scaled;
    }

    // Test threshold schemes with edge cases
    {
        // 1-of-1 threshold
        threshold_scheme<32> scheme_1of1(1, 1);
        auto td = factory.create(42);
        auto shares = scheme_1of1.create_shares(td);
        assert(shares.size() == 1);

        // n-of-n threshold
        threshold_scheme<32> scheme_5of5(5, 5);
        auto shares_5of5 = scheme_5of5.create_shares(td);
        assert(shares_5of5.size() == 5);
    }

    std::cout << " PASSED\n";
}

int main() {
    std::cout << "\n=== Running Homomorphic Operations Tests ===\n\n";

    test_additive_trapdoor();
    test_secure_aggregator();
    test_threshold_scheme();
    test_compound_trapdoor();
    test_homomorphic_edge_cases();

    std::cout << "\nAll homomorphic tests passed!\n\n";
    return 0;
}