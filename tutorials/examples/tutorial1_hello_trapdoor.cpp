// Tutorial 1: Hello Trapdoor - Your First Privacy-Preserving Program
// Compile: g++ -std=c++17 -I../../include tutorial1_hello_trapdoor.cpp -o hello_trapdoor
// Run: ./hello_trapdoor

#include <cipher_trapdoor_sets/cts.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

using namespace cts;

void print_separator() {
    std::cout << "─────────────────────────────────────────────────\n";
}

int main() {
    std::cout << "╔═══════════════════════════════════════════════╗\n";
    std::cout << "║   HELLO TRAPDOOR - Privacy-Preserving Demo   ║\n";
    std::cout << "╚═══════════════════════════════════════════════╝\n\n";

    // ═══════════════════════════════════════════════════════════
    // PART 1: Creating Your First Trapdoor
    // ═══════════════════════════════════════════════════════════

    std::cout << "PART 1: Creating Your First Trapdoor\n";
    print_separator();

    // Secret key - in production, this would be securely stored
    std::string_view secret_key = "tutorial-secret-key-2025";

    // Create a trapdoor factory with 256-bit security (32 bytes)
    trapdoor_factory<32> factory(secret_key);

    std::cout << "✓ Trapdoor factory initialized\n";
    std::cout << "  Hash size: 256 bits (32 bytes)\n";
    std::cout << "  Key fingerprint: " << factory.key_fingerprint() << "\n\n";

    // Transform sensitive data into trapdoors
    std::string sensitive_email = "alice@private.com";
    auto td_email = factory.create(sensitive_email);

    std::cout << "✓ Created trapdoor for: " << sensitive_email << "\n";
    std::cout << "  Original data is now protected!\n\n";

    // ═══════════════════════════════════════════════════════════
    // PART 2: Demonstrating One-Way Property
    // ═══════════════════════════════════════════════════════════

    std::cout << "PART 2: One-Way Transformation\n";
    print_separator();

    std::cout << "Original value: " << sensitive_email << "\n";
    std::cout << "Trapdoor hash (hex): ";

    // Display first 16 bytes of hash
    const auto& hash = td_email.hash();
    for (size_t i = 0; i < 16; ++i) {
        printf("%02x", hash.data[i]);
    }
    std::cout << "...\n";

    std::cout << "\n⚠️  This hash CANNOT be reversed to get '"
              << sensitive_email << "'\n";
    std::cout << "   Reversal would require ~2^256 operations!\n\n";

    // ═══════════════════════════════════════════════════════════
    // PART 3: Privacy-Preserving Comparison
    // ═══════════════════════════════════════════════════════════

    std::cout << "PART 3: Privacy-Preserving Comparison\n";
    print_separator();

    // Create more trapdoors
    auto td_alice1 = factory.create(std::string("alice@private.com"));
    auto td_alice2 = factory.create(std::string("alice@private.com"));  // Same
    auto td_bob = factory.create(std::string("bob@private.com"));       // Different

    // Compare without revealing original values
    auto result_same = (td_alice1 == td_alice2);
    auto result_diff = (td_alice1 == td_bob);

    std::cout << "Comparing identical values:\n";
    std::cout << "  Result: " << (result_same.value() ? "MATCH ✓" : "DIFFERENT ✗") << "\n";
    std::cout << "  Confidence: " << std::fixed << std::setprecision(15)
              << (1.0 - result_same.false_positive_rate()) * 100 << "%\n\n";

    std::cout << "Comparing different values:\n";
    std::cout << "  Result: " << (result_diff.value() ? "MATCH ✓" : "DIFFERENT ✗") << "\n";
    std::cout << "  False positive rate: " << std::scientific
              << result_diff.false_positive_rate() << "\n\n";

    // ═══════════════════════════════════════════════════════════
    // PART 4: Practical Application - Anonymous Matching
    // ═══════════════════════════════════════════════════════════

    std::cout << "PART 4: Real-World Application\n";
    print_separator();
    std::cout << "Scenario: Find duplicate emails without seeing them\n\n";

    // Simulate a database of encrypted emails
    std::vector<std::string> email_database = {
        "alice@private.com",
        "bob@secure.net",
        "charlie@anonymous.org",
        "alice@private.com",     // Duplicate
        "diana@encrypted.io",
        "bob@secure.net"         // Another duplicate
    };

    std::cout << "Processing " << email_database.size() << " emails...\n";

    // Convert all to trapdoors
    std::vector<trapdoor<std::string, 32>> td_database;
    for (const auto& email : email_database) {
        td_database.push_back(factory.create(email));
    }

    // Find duplicates without knowing actual emails
    std::cout << "\nSearching for duplicates (privacy-preserved):\n";
    int duplicate_count = 0;

    for (size_t i = 0; i < td_database.size(); ++i) {
        for (size_t j = i + 1; j < td_database.size(); ++j) {
            auto match = (td_database[i] == td_database[j]);
            if (match.value()) {
                std::cout << "  ✓ Found duplicate: Entry " << i
                          << " matches Entry " << j << "\n";
                duplicate_count++;
            }
        }
    }

    std::cout << "\nTotal duplicates found: " << duplicate_count << "\n";
    std::cout << "Original emails remain private throughout!\n\n";

    // ═══════════════════════════════════════════════════════════
    // PART 5: Key Independence
    // ═══════════════════════════════════════════════════════════

    std::cout << "PART 5: Key Independence\n";
    print_separator();

    // Different keys produce completely different trapdoors
    trapdoor_factory<32> factory2("different-key-2025");

    auto td_with_key1 = factory.create(std::string("same-data"));
    auto td_with_key2 = factory2.create(std::string("same-data"));

    std::cout << "Same data, different keys:\n";
    std::cout << "  Key 1 hash: ";
    for (size_t i = 0; i < 8; ++i) {
        printf("%02x", td_with_key1.hash().data[i]);
    }
    std::cout << "...\n";

    std::cout << "  Key 2 hash: ";
    for (size_t i = 0; i < 8; ++i) {
        printf("%02x", td_with_key2.hash().data[i]);
    }
    std::cout << "...\n";

    std::cout << "\n✓ Different keys = Completely different trapdoors\n";
    std::cout << "  This provides cryptographic isolation\n\n";

    // ═══════════════════════════════════════════════════════════
    // Summary
    // ═══════════════════════════════════════════════════════════

    std::cout << "╔═══════════════════════════════════════════════╗\n";
    std::cout << "║                   SUMMARY                     ║\n";
    std::cout << "╚═══════════════════════════════════════════════╝\n\n";

    std::cout << "What you learned:\n";
    std::cout << "  ✓ Trapdoors are one-way transformations\n";
    std::cout << "  ✓ Original data cannot be recovered from hashes\n";
    std::cout << "  ✓ Can compare encrypted values privately\n";
    std::cout << "  ✓ Different keys provide isolation\n";
    std::cout << "  ✓ Real applications: duplicate detection, matching\n\n";

    std::cout << "Next steps:\n";
    std::cout << "  → Try Tutorial 2 for advanced operations\n";
    std::cout << "  → Experiment with different data types\n";
    std::cout << "  → Build your own privacy-preserving application\n\n";

    return 0;
}