#include <cipher_trapdoor_sets/cts.hpp>
#include <cassert>
#include <iostream>
#include <string>

using namespace cts;

void test_trapdoor_creation() {
    std::cout << "Testing trapdoor creation...";
    
    trapdoor_factory<32> factory("test-key");
    auto td1 = factory.create(42);
    auto td2 = factory.create(42);
    auto td3 = factory.create(43);
    
    // Same input should produce same hash
    assert(td1.hash() == td2.hash());
    assert(td1.key_fingerprint() == td2.key_fingerprint());
    
    // Different input should produce different hash (with high probability)
    assert(td1.hash() != td3.hash());
    
    std::cout << " PASSED\n";
}

void test_trapdoor_equality() {
    std::cout << "Testing trapdoor equality...";
    
    trapdoor_factory<32> factory("test-key");
    auto td1 = factory.create(std::string("test"));
    auto td2 = factory.create(std::string("test"));
    auto td3 = factory.create(std::string("different"));
    
    auto eq1 = (td1 == td2);
    assert(eq1.value() == true);
    assert(eq1.false_positive_rate() > 0);
    assert(eq1.false_negative_rate() == 0);
    
    auto eq2 = (td1 == td3);
    assert(eq2.value() == false);
    
    std::cout << " PASSED\n";
}

void test_different_keys() {
    std::cout << "Testing different keys...";
    
    trapdoor_factory<32> factory1("key1");
    trapdoor_factory<32> factory2("key2");
    
    auto td1 = factory1.create(100);
    auto td2 = factory2.create(100);
    
    // Same value with different keys should produce different hashes
    assert(td1.hash() != td2.hash());
    assert(td1.key_fingerprint() != td2.key_fingerprint());
    assert(!td1.compatible_with(td2));
    
    std::cout << " PASSED\n";
}

void test_hash_operations() {
    std::cout << "Testing hash operations...";
    
    core::hash_value<16> h1, h2;
    for (size_t i = 0; i < 16; ++i) {
        h1.data[i] = i;
        h2.data[i] = i * 2;
    }
    
    auto h_xor = h1 ^ h2;
    auto h_and = h1 & h2;
    auto h_or = h1 | h2;
    auto h_not = ~h1;
    
    // Test XOR
    for (size_t i = 0; i < 16; ++i) {
        assert(h_xor.data[i] == (h1.data[i] ^ h2.data[i]));
    }
    
    // Test zero detection
    core::hash_value<16> zero;
    assert(zero.is_zero());
    assert(!h1.is_zero());
    
    std::cout << " PASSED\n";
}

void test_approximate_values() {
    std::cout << "Testing approximate values...";
    
    core::approximate_bool true_exact(true, 0.0, 0.0);
    assert(true_exact.is_exact());
    assert(true_exact.value() == true);
    
    core::approximate_bool false_approx(false, 0.1, 0.2);
    assert(!false_approx.is_exact());
    assert(false_approx.error_rate() == 0.2);
    
    // Test logical operations
    auto and_result = true_exact && false_approx;
    assert(and_result.value() == false);
    assert(and_result.error_rate() > 0);
    
    auto not_result = !false_approx;
    assert(not_result.value() == true);
    assert(not_result.false_positive_rate() == false_approx.false_negative_rate());
    
    std::cout << " PASSED\n";
}

int main() {
    std::cout << "\n=== Running Trapdoor Tests ===\n\n";
    
    test_trapdoor_creation();
    test_trapdoor_equality();
    test_different_keys();
    test_hash_operations();
    test_approximate_values();
    
    std::cout << "\nAll tests passed!\n\n";
    return 0;
}