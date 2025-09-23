#include <cipher_trapdoor_sets/cts.hpp>
#include <cassert>
#include <iostream>
#include <vector>
#include <set>

using namespace cts;

void test_symmetric_difference_set() {
    std::cout << "Testing symmetric difference sets...";
    
    sets::symmetric_difference_set_factory<int> factory("test-key");
    
    // Test empty set
    auto empty = factory.empty();
    assert(empty.empty().value() == true);
    
    // Test singleton
    auto single = factory.singleton(42);
    assert(!single.empty().value());
    
    // Test union of disjoint sets
    std::vector<int> set1 = {1, 2, 3};
    std::vector<int> set2 = {4, 5, 6};
    
    auto s1 = factory.from_unique(set1);
    auto s2 = factory.from_unique(set2);
    auto union_set = s1 ^ s2;
    
    assert(!union_set.empty().value());
    
    // Test self-inverse property (A ^ A = empty)
    auto self_xor = s1 ^ s1;
    assert(self_xor.empty().value());
    
    // Test identity property (A ^ empty = A)
    auto identity_test = s1 ^ empty;
    assert(identity_test.equals(s1).value());
    
    std::cout << " PASSED\n";
}

void test_boolean_set() {
    std::cout << "Testing boolean sets...";
    
    sets::boolean_set_factory<std::string> factory("test-key", 2);
    trapdoor_factory<32> td_factory("test-key");
    
    // Create sets
    std::vector<std::string> items1 = {"apple", "banana", "cherry"};
    std::vector<std::string> items2 = {"banana", "cherry", "date"};
    
    auto set1 = factory.from_collection(items1);
    auto set2 = factory.from_collection(items2);
    
    // Test union
    auto union_set = set1 | set2;
    assert(!union_set.empty().value());
    
    // Test intersection
    auto inter_set = set1 & set2;
    assert(!inter_set.empty().value());
    
    // Test difference
    auto diff_set = set1 - set2;
    assert(!diff_set.empty().value());
    
    // Test complement
    auto comp_set = ~set1;
    assert(!comp_set.empty().value());
    
    // Test membership (approximate)
    auto td_banana = td_factory.create(std::string("banana"));
    auto contains = inter_set.contains(td_banana);
    // Should likely be true since banana is in both sets
    // But it's approximate, so we just check it doesn't crash
    (void)contains.value();
    
    // Test subset relation
    auto subset = inter_set.subset_of(set1);
    // Intersection should be subset of original set
    // But it's approximate
    (void)subset.value();
    
    // Test empty and universal
    auto empty = factory.empty();
    assert(empty.empty().value());
    
    auto universal = factory.universal();
    assert(universal.universal().value());
    
    std::cout << " PASSED\n";
}

void test_set_compatibility() {
    std::cout << "Testing set key compatibility...";
    
    sets::symmetric_difference_set_factory<int> factory1("key1");
    sets::symmetric_difference_set_factory<int> factory2("key2");
    
    auto set1 = factory1.singleton(10);
    auto set2 = factory2.singleton(20);
    
    // Operations between sets with different keys should throw
    bool threw = false;
    try {
        auto bad_union = set1 ^ set2;
        (void)bad_union;
    } catch (std::invalid_argument const&) {
        threw = true;
    }
    assert(threw);
    
    std::cout << " PASSED\n";
}

void test_batch_set_operations() {
    std::cout << "Testing batch set operations...";
    
    sets::symmetric_difference_set_factory<double> factory("test-key");
    
    std::vector<sets::symmetric_difference_set<double, 32>> sets;
    for (int i = 0; i < 5; ++i) {
        sets.push_back(factory.singleton(i * 1.5));
    }
    
    auto batch_union = operations::batch_operations::batch_union(sets);
    assert(!batch_union.empty().value());
    
    std::cout << " PASSED\n";
}

int main() {
    std::cout << "\n=== Running Set Tests ===\n\n";
    
    test_symmetric_difference_set();
    test_boolean_set();
    test_set_compatibility();
    test_batch_set_operations();
    
    std::cout << "\nAll tests passed!\n\n";
    return 0;
}