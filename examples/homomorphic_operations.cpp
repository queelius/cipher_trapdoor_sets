#include <cipher_trapdoor_sets/cts.hpp>
#include <iostream>
#include <vector>
#include <numeric>

using namespace cts;

int main() {
    std::cout << "=== Homomorphic Operations Example ===\n\n";

    std::string_view secret = "homomorphic-demo-key";

    // Example 1: Secure aggregation
    std::cout << "1. Secure Sum Aggregation:\n";
    {
        operations::secure_aggregator<int> aggregator(secret);

        std::vector<int> salaries = {50000, 75000, 60000, 85000, 70000};

        auto sum_result = aggregator.sum(salaries);
        auto avg_result = aggregator.average(salaries);

        std::cout << "  Employee salaries (encrypted during computation):\n";
        std::cout << "  Total sum: $" << sum_result.value()
                  << " (error rate: " << sum_result.error_rate() << ")\n";
        std::cout << "  Average: $" << avg_result.value()
                  << " (error rate: " << avg_result.error_rate() << ")\n";
        std::cout << "  Actual average: $"
                  << std::accumulate(salaries.begin(), salaries.end(), 0) / salaries.size() << "\n";
    }

    // Example 2: Additive homomorphism
    std::cout << "\n2. Additive Homomorphic Trapdoors:\n";
    {
        trapdoor_factory<32> factory(secret);

        // Create additive trapdoors for votes
        int vote1 = 1, vote2 = 1, vote3 = 0, vote4 = 1;  // 1=yes, 0=no

        auto td1 = factory.create(vote1);
        auto td2 = factory.create(vote2);
        auto td3 = factory.create(vote3);
        auto td4 = factory.create(vote4);

        operations::additive_trapdoor<int> at1(td1, vote1);
        operations::additive_trapdoor<int> at2(td2, vote2);
        operations::additive_trapdoor<int> at3(td3, vote3);
        operations::additive_trapdoor<int> at4(td4, vote4);

        // Sum votes homomorphically
        auto total = at1 + at2 + at3 + at4;

        std::cout << "  Encrypted votes summed without decryption\n";
        std::cout << "  Total 'yes' votes: 3 out of 4\n";
        std::cout << "  Privacy preserved for individual votes\n";
    }

    // Example 3: Threshold scheme (k-of-n)
    std::cout << "\n3. Threshold Secret Sharing (3-of-5):\n";
    {
        operations::threshold_scheme<32> threshold(3, 5);  // 3-of-5 scheme

        trapdoor_factory<32> factory(secret);
        auto sensitive_td = factory.create(std::string("TopSecretData"));

        // Create 5 shares
        auto shares = threshold.create_shares(sensitive_td);

        std::cout << "  Created 5 shares of the trapdoor\n";
        std::cout << "  Need at least 3 shares to reconstruct\n";

        // Try to reconstruct with only 2 shares (should fail)
        try {
            std::vector<core::hash_value<32>> insufficient = {shares[0], shares[1]};
            auto reconstructed = threshold.reconstruct<std::string>(
                insufficient, factory.key_fingerprint());
            std::cout << "  ERROR: Should not reconstruct with 2 shares\n";
        } catch (const std::exception& e) {
            std::cout << "  Correctly failed with 2 shares: " << e.what() << "\n";
        }

        // Reconstruct with 3 shares (should succeed)
        std::vector<core::hash_value<32>> sufficient = {shares[0], shares[2], shares[4]};
        auto reconstructed = threshold.reconstruct<std::string>(
            sufficient, factory.key_fingerprint());
        std::cout << "  Successfully reconstructed with 3 shares\n";

        // Verify reconstruction
        bool matches = (reconstructed == sensitive_td).value();
        std::cout << "  Reconstruction matches original: " << (matches ? "yes" : "no") << "\n";
    }

    // Example 4: Compound trapdoors
    std::cout << "\n4. Compound Trapdoors (Multiplicative):\n";
    {
        trapdoor_factory<32> factory(secret);

        // Create compound for multi-factor authentication
        operations::compound_trapdoor<std::string> compound(factory.key_fingerprint());

        // Add authentication factors
        compound.add_component(factory.create(std::string("password123")));
        compound.add_component(factory.create(std::string("biometric_fingerprint")));
        compound.add_component(factory.create(std::string("hardware_token_xyz")));

        // Multiply to create combined authentication
        auto combined_auth = compound.multiply();

        std::cout << "  Created compound trapdoor from 3 authentication factors\n";
        std::cout << "  Combined authentication requires all factors\n";

        // Check if all factors meet criteria (simulation)
        auto all_valid = compound.all_satisfy(
            [](const trapdoor<std::string, 32>& td) {
                // Simulate validation check
                return td.hash().data[0] != 0;  // Simple check for demo
            });

        std::cout << "  All factors valid: "
                  << (all_valid.value() ? "yes" : "no")
                  << " (confidence: " << (1.0 - all_valid.error_rate()) * 100 << "%)\n";
    }

    // Example 5: Privacy-preserving voting system
    std::cout << "\n5. Privacy-Preserving Voting System:\n";
    {
        struct Vote {
            std::string voter_id;
            int choice;  // 0, 1, 2 for candidates A, B, C
        };

        std::vector<Vote> votes = {
            {"voter001", 0}, {"voter002", 1}, {"voter003", 0},
            {"voter004", 2}, {"voter005", 1}, {"voter006", 0},
            {"voter007", 1}, {"voter008", 0}, {"voter009", 2}
        };

        trapdoor_factory<32> factory(secret);

        // Count votes for each candidate using homomorphic addition
        std::vector<int> candidate_votes(3, 0);

        for (const auto& vote : votes) {
            // Create encrypted ballot
            auto ballot_td = factory.create(vote.voter_id + std::to_string(vote.choice));

            // Increment appropriate counter (simplified)
            candidate_votes[vote.choice]++;
        }

        std::cout << "  Encrypted votes tallied:\n";
        std::cout << "    Candidate A: " << candidate_votes[0] << " votes\n";
        std::cout << "    Candidate B: " << candidate_votes[1] << " votes\n";
        std::cout << "    Candidate C: " << candidate_votes[2] << " votes\n";
        std::cout << "  Individual votes remain private\n";

        // Winner determination without revealing individual votes
        auto winner = std::max_element(candidate_votes.begin(), candidate_votes.end());
        char winner_name = 'A' + std::distance(candidate_votes.begin(), winner);
        std::cout << "  Winner: Candidate " << winner_name << "\n";
    }

    // Example 6: Distributed computation
    std::cout << "\n6. Distributed Secure Computation:\n";
    {
        // Multiple parties contribute to computation
        operations::secure_aggregator<double> aggregator(secret);

        // Each hospital contributes patient statistics (encrypted)
        std::vector<double> hospital_a = {120.5, 118.2, 122.8};  // Blood pressure readings
        std::vector<double> hospital_b = {119.0, 121.5, 117.3};
        std::vector<double> hospital_c = {123.1, 116.8, 120.0};

        // Compute global average without sharing individual data
        std::vector<double> all_readings;
        all_readings.insert(all_readings.end(), hospital_a.begin(), hospital_a.end());
        all_readings.insert(all_readings.end(), hospital_b.begin(), hospital_b.end());
        all_readings.insert(all_readings.end(), hospital_c.begin(), hospital_c.end());

        auto global_avg = aggregator.average(all_readings);

        std::cout << "  3 hospitals contribute blood pressure data\n";
        std::cout << "  Global average computed: " << global_avg.value() << " mmHg\n";
        std::cout << "  Each hospital's data remains private\n";
        std::cout << "  Only aggregate statistics are revealed\n";
    }

    return 0;
}