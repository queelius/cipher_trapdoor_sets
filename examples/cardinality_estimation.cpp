#include <cipher_trapdoor_sets/cts.hpp>
#include <iostream>
#include <vector>
#include <random>

using namespace cts;

int main() {
    std::cout << "=== Cardinality Estimation Example ===\n\n";

    std::string_view secret = "cardinality-demo-key";

    // Example 1: HyperLogLog-style estimation
    std::cout << "1. HyperLogLog-style Cardinality Estimation:\n";
    {
        sets::boolean_set_factory<int> factory(secret);

        // Create sets of different sizes
        std::vector<int> small_set = {1, 2, 3, 4, 5};
        std::vector<int> medium_set;
        for (int i = 0; i < 100; ++i) {
            medium_set.push_back(i);
        }

        std::vector<int> large_set;
        for (int i = 0; i < 1000; ++i) {
            large_set.push_back(i * 7);  // Spread out values
        }

        auto small = factory.from_collection(small_set);
        auto medium = factory.from_collection(medium_set);
        auto large = factory.from_collection(large_set);

        auto small_card = operations::cardinality_estimator<>::estimate(small);
        auto medium_card = operations::cardinality_estimator<>::estimate(medium);
        auto large_card = operations::cardinality_estimator<>::estimate(large);

        std::cout << "  Small set (actual: " << small_set.size() << "): "
                  << "estimated = " << small_card.value()
                  << ", error rate = " << small_card.error_rate() << "\n";

        std::cout << "  Medium set (actual: " << medium_set.size() << "): "
                  << "estimated = " << medium_card.value()
                  << ", error rate = " << medium_card.error_rate() << "\n";

        std::cout << "  Large set (actual: " << large_set.size() << "): "
                  << "estimated = " << large_card.value()
                  << ", error rate = " << large_card.error_rate() << "\n";
    }

    // Example 2: Comparing cardinalities
    std::cout << "\n2. Comparing Set Cardinalities:\n";
    {
        sets::symmetric_difference_set_factory<std::string> factory(secret);

        std::vector<std::string> docs_a = {"doc1", "doc2", "doc3", "doc4", "doc5"};
        std::vector<std::string> docs_b = {"doc3", "doc4", "doc5", "doc6", "doc7", "doc8", "doc9"};

        auto set_a = factory.from_unique(docs_a);
        auto set_b = factory.from_unique(docs_b);

        auto comparison = operations::cardinality_estimator<>::compare_cardinality(set_a, set_b);

        std::cout << "  Set A has " << docs_a.size() << " documents\n";
        std::cout << "  Set B has " << docs_b.size() << " documents\n";
        std::cout << "  Comparison result: ";

        if (comparison.value() < 0) {
            std::cout << "|A| < |B|";
        } else if (comparison.value() > 0) {
            std::cout << "|A| > |B|";
        } else {
            std::cout << "|A| ≈ |B|";
        }
        std::cout << " (confidence: " << (1.0 - comparison.error_rate()) * 100 << "%)\n";
    }

    // Example 3: Union cardinality estimation
    std::cout << "\n3. Union Cardinality (Inclusion-Exclusion):\n";
    {
        sets::boolean_set_factory<int> factory(secret);

        std::vector<int> set_a = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        std::vector<int> set_b = {6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

        auto bool_a = factory.from_collection(set_a);
        auto bool_b = factory.from_collection(set_b);

        auto union_card = operations::cardinality_estimator<>::estimate_union(bool_a, bool_b);

        std::cout << "  Set A: {1..10}, size = " << set_a.size() << "\n";
        std::cout << "  Set B: {6..15}, size = " << set_b.size() << "\n";
        std::cout << "  Expected union size: 15\n";
        std::cout << "  Estimated union size: " << union_card.value()
                  << " (error: " << union_card.error_rate() * 100 << "%)\n";
    }

    // Example 4: Linear counting for small sets
    std::cout << "\n4. Linear Counting (Small Sets):\n";
    {
        sets::symmetric_difference_set_factory<std::string> factory(secret);

        std::vector<std::string> tiny_set = {"apple", "banana", "cherry"};
        auto sym_set = factory.from_unique(tiny_set);

        auto linear_est = operations::linear_counter<>::estimate(sym_set);
        auto hyperlog_est = operations::cardinality_estimator<>::estimate(sym_set);

        std::cout << "  Tiny set size: " << tiny_set.size() << "\n";
        std::cout << "  Linear counting estimate: " << linear_est.value()
                  << " (error: " << linear_est.error_rate() << ")\n";
        std::cout << "  HyperLogLog estimate: " << hyperlog_est.value()
                  << " (error: " << hyperlog_est.error_rate() << ")\n";
        std::cout << "  Linear counting is typically more accurate for small sets\n";
    }

    // Example 5: Adaptive cardinality estimation
    std::cout << "\n5. Adaptive Cardinality (Automatic Algorithm Selection):\n";
    {
        sets::boolean_set_factory<int> factory(secret);

        // Test with various set sizes
        std::vector<std::size_t> sizes = {5, 50, 500, 5000};

        for (auto size : sizes) {
            std::vector<int> test_set;
            for (std::size_t i = 0; i < size; ++i) {
                test_set.push_back(i);
            }

            auto bool_set = factory.from_collection(test_set);
            auto estimate = operations::adaptive_cardinality<>::estimate(bool_set);

            std::cout << "  Set size " << size << ": estimated = " << estimate.value()
                      << ", relative error = "
                      << std::abs(static_cast<double>(estimate.value()) - size) / size * 100
                      << "%\n";
        }
    }

    // Example 6: Privacy-preserving count without revealing elements
    std::cout << "\n6. Privacy-Preserving Element Count:\n";
    {
        sets::boolean_set_factory<std::string> factory(secret);

        // Alice has a set of sensitive documents
        std::vector<std::string> alice_docs = {
            "medical_record_001.pdf",
            "medical_record_002.pdf",
            "financial_statement.xlsx",
            "personal_diary.txt",
            "tax_return_2023.pdf"
        };

        auto alice_set = factory.from_collection(alice_docs);

        // Alice can share the trapdoored set without revealing documents
        std::cout << "  Alice has sensitive documents (not revealed)\n";
        std::cout << "  She shares only the trapdoored set\n";

        // Bob can estimate the count without seeing the documents
        auto doc_count = operations::cardinality_estimator<>::estimate(alice_set);
        std::cout << "  Bob estimates Alice has approximately "
                  << doc_count.value() << " documents\n";
        std::cout << "  Actual count: " << alice_docs.size() << "\n";
        std::cout << "  Privacy is preserved - documents remain hidden\n";
    }

    return 0;
}