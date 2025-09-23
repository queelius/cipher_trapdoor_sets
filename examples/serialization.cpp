#include <cipher_trapdoor_sets/cts.hpp>
#include <iostream>
#include <fstream>
#include <string>

using namespace cts;

void save_trapdoor_to_file(std::string const& filename, 
                          trapdoor<std::string, 32> const& td) {
    auto bytes = serialization::binary_serializer::serialize(td);
    std::ofstream file(filename, std::ios::binary);
    file.write(reinterpret_cast<char const*>(bytes.data()), bytes.size());
    std::cout << "Saved trapdoor to " << filename 
              << " (" << bytes.size() << " bytes)\n";
}

trapdoor<std::string, 32> load_trapdoor_from_file(std::string const& filename) {
    std::ifstream file(filename, std::ios::binary);
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
    
    auto td = serialization::binary_deserializer::deserialize_trapdoor<std::string, 32>(
        std::span(bytes));
    std::cout << "Loaded trapdoor from " << filename << "\n";
    return td;
}

int main() {
    std::string_view secret = "serialization-demo-key";
    
    std::cout << "=== Serialization Example ===\n\n";
    
    // Create trapdoors
    trapdoor_factory<32> factory(secret);
    auto td_original = factory.create(std::string("SecretData"));
    
    // Serialize to file
    save_trapdoor_to_file("trapdoor.bin", td_original);
    
    // Load from file
    auto td_loaded = load_trapdoor_from_file("trapdoor.bin");
    
    // Verify they're equal
    auto equal = (td_original == td_loaded);
    std::cout << "Original == Loaded: " 
              << (equal.value() ? "true" : "false") << "\n";
    
    std::cout << "\n=== Set Serialization ===\n\n";
    
    // Create and serialize a set
    sets::symmetric_difference_set_factory<int> set_factory(secret);
    std::vector<int> values = {10, 20, 30, 40, 50};
    auto set_original = set_factory.from_unique(values);
    
    // Serialize set
    auto set_bytes = serialization::binary_serializer::serialize(set_original);
    std::cout << "Serialized set to " << set_bytes.size() << " bytes\n";
    
    // Deserialize set
    auto set_loaded = serialization::binary_deserializer::
        deserialize_sym_diff_set<int, 32>(std::span(set_bytes));
    
    // Verify equality
    auto sets_equal = set_original.equals(set_loaded);
    std::cout << "Original set == Loaded set: "
              << (sets_equal.value() ? "true" : "false") << "\n";
    
    // Demonstrate network transmission simulation
    std::cout << "\n=== Network Transmission Simulation ===\n\n";
    
    // Alice creates a set
    sets::boolean_set_factory<std::string> alice_factory(secret);
    std::vector<std::string> alice_data = {"doc1", "doc2", "doc3"};
    auto alice_set = alice_factory.from_collection(alice_data);
    
    // Alice serializes and "sends" to Bob
    auto transmitted = serialization::binary_serializer::serialize(alice_set);
    std::cout << "Alice sends " << transmitted.size() 
              << " bytes (privacy preserved)\n";
    
    // Bob receives and deserializes
    // Note: Bob needs the same secret to perform operations
    sets::boolean_set_factory<std::string> bob_factory(secret);
    // In real scenario, Bob would deserialize the boolean_set
    // For this demo, we'll create Bob's set
    std::vector<std::string> bob_data = {"doc2", "doc3", "doc4"};
    auto bob_set = bob_factory.from_collection(bob_data);
    
    // Bob can compute intersection without seeing actual values
    auto shared_docs = alice_set & bob_set;
    std::cout << "Bob computed intersection without seeing Alice's data\n";
    
    // Test if specific document is in intersection
    auto td_doc2 = factory.create(std::string("doc2"));
    auto has_doc2 = shared_docs.contains(td_doc2);
    std::cout << "Intersection contains 'doc2': "
              << (has_doc2.value() ? "yes" : "no")
              << " (approximate)\n";
    
    // Clean up
    std::remove("trapdoor.bin");
    
    return 0;
}