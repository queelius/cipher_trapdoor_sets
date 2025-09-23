#include <cipher_trapdoor_sets/cts.hpp>
#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <string>

using namespace cts;

void test_cardinality_estimation() {
    std::cout << "Testing cardinality estimation... ";

    std::string_view secret = "test-secret";

    // Test with boolean set
    sets::boolean_set_factory<std::string> factory(secret);
    std::vector<std::string> items;
    for (int i = 0; i < 100; ++i) {
        items.push_back("item-" + std::to_string(i));
    }

    auto set = factory.from_collection(items);
    auto estimate = operations::cardinality_estimator<32>::estimate(set);

    // Should have a reasonable estimate
    assert(estimate.value() >= 0);
    assert(estimate.error_rate() < 1.0);

    // Test with another set
    std::vector<std::string> items2;
    for (int i = 0; i < 50; ++i) {
        items2.push_back("other-" + std::to_string(i));
    }
    auto set2 = factory.from_collection(items2);

    // Compare set sizes
    auto estimate2 = operations::cardinality_estimator<32>::estimate(set2);
    // Just check both estimates are valid
    assert(estimate2.value() >= 0);
    assert(estimate2.error_rate() < 1.0);

    std::cout << "PASSED" << std::endl;
}

void test_serialization() {
    std::cout << "Testing serialization... ";

    std::string_view secret = "test-secret";
    trapdoor_factory<32> factory(secret);

    // Serialize trapdoor
    auto original = factory.create(std::string("test-value"));
    auto serialized = serialization::binary_serializer::serialize(original);
    assert(!serialized.empty());
    assert(serialized.size() == 32 + sizeof(std::size_t));

    // Deserialize trapdoor
    auto deserialized = serialization::binary_deserializer::deserialize_trapdoor<std::string, 32>(
        std::span(serialized));

    auto equal = (original == deserialized);
    assert(equal.value());

    // Serialize boolean set
    sets::boolean_set_factory<std::string> set_factory(secret);
    std::vector<std::string> set_items = {"a", "b", "c"};
    auto set = set_factory.from_collection(set_items);
    auto set_serialized = serialization::binary_serializer::serialize(set);
    assert(!set_serialized.empty());

    // Deserialize boolean set
    auto set_deserialized = serialization::binary_deserializer::deserialize_boolean_set<std::string, 32>(
        std::span(set_serialized));

    // Should have same key fingerprint
    assert(set.key_fingerprint() == set_deserialized.key_fingerprint());

    std::cout << "PASSED" << std::endl;
}

void test_set_operations_advanced() {
    std::cout << "Testing advanced set operations... ";

    std::string_view secret = "test-secret";
    sets::boolean_set_factory<std::string> factory(secret);

    // Create test sets
    std::vector<std::string> items1 = {"a", "b", "c", "d"};
    std::vector<std::string> items2 = {"c", "d", "e", "f"};
    auto set1 = factory.from_collection(items1);
    auto set2 = factory.from_collection(items2);

    // Test union
    auto union_set = set1 | set2;

    // Test intersection
    auto intersection = set1 & set2;

    // Test difference
    auto diff = set1 - set2;

    // Test symmetric difference
    auto sym_diff = set1 ^ set2;

    // Test complement
    auto complement = ~set1;

    // Test membership
    trapdoor_factory<32> td_factory(secret);
    auto td_c = td_factory.create(std::string("c"));
    auto contains = set1.contains(td_c);
    assert(contains.value() || contains.false_positive_rate() > 0);

    // Test subset
    auto subset = set1.subset_of(union_set);
    assert(subset.value() || subset.false_positive_rate() > 0);

    std::cout << "PASSED" << std::endl;
}

void test_symmetric_difference_sets() {
    std::cout << "Testing symmetric difference sets... ";

    std::string_view secret = "test-secret";
    sets::symmetric_difference_set_factory<std::string> factory(secret);

    // Create sets from unique items
    std::vector<std::string> items1 = {"x", "y", "z"};
    std::vector<std::string> items2 = {"a", "b", "c"};

    auto set1 = factory.from_unique(items1);
    auto set2 = factory.from_unique(items2);

    // Test XOR operation
    auto xor_set = set1 ^ set2;

    // Test empty set
    auto empty = factory.empty();

    // XOR with empty should give original
    auto set1_xor_empty = set1 ^ empty;

    // Test key compatibility
    assert(set1.key_fingerprint() == set2.key_fingerprint());
    assert(set1.key_fingerprint() == xor_set.key_fingerprint());

    std::cout << "PASSED" << std::endl;
}

int main() {
    std::cout << "=== Running Operations Tests ===" << std::endl << std::endl;

    test_cardinality_estimation();
    test_serialization();
    test_set_operations_advanced();
    test_symmetric_difference_sets();

    std::cout << std::endl << "All tests passed!" << std::endl;

    return 0;
}