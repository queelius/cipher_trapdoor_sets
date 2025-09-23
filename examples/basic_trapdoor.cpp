#include <cipher_trapdoor_sets/cts.hpp>
#include <iostream>
#include <string>
#include <vector>

using namespace cts;

int main() {
    // Secret key for trapdoor generation
    std::string_view secret = "my-secret-key-2024";
    
    // Create trapdoor factory
    trapdoor_factory<32> factory(secret);
    
    std::cout << "=== Basic Trapdoor Operations ===\n\n";
    
    // Create trapdoors from different values
    auto td1 = factory.create(std::string("Alice"));
    auto td2 = factory.create(std::string("Bob"));
    auto td3 = factory.create(std::string("Alice"));  // Same as td1
    
    // Test equality (approximate due to hash collisions)
    auto eq1 = (td1 == td3);
    std::cout << "Trapdoor(Alice) == Trapdoor(Alice): "
              << (eq1.value() ? "true" : "false")
              << " (FPR: " << eq1.false_positive_rate() << ")\n";
    
    auto eq2 = (td1 == td2);
    std::cout << "Trapdoor(Alice) == Trapdoor(Bob): "
              << (eq2.value() ? "true" : "false")
              << " (FPR: " << eq2.false_positive_rate() << ")\n";
    
    // Demonstrate one-way property
    std::cout << "\nHash values (one-way - cannot reverse):\n";
    std::cout << "Alice hash: ";
    for (size_t i = 0; i < 8; ++i) {  // Show first 8 bytes
        printf("%02x", td1.hash().data[i]);
    }
    std::cout << "...\n";
    
    std::cout << "Bob hash:   ";
    for (size_t i = 0; i < 8; ++i) {
        printf("%02x", td2.hash().data[i]);
    }
    std::cout << "...\n";
    
    // Batch operations
    std::cout << "\n=== Batch Operations ===\n\n";
    
    std::vector<std::string> names = {"Alice", "Bob", "Charlie", "Diana"};
    auto batch_trapdoors = operations::batch_operations::batch_create_trapdoors(
        names, secret);
    
    std::cout << "Created " << batch_trapdoors.size() 
              << " trapdoors in batch\n";
    
    // Compare batch
    auto batch_compare = batch_trapdoors;
    batch_compare[2] = factory.create(std::string("Eve"));  // Change one
    
    auto results = operations::batch_operations::batch_equals(
        batch_trapdoors, batch_compare);
    
    for (size_t i = 0; i < names.size(); ++i) {
        std::cout << names[i] << " comparison: "
                  << (results[i].value() ? "match" : "different") << "\n";
    }
    
    return 0;
}