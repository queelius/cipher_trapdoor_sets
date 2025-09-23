#include <cipher_trapdoor_sets/cts.hpp>
#include <iostream>
#include <set>
#include <string>

using namespace cts;

int main() {
    std::string_view secret = "set-operations-key";
    
    std::cout << "=== Symmetric Difference Set Operations ===\n\n";
    
    // Create factory for symmetric difference sets
    sets::symmetric_difference_set_factory<std::string> sym_factory(secret);
    
    // Create sets from collections
    std::set<std::string> team_a = {"Alice", "Bob", "Charlie"};
    std::set<std::string> team_b = {"Diana", "Eve", "Frank"};
    
    auto set_a = sym_factory.from_unique(team_a);
    auto set_b = sym_factory.from_unique(team_b);
    
    // Union (via XOR for disjoint sets)
    auto union_set = set_a ^ set_b;
    std::cout << "Created union of disjoint sets (6 members total)\n";
    
    // Test emptiness
    auto empty_set = sym_factory.empty();
    auto is_empty = empty_set.empty();
    std::cout << "Empty set test: " << (is_empty.value() ? "empty" : "not empty")
              << " (FPR: " << is_empty.false_positive_rate() << ")\n";
    
    // Self XOR results in empty set (group inverse property)
    auto self_xor = set_a ^ set_a;
    auto should_be_empty = self_xor.empty();
    std::cout << "A XOR A = empty: " << (should_be_empty.value() ? "yes" : "no") << "\n";
    
    std::cout << "\n=== Boolean Set Operations ===\n\n";
    
    // Create Boolean set factory
    sets::boolean_set_factory<int> bool_factory(secret, 3);  // 3 hash functions
    
    // Create sets
    std::vector<int> numbers1 = {1, 2, 3, 4, 5};
    std::vector<int> numbers2 = {4, 5, 6, 7, 8};
    
    auto bool_set1 = bool_factory.from_collection(numbers1);
    auto bool_set2 = bool_factory.from_collection(numbers2);
    
    // Set operations
    auto intersection = bool_set1 & bool_set2;  // Should contain {4, 5}
    auto union_bool = bool_set1 | bool_set2;    // Should contain {1..8}
    auto difference = bool_set1 - bool_set2;    // Should contain {1, 2, 3}
    auto sym_diff = bool_set1 ^ bool_set2;      // Should contain {1,2,3,6,7,8}
    
    std::cout << "Boolean set operations completed\n";
    
    // Test membership
    trapdoor_factory<32> td_factory(secret);
    auto td_4 = td_factory.create(4);
    auto td_9 = td_factory.create(9);
    
    auto contains_4 = intersection.contains(td_4);
    auto contains_9 = intersection.contains(td_9);
    
    std::cout << "Intersection contains 4: " 
              << (contains_4.value() ? "yes" : "no")
              << " (FPR: " << contains_4.false_positive_rate() << ")\n";
    std::cout << "Intersection contains 9: "
              << (contains_9.value() ? "yes" : "no")
              << " (FPR: " << contains_9.false_positive_rate() << ")\n";
    
    // Test subset relation
    auto is_subset = intersection.subset_of(bool_set1);
    std::cout << "Intersection ⊆ Set1: "
              << (is_subset.value() ? "yes" : "no")
              << " (FPR: " << is_subset.false_positive_rate() << ")\n";
    
    // Complement
    auto complement = ~bool_set1;
    std::cout << "\nCreated complement of Set1\n";
    
    // Universal set
    auto universal = bool_factory.universal();
    auto is_universal = universal.universal();
    std::cout << "Universal set test: "
              << (is_universal.value() ? "universal" : "not universal") << "\n";
    
    return 0;
}