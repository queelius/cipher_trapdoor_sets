#include <cipher_trapdoor_sets/cts.hpp>
#include <iostream>
#include <vector>
#include <string>

using namespace cts;

int main() {
    std::cout << "=== Set Similarity Search Example ===\n\n";

    std::string_view secret = "similarity-demo-key";

    // Example 1: Jaccard similarity
    std::cout << "1. Jaccard Similarity Between Document Sets:\n";
    {
        sets::boolean_set_factory<std::string> factory(secret);

        // Two users' document collections
        std::vector<std::string> user_a_docs = {
            "report.pdf", "presentation.pptx", "budget.xlsx",
            "notes.txt", "proposal.docx"
        };

        std::vector<std::string> user_b_docs = {
            "presentation.pptx", "budget.xlsx", "summary.pdf",
            "proposal.docx", "timeline.xlsx"
        };

        auto set_a = factory.from_collection(user_a_docs);
        auto set_b = factory.from_collection(user_b_docs);

        auto similarity = operations::jaccard_similarity<>::estimate(set_a, set_b);

        std::cout << "  User A has " << user_a_docs.size() << " documents\n";
        std::cout << "  User B has " << user_b_docs.size() << " documents\n";
        std::cout << "  Jaccard similarity: " << similarity.value()
                  << " (error: ±" << similarity.error_rate() << ")\n";
        std::cout << "  Interpretation: "
                  << (similarity.value() > 0.5 ? "High" :
                      similarity.value() > 0.2 ? "Moderate" : "Low")
                  << " similarity\n";
    }

    // Example 2: MinHash signatures
    std::cout << "\n2. MinHash for Scalable Similarity:\n";
    {
        operations::minhash<std::string, 32, 128> minhash_engine(secret, 128);

        // Large document sets
        std::vector<std::string> corpus_a, corpus_b, corpus_c;

        // Generate synthetic documents
        for (int i = 0; i < 100; ++i) {
            corpus_a.push_back("doc_a_" + std::to_string(i));
            if (i < 60) {  // 60% overlap with A
                corpus_b.push_back("doc_a_" + std::to_string(i));
            } else {
                corpus_b.push_back("doc_b_" + std::to_string(i));
            }
            if (i < 20) {  // 20% overlap with A
                corpus_c.push_back("doc_a_" + std::to_string(i));
            } else {
                corpus_c.push_back("doc_c_" + std::to_string(i));
            }
        }

        // Generate MinHash signatures
        auto sig_a = minhash_engine.generate_signature(corpus_a);
        auto sig_b = minhash_engine.generate_signature(corpus_b);
        auto sig_c = minhash_engine.generate_signature(corpus_c);

        // Estimate similarities
        auto sim_ab = minhash_engine.estimate_similarity(sig_a, sig_b);
        auto sim_ac = minhash_engine.estimate_similarity(sig_a, sig_c);
        auto sim_bc = minhash_engine.estimate_similarity(sig_b, sig_c);

        std::cout << "  Corpus A: 100 documents\n";
        std::cout << "  Corpus B: 100 documents (60% overlap with A)\n";
        std::cout << "  Corpus C: 100 documents (20% overlap with A)\n\n";
        std::cout << "  Similarity A-B: " << sim_ab.value() * 100 << "%"
                  << " (expected ~60%)\n";
        std::cout << "  Similarity A-C: " << sim_ac.value() * 100 << "%"
                  << " (expected ~20%)\n";
        std::cout << "  Similarity B-C: " << sim_bc.value() * 100 << "%"
                  << " (expected ~12%)\n";
    }

    // Example 3: LSH for nearest neighbor search
    std::cout << "\n3. Locality-Sensitive Hashing (LSH) Search:\n";
    {
        trapdoor_factory<32> factory(secret);
        operations::lsh_index<std::string> lsh(secret, 20, 5);

        // Database of items
        std::vector<std::string> database = {
            "apple", "application", "apply", "banana", "band",
            "bandana", "orange", "orangutan", "grape", "grapefruit"
        };

        // Create trapdoors for database
        std::vector<trapdoor<std::string, 32>> db_trapdoors;
        for (const auto& item : database) {
            db_trapdoors.push_back(factory.create(item));
        }

        // Query item
        std::string query = "app";
        auto query_td = factory.create(query);

        // Find similar items
        auto similar_indices = lsh.find_similar(query_td, db_trapdoors, 0.3);

        std::cout << "  Database: ";
        for (const auto& item : database) {
            std::cout << item << " ";
        }
        std::cout << "\n\n";
        std::cout << "  Query: \"" << query << "\"\n";
        std::cout << "  Similar items found:\n";
        for (auto idx : similar_indices) {
            std::cout << "    - " << database[idx] << "\n";
        }
    }

    // Example 4: Cosine similarity
    std::cout << "\n4. Cosine Similarity for Hash Vectors:\n";
    {
        trapdoor_factory<32> factory(secret);

        // Create trapdoors for text embeddings (simulated)
        auto td_doc1 = factory.create(std::string("machine learning algorithms"));
        auto td_doc2 = factory.create(std::string("deep learning neural networks"));
        auto td_doc3 = factory.create(std::string("classical music compositions"));

        // Compute similarities
        auto sim_12 = operations::cosine_similarity<>::compute(td_doc1, td_doc2);
        auto sim_13 = operations::cosine_similarity<>::compute(td_doc1, td_doc3);
        auto sim_23 = operations::cosine_similarity<>::compute(td_doc2, td_doc3);

        std::cout << "  Document 1: \"machine learning algorithms\"\n";
        std::cout << "  Document 2: \"deep learning neural networks\"\n";
        std::cout << "  Document 3: \"classical music compositions\"\n\n";

        std::cout << "  Cosine similarity:\n";
        std::cout << "    Doc1-Doc2: " << sim_12.value()
                  << " (related ML topics)\n";
        std::cout << "    Doc1-Doc3: " << sim_13.value()
                  << " (unrelated topics)\n";
        std::cout << "    Doc2-Doc3: " << sim_23.value()
                  << " (unrelated topics)\n";
    }

    // Example 5: Privacy-preserving recommendation system
    std::cout << "\n5. Privacy-Preserving Recommendations:\n";
    {
        sets::boolean_set_factory<std::string> factory(secret);

        // User preferences (movies watched)
        std::vector<std::string> alice_movies = {
            "The Matrix", "Inception", "Interstellar", "Blade Runner", "Ex Machina"
        };

        std::vector<std::string> bob_movies = {
            "Inception", "The Dark Knight", "Interstellar", "Dunkirk", "Tenet"
        };

        std::vector<std::string> charlie_movies = {
            "The Matrix", "Blade Runner", "Ghost in the Shell", "Ex Machina", "Minority Report"
        };

        // Create privacy-preserving sets
        auto alice_set = factory.from_collection(alice_movies);
        auto bob_set = factory.from_collection(bob_movies);
        auto charlie_set = factory.from_collection(charlie_movies);

        // Find most similar user to Alice
        auto sim_alice_bob = operations::jaccard_similarity<>::estimate(alice_set, bob_set);
        auto sim_alice_charlie = operations::jaccard_similarity<>::estimate(alice_set, charlie_set);

        std::cout << "  Finding similar users for Alice...\n";
        std::cout << "  Similarity with Bob: " << sim_alice_bob.value() << "\n";
        std::cout << "  Similarity with Charlie: " << sim_alice_charlie.value() << "\n";

        if (sim_alice_charlie.value() > sim_alice_bob.value()) {
            std::cout << "  Charlie has most similar taste to Alice\n";
            std::cout << "  Recommending movies from Charlie's list not in Alice's\n";
        } else {
            std::cout << "  Bob has most similar taste to Alice\n";
            std::cout << "  Recommending movies from Bob's list not in Alice's\n";
        }

        std::cout << "  Note: Actual movie titles remain private!\n";
    }

    // Example 6: Duplicate detection
    std::cout << "\n6. Near-Duplicate Detection:\n";
    {
        operations::minhash<std::string> dedup_engine(secret, 64);

        // Documents with minor variations
        std::vector<std::string> doc1_words = {
            "The", "quick", "brown", "fox", "jumps", "over", "the", "lazy", "dog"
        };

        std::vector<std::string> doc2_words = {
            "The", "quick", "brown", "fox", "jumped", "over", "the", "lazy", "dog"  // "jumped" vs "jumps"
        };

        std::vector<std::string> doc3_words = {
            "A", "quick", "brown", "fox", "jumps", "over", "a", "lazy", "dog"  // "A" vs "The"
        };

        std::vector<std::string> doc4_words = {
            "Lorem", "ipsum", "dolor", "sit", "amet", "consectetur", "adipiscing"  // Completely different
        };

        // Generate signatures
        auto sig1 = dedup_engine.generate_signature(doc1_words);
        auto sig2 = dedup_engine.generate_signature(doc2_words);
        auto sig3 = dedup_engine.generate_signature(doc3_words);
        auto sig4 = dedup_engine.generate_signature(doc4_words);

        // Check for near-duplicates
        auto sim_12 = dedup_engine.estimate_similarity(sig1, sig2);
        auto sim_13 = dedup_engine.estimate_similarity(sig1, sig3);
        auto sim_14 = dedup_engine.estimate_similarity(sig1, sig4);

        double duplicate_threshold = 0.8;

        std::cout << "  Duplicate detection threshold: " << duplicate_threshold * 100 << "%\n\n";
        std::cout << "  Doc1 vs Doc2 (one word different): "
                  << sim_12.value() * 100 << "% similar - "
                  << (sim_12.value() > duplicate_threshold ? "DUPLICATE" : "UNIQUE") << "\n";
        std::cout << "  Doc1 vs Doc3 (articles changed): "
                  << sim_13.value() * 100 << "% similar - "
                  << (sim_13.value() > duplicate_threshold ? "DUPLICATE" : "UNIQUE") << "\n";
        std::cout << "  Doc1 vs Doc4 (completely different): "
                  << sim_14.value() * 100 << "% similar - "
                  << (sim_14.value() > duplicate_threshold ? "DUPLICATE" : "UNIQUE") << "\n";
    }

    return 0;
}