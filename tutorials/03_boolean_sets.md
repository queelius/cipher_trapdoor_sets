# Tutorial 3: Boolean Set Operations

**Duration:** 90 minutes | **Level:** Intermediate | **Prerequisites:** Tutorials 1 & 2

## Learning Objectives

By the end of this tutorial, you will:
- Perform privacy-preserving set operations (union, intersection, complement)
- Understand the Boolean algebra of trapdoor sets
- Implement approximate membership testing
- Build oblivious data structures
- Apply homomorphic properties for secure computation

## Table of Contents

1. [Boolean Algebra Fundamentals](#boolean-algebra-fundamentals)
2. [Basic Set Operations](#basic-set-operations)
3. [Advanced Set Operations](#advanced-set-operations)
4. [Approximate Membership Testing](#approximate-membership-testing)
5. [Building Oblivious Data Structures](#building-oblivious-data-structures)
6. [Real-World Applications](#real-world-applications)
7. [Exercises](#exercises)
8. [Key Takeaways](#key-takeaways)

## Boolean Algebra Fundamentals

### Understanding Boolean Sets

In our library, Boolean sets use bitwise operations on hash values to implement set operations:

```cpp
#include <cipher_trapdoor_sets/cts.hpp>
#include <cipher_trapdoor_sets/sets/boolean_set.hpp>
#include <iostream>

using namespace cts;
using namespace cts::sets;

void demonstrate_boolean_fundamentals() {
    // Boolean sets operate on the bit representation of hashes
    // Each bit position can be thought of as a "dimension"

    trapdoor_factory<32> factory("boolean-key-2025");

    // Create individual trapdoors
    auto td_alice = factory.create(std::string("Alice"));
    auto td_bob = factory.create(std::string("Bob"));

    // Convert to boolean sets
    boolean_set<std::string, 32> set_alice(td_alice.hash(), td_alice.key_fingerprint());
    boolean_set<std::string, 32> set_bob(td_bob.hash(), td_bob.key_fingerprint());

    // Boolean operations work on bit patterns
    // Union: OR operation on bits
    // Intersection: AND operation on bits
    // Complement: NOT operation on bits

    std::cout << "Boolean set operations preserve privacy:\n";
    std::cout << "- Original values (Alice, Bob) are never revealed\n";
    std::cout << "- Operations work directly on hash values\n";
    std::cout << "- Results are also trapdoor-protected\n";
}
```

### Mathematical Foundation

```cpp
// Visual representation of boolean operations on hash bits

// Example with 8-bit hashes (simplified):
// Alice hash: 10110011
// Bob hash:   11010101

// Union (OR):      11110111  (combines both patterns)
// Intersection:    10010001  (common bits only)
// Difference:      00100010  (Alice - Bob)
// Symmetric Diff:  01100110  (XOR - exclusive bits)

void visualize_operations() {
    // Simplified 1-byte example for visualization
    uint8_t alice = 0b10110011;
    uint8_t bob   = 0b11010101;

    uint8_t union_result = alice | bob;           // 0b11110111
    uint8_t intersect_result = alice & bob;       // 0b10010001
    uint8_t diff_result = alice & ~bob;           // 0b00100010
    uint8_t sym_diff_result = alice ^ bob;        // 0b01100110

    std::cout << "Alice:         " << std::bitset<8>(alice) << "\n";
    std::cout << "Bob:           " << std::bitset<8>(bob) << "\n";
    std::cout << "Union:         " << std::bitset<8>(union_result) << "\n";
    std::cout << "Intersection:  " << std::bitset<8>(intersect_result) << "\n";
    std::cout << "Difference:    " << std::bitset<8>(diff_result) << "\n";
    std::cout << "Symmetric:     " << std::bitset<8>(sym_diff_result) << "\n";
}
```

## Basic Set Operations

### Union Operation

```cpp
#include <cipher_trapdoor_sets/sets/boolean_set.hpp>
#include <vector>
#include <string>

void demonstrate_union() {
    using namespace cts;
    using namespace cts::sets;

    trapdoor_factory<32> factory("union-key-2025");

    // Create sets from multiple elements
    std::vector<std::string> team_a = {"Alice", "Bob", "Charlie"};
    std::vector<std::string> team_b = {"Bob", "Diana", "Eve"};

    // Build boolean sets
    boolean_set<std::string, 32> set_a;
    for (const auto& member : team_a) {
        auto td = factory.create(member);
        boolean_set<std::string, 32> member_set(td.hash(), td.key_fingerprint());
        set_a = set_a | member_set;  // Union to add member
    }

    boolean_set<std::string, 32> set_b;
    for (const auto& member : team_b) {
        auto td = factory.create(member);
        boolean_set<std::string, 32> member_set(td.hash(), td.key_fingerprint());
        set_b = set_b | member_set;
    }

    // Compute union
    auto union_set = set_a | set_b;

    std::cout << "Team A: Alice, Bob, Charlie\n";
    std::cout << "Team B: Bob, Diana, Eve\n";
    std::cout << "Union contains all members from both teams\n";

    // Test membership
    std::vector<std::string> all_people = {"Alice", "Bob", "Charlie", "Diana", "Eve", "Frank"};

    for (const auto& person : all_people) {
        auto td = factory.create(person);
        auto result = union_set.contains(td);

        std::cout << person << " in union: "
                  << (result.value() ? "YES" : "NO")
                  << " (confidence: " << (1.0 - result.false_positive_rate()) * 100 << "%)\n";
    }
}
```

### Intersection Operation

```cpp
void demonstrate_intersection() {
    using namespace cts;
    using namespace cts::sets;

    trapdoor_factory<32> factory("intersect-key-2025");

    // Scenario: Find common customers between two stores
    std::vector<std::string> store1_customers = {
        "customer1@example.com",
        "customer2@example.com",
        "customer3@example.com",
        "customer4@example.com"
    };

    std::vector<std::string> store2_customers = {
        "customer2@example.com",
        "customer3@example.com",
        "customer5@example.com",
        "customer6@example.com"
    };

    // Build sets
    auto build_set = [&factory](const std::vector<std::string>& items) {
        boolean_set<std::string, 32> result;
        for (const auto& item : items) {
            auto td = factory.create(item);
            boolean_set<std::string, 32> item_set(td.hash(), td.key_fingerprint());
            result = result | item_set;
        }
        return result;
    };

    auto set1 = build_set(store1_customers);
    auto set2 = build_set(store2_customers);

    // Find common customers
    auto common_set = set1 & set2;

    std::cout << "Store 1 has " << store1_customers.size() << " customers\n";
    std::cout << "Store 2 has " << store2_customers.size() << " customers\n";
    std::cout << "\nChecking for common customers:\n";

    // Test all possible customers
    std::set<std::string> all_customers;
    all_customers.insert(store1_customers.begin(), store1_customers.end());
    all_customers.insert(store2_customers.begin(), store2_customers.end());

    for (const auto& customer : all_customers) {
        auto td = factory.create(customer);
        auto in_common = common_set.contains(td);

        if (in_common.value()) {
            std::cout << "Customer " << customer.substr(0, 9) << "*** is common to both stores\n";
        }
    }
}
```

### Complement and Difference Operations

```cpp
void demonstrate_complement_and_difference() {
    using namespace cts;
    using namespace cts::sets;

    trapdoor_factory<32> factory("complement-key-2025");

    // Scenario: Access control - users with access vs. blocked users
    std::vector<std::string> all_users = {"user1", "user2", "user3", "user4", "user5"};
    std::vector<std::string> allowed_users = {"user1", "user3", "user5"};

    // Build allowed set
    boolean_set<std::string, 32> allowed_set;
    for (const auto& user : allowed_users) {
        auto td = factory.create(user);
        boolean_set<std::string, 32> user_set(td.hash(), td.key_fingerprint());
        allowed_set = allowed_set | user_set;
    }

    // Complement gives us the blocked set (in theory)
    // Note: Complement in hash space doesn't directly map to set complement
    auto complement_set = ~allowed_set;

    std::cout << "Allowed users: user1, user3, user5\n";
    std::cout << "Complement represents 'not allowed' pattern\n\n";

    // Set difference for explicit blocking
    boolean_set<std::string, 32> all_set;
    for (const auto& user : all_users) {
        auto td = factory.create(user);
        boolean_set<std::string, 32> user_set(td.hash(), td.key_fingerprint());
        all_set = all_set | user_set;
    }

    // Blocked = All - Allowed
    auto blocked_set = all_set - allowed_set;

    std::cout << "Testing blocked set (All - Allowed):\n";
    for (const auto& user : all_users) {
        auto td = factory.create(user);
        auto is_blocked = blocked_set.contains(td);

        std::cout << user << ": "
                  << (is_blocked.value() ? "BLOCKED" : "ALLOWED") << "\n";
    }
}
```

## Advanced Set Operations

### Symmetric Difference

```cpp
void demonstrate_symmetric_difference() {
    using namespace cts;
    using namespace cts::sets;

    trapdoor_factory<32> factory("symmetric-key-2025");

    // Scenario: Find changes between two snapshots
    std::vector<std::string> snapshot1 = {"file1.txt", "file2.txt", "file3.txt"};
    std::vector<std::string> snapshot2 = {"file2.txt", "file3.txt", "file4.txt"};

    auto build_set = [&factory](const std::vector<std::string>& items) {
        boolean_set<std::string, 32> result;
        for (const auto& item : items) {
            auto td = factory.create(item);
            boolean_set<std::string, 32> item_set(td.hash(), td.key_fingerprint());
            result = result | item_set;
        }
        return result;
    };

    auto set1 = build_set(snapshot1);
    auto set2 = build_set(snapshot2);

    // Symmetric difference: items in one set but not both
    auto changes = set1 ^ set2;

    std::cout << "Snapshot 1: file1.txt, file2.txt, file3.txt\n";
    std::cout << "Snapshot 2: file2.txt, file3.txt, file4.txt\n";
    std::cout << "\nChanges (symmetric difference):\n";

    std::vector<std::string> all_files = {"file1.txt", "file2.txt", "file3.txt", "file4.txt"};
    for (const auto& file : all_files) {
        auto td = factory.create(file);
        auto changed = changes.contains(td);

        if (changed.value()) {
            std::cout << file << " was added or removed\n";
        }
    }
}
```

### Subset Testing

```cpp
void demonstrate_subset_testing() {
    using namespace cts;
    using namespace cts::sets;

    trapdoor_factory<32> factory("subset-key-2025");

    // Scenario: Check if user permissions are subset of role permissions
    std::vector<std::string> admin_perms = {"read", "write", "delete", "admin"};
    std::vector<std::string> user_perms = {"read", "write"};
    std::vector<std::string> guest_perms = {"read"};

    auto build_set = [&factory](const std::vector<std::string>& items) {
        boolean_set<std::string, 32> result;
        for (const auto& item : items) {
            auto td = factory.create(item);
            boolean_set<std::string, 32> item_set(td.hash(), td.key_fingerprint());
            result = result | item_set;
        }
        return result;
    };

    auto admin_set = build_set(admin_perms);
    auto user_set = build_set(user_perms);
    auto guest_set = build_set(guest_perms);

    // Check subset relationships
    auto guest_subset_user = guest_set.subset_of(user_set);
    auto user_subset_admin = user_set.subset_of(admin_set);
    auto admin_subset_user = admin_set.subset_of(user_set);

    std::cout << "Permission hierarchy check:\n";
    std::cout << "Guest ⊆ User: " << guest_subset_user.value() << "\n";
    std::cout << "User ⊆ Admin: " << user_subset_admin.value() << "\n";
    std::cout << "Admin ⊆ User: " << admin_subset_user.value() << " (should be false)\n";
}
```

## Approximate Membership Testing

### Understanding Bloom Filter Properties

```cpp
#include <cipher_trapdoor_sets/sets/boolean_set.hpp>
#include <random>
#include <cmath>

class PrivateBloomFilter {
private:
    std::vector<boolean_set<std::string, 32>> hash_sets_;
    trapdoor_factory<32> factory_;
    size_t num_hashes_;

public:
    PrivateBloomFilter(const std::string& key, size_t num_hashes = 3)
        : factory_(key), num_hashes_(num_hashes) {
        hash_sets_.resize(num_hashes);
    }

    void add(const std::string& element) {
        // Use different hash functions by adding salt
        for (size_t i = 0; i < num_hashes_; ++i) {
            auto salted = element + "_hash_" + std::to_string(i);
            auto td = factory_.create(salted);
            boolean_set<std::string, 32> elem_set(td.hash(), td.key_fingerprint());
            hash_sets_[i] = hash_sets_[i] | elem_set;
        }
    }

    core::approximate_bool contains(const std::string& element) {
        bool all_present = true;
        double combined_fpr = 1.0;

        for (size_t i = 0; i < num_hashes_; ++i) {
            auto salted = element + "_hash_" + std::to_string(i);
            auto td = factory_.create(salted);
            auto result = hash_sets_[i].contains(td);

            if (!result.value()) {
                all_present = false;
                break;
            }

            // Combine false positive rates
            combined_fpr *= result.false_positive_rate();
        }

        // With multiple hashes, FPR improves
        return core::approximate_bool(all_present, combined_fpr, 0.0);
    }

    double calculate_theoretical_fpr(size_t num_elements) {
        // Bloom filter false positive rate formula
        // FPR ≈ (1 - e^(-kn/m))^k
        // k = num_hashes, n = num_elements, m = bits available

        size_t bits = 256 * num_hashes_;  // Total bits across all hashes
        double ratio = static_cast<double>(num_hashes_ * num_elements) / bits;
        double fpr = std::pow(1.0 - std::exp(-ratio), num_hashes_);

        return fpr;
    }
};

void test_bloom_filter() {
    PrivateBloomFilter filter("bloom-key-2025", 3);

    // Add elements
    std::vector<std::string> elements = {
        "apple", "banana", "cherry", "date", "elderberry"
    };

    for (const auto& elem : elements) {
        filter.add(elem);
    }

    // Test membership
    std::vector<std::string> test_items = {
        "apple", "banana", "grape", "kiwi", "cherry"
    };

    std::cout << "Bloom Filter Membership Test:\n";
    std::cout << "Added: apple, banana, cherry, date, elderberry\n\n";

    for (const auto& item : test_items) {
        auto result = filter.contains(item);
        std::cout << item << ": "
                  << (result.value() ? "PROBABLY IN SET" : "DEFINITELY NOT IN SET")
                  << " (FPR: " << result.false_positive_rate() << ")\n";
    }

    std::cout << "\nTheoretical FPR for " << elements.size()
              << " elements: " << filter.calculate_theoretical_fpr(elements.size()) << "\n";
}
```

### Cardinality Estimation

```cpp
#include <cipher_trapdoor_sets/operations/cardinality.hpp>

void demonstrate_cardinality_estimation() {
    using namespace cts;
    using namespace cts::sets;
    using namespace cts::operations;

    trapdoor_factory<32> factory("cardinality-key-2025");

    // Build a large set
    boolean_set<int, 32> large_set;
    const int num_elements = 10000;

    for (int i = 0; i < num_elements; ++i) {
        auto td = factory.create(i);
        boolean_set<int, 32> elem_set(td.hash(), td.key_fingerprint());
        large_set = large_set | elem_set;
    }

    // Estimate cardinality using hash properties
    auto estimator = cardinality_estimator<32>();
    auto estimate = estimator.estimate(large_set);

    std::cout << "Actual elements added: " << num_elements << "\n";
    std::cout << "Estimated cardinality: " << estimate.value() << "\n";
    std::cout << "Error margin: ±" << estimate.error_margin() << "\n";
    std::cout << "Relative error: "
              << std::abs(estimate.value() - num_elements) / (double)num_elements * 100
              << "%\n";
}
```

## Building Oblivious Data Structures

### Oblivious Set

```cpp
template<typename T, size_t N = 32>
class ObliviousSet {
private:
    boolean_set<T, N> set_;
    trapdoor_factory<N> factory_;
    std::vector<core::hash_value<N>> dummy_queries_;

public:
    ObliviousSet(const std::string& key)
        : factory_(key) {
        // Pre-generate dummy queries for oblivious access
        generate_dummy_queries(100);
    }

    void insert(const T& element) {
        auto td = factory_.create(element);
        boolean_set<T, N> elem_set(td.hash(), td.key_fingerprint());
        set_ = set_ | elem_set;
    }

    // Oblivious query - always performs the same operations
    bool oblivious_contains(const T& element) {
        auto td = factory_.create(element);

        // Perform dummy operations to hide access pattern
        for (const auto& dummy : dummy_queries_) {
            boolean_set<T, N> dummy_set(dummy, factory_.key_fingerprint());
            volatile auto dummy_result = set_.contains(trapdoor<T, N>(dummy, factory_.key_fingerprint()));
        }

        // Real query hidden among dummies
        auto result = set_.contains(td);

        return result.value();
    }

private:
    void generate_dummy_queries(size_t count) {
        std::random_device rd;
        std::mt19937 gen(rd());

        for (size_t i = 0; i < count; ++i) {
            core::hash_value<N> dummy;
            for (size_t j = 0; j < N; ++j) {
                dummy.data[j] = gen() & 0xFF;
            }
            dummy_queries_.push_back(dummy);
        }
    }
};
```

### Private Set Intersection Protocol

```cpp
class PrivateSetIntersection {
public:
    struct PartyData {
        std::vector<std::string> elements;
        boolean_set<std::string, 32> set;
    };

    // Two-party PSI protocol
    static std::vector<std::string> compute_intersection(
        const std::vector<std::string>& party_a_data,
        const std::vector<std::string>& party_b_data,
        const std::string& shared_key) {

        trapdoor_factory<32> factory(shared_key);

        // Party A creates their set
        boolean_set<std::string, 32> set_a;
        for (const auto& elem : party_a_data) {
            auto td = factory.create(elem);
            boolean_set<std::string, 32> elem_set(td.hash(), td.key_fingerprint());
            set_a = set_a | elem_set;
        }

        // Party B creates their set
        boolean_set<std::string, 32> set_b;
        for (const auto& elem : party_b_data) {
            auto td = factory.create(elem);
            boolean_set<std::string, 32> elem_set(td.hash(), td.key_fingerprint());
            set_b = set_b | elem_set;
        }

        // Compute intersection
        auto intersection = set_a & set_b;

        // Find actual intersecting elements
        std::vector<std::string> result;

        // Check all elements from both parties
        std::set<std::string> all_elements;
        all_elements.insert(party_a_data.begin(), party_a_data.end());
        all_elements.insert(party_b_data.begin(), party_b_data.end());

        for (const auto& elem : all_elements) {
            auto td = factory.create(elem);
            if (intersection.contains(td).value()) {
                // Only add if it was in both original sets
                bool in_a = std::find(party_a_data.begin(), party_a_data.end(), elem)
                           != party_a_data.end();
                bool in_b = std::find(party_b_data.begin(), party_b_data.end(), elem)
                           != party_b_data.end();

                if (in_a && in_b) {
                    result.push_back(elem);
                }
            }
        }

        return result;
    }
};

void test_private_set_intersection() {
    // Hospital A's patients with rare disease
    std::vector<std::string> hospital_a = {
        "patient001", "patient002", "patient003", "patient004"
    };

    // Hospital B's patients with rare disease
    std::vector<std::string> hospital_b = {
        "patient003", "patient004", "patient005", "patient006"
    };

    // Compute intersection without revealing non-overlapping patients
    auto common_patients = PrivateSetIntersection::compute_intersection(
        hospital_a, hospital_b, "medical-research-key-2025");

    std::cout << "Private Set Intersection Result:\n";
    std::cout << "Hospital A has " << hospital_a.size() << " patients\n";
    std::cout << "Hospital B has " << hospital_b.size() << " patients\n";
    std::cout << "Common patients: " << common_patients.size() << "\n";

    for (const auto& patient : common_patients) {
        std::cout << "  - " << patient << "\n";
    }

    std::cout << "\nPrivacy preserved: Neither hospital learns about "
              << "non-overlapping patients\n";
}
```

## Real-World Applications

### Application 1: Contact Tracing

```cpp
class PrivacyPreservingContactTracing {
private:
    struct UserData {
        std::string user_id;
        boolean_set<std::string, 32> contact_set;
    };

    std::unordered_map<std::string, UserData> users_;
    trapdoor_factory<32> factory_;

public:
    PrivacyPreservingContactTracing()
        : factory_("contact-tracing-key-2025") {}

    void register_contact(const std::string& user_a, const std::string& user_b) {
        // Create trapdoors for both users
        auto td_a = factory_.create(user_a);
        auto td_b = factory_.create(user_b);

        // Update user A's contacts
        if (users_.find(user_a) == users_.end()) {
            users_[user_a] = {user_a, boolean_set<std::string, 32>()};
        }
        boolean_set<std::string, 32> set_b(td_b.hash(), td_b.key_fingerprint());
        users_[user_a].contact_set = users_[user_a].contact_set | set_b;

        // Update user B's contacts
        if (users_.find(user_b) == users_.end()) {
            users_[user_b] = {user_b, boolean_set<std::string, 32>()};
        }
        boolean_set<std::string, 32> set_a(td_a.hash(), td_a.key_fingerprint());
        users_[user_b].contact_set = users_[user_b].contact_set | set_a;
    }

    std::vector<std::string> find_exposures(const std::string& infected_user) {
        std::vector<std::string> exposed_users;

        if (users_.find(infected_user) == users_.end()) {
            return exposed_users;
        }

        const auto& infected_contacts = users_[infected_user].contact_set;

        // Check all users for exposure
        for (const auto& [user_id, user_data] : users_) {
            if (user_id == infected_user) continue;

            auto td = factory_.create(user_id);

            // Check if this user had contact with infected person
            if (infected_contacts.contains(td).value()) {
                exposed_users.push_back(user_id);
            }
        }

        return exposed_users;
    }

    void print_statistics() {
        std::cout << "Contact Tracing System Statistics:\n";
        std::cout << "Registered users: " << users_.size() << "\n";

        size_t total_contacts = 0;
        for (const auto& [_, user_data] : users_) {
            // Estimate contacts per user (would need cardinality estimator)
            total_contacts++;  // Simplified
        }

        std::cout << "Privacy guarantee: Contact graphs are hash-protected\n";
        std::cout << "Storage per user: " << sizeof(boolean_set<std::string, 32>) << " bytes\n";
    }
};

void test_contact_tracing() {
    PrivacyPreservingContactTracing system;

    // Register contacts
    system.register_contact("Alice", "Bob");
    system.register_contact("Alice", "Charlie");
    system.register_contact("Bob", "Diana");
    system.register_contact("Charlie", "Eve");
    system.register_contact("Diana", "Eve");

    // Alice tests positive
    auto exposed = system.find_exposures("Alice");

    std::cout << "Contact Tracing Results:\n";
    std::cout << "Infected person: Alice\n";
    std::cout << "Direct contacts at risk:\n";

    for (const auto& person : exposed) {
        std::cout << "  - " << person << "\n";
    }

    system.print_statistics();
}
```

### Application 2: Anonymous Voting

```cpp
class AnonymousVotingSystem {
private:
    struct Ballot {
        boolean_set<std::string, 32> choices;
        size_t timestamp;
    };

    std::vector<Ballot> ballots_;
    boolean_set<std::string, 32> eligible_voters_;
    boolean_set<std::string, 32> voted_set_;
    trapdoor_factory<32> factory_;

public:
    AnonymousVotingSystem(const std::string& election_key)
        : factory_(election_key) {}

    void register_voter(const std::string& voter_id) {
        auto td = factory_.create(voter_id);
        boolean_set<std::string, 32> voter_set(td.hash(), td.key_fingerprint());
        eligible_voters_ = eligible_voters_ | voter_set;
    }

    bool cast_vote(const std::string& voter_id,
                   const std::vector<std::string>& choices) {
        auto td_voter = factory_.create(voter_id);

        // Check eligibility
        if (!eligible_voters_.contains(td_voter).value()) {
            std::cout << "Voter not eligible\n";
            return false;
        }

        // Check for double voting
        if (voted_set_.contains(td_voter).value()) {
            std::cout << "Voter already voted\n";
            return false;
        }

        // Record vote
        boolean_set<std::string, 32> choice_set;
        for (const auto& choice : choices) {
            auto td_choice = factory_.create(choice);
            boolean_set<std::string, 32> c_set(td_choice.hash(), td_choice.key_fingerprint());
            choice_set = choice_set | c_set;
        }

        ballots_.push_back({choice_set, std::time(nullptr)});

        // Mark as voted
        boolean_set<std::string, 32> v_set(td_voter.hash(), td_voter.key_fingerprint());
        voted_set_ = voted_set_ | v_set;

        return true;
    }

    std::map<std::string, int> tally_votes(const std::vector<std::string>& candidates) {
        std::map<std::string, int> results;

        for (const auto& candidate : candidates) {
            auto td = factory_.create(candidate);
            int count = 0;

            for (const auto& ballot : ballots_) {
                if (ballot.choices.contains(td).value()) {
                    count++;
                }
            }

            results[candidate] = count;
        }

        return results;
    }

    void print_election_summary() {
        std::cout << "\nElection Summary:\n";
        std::cout << "Total ballots cast: " << ballots_.size() << "\n";
        std::cout << "Privacy preserved: Voter identities not linked to ballots\n";
        std::cout << "Integrity maintained: No double voting possible\n";
    }
};

void test_voting_system() {
    AnonymousVotingSystem election("election-2025-key");

    // Register eligible voters
    std::vector<std::string> voters = {"voter001", "voter002", "voter003", "voter004"};
    for (const auto& voter : voters) {
        election.register_voter(voter);
    }

    // Cast votes
    election.cast_vote("voter001", {"candidate_a"});
    election.cast_vote("voter002", {"candidate_b"});
    election.cast_vote("voter003", {"candidate_a"});
    election.cast_vote("voter004", {"candidate_a"});

    // Attempt double voting
    election.cast_vote("voter001", {"candidate_b"});  // Should fail

    // Tally results
    std::vector<std::string> candidates = {"candidate_a", "candidate_b", "candidate_c"};
    auto results = election.tally_votes(candidates);

    std::cout << "Election Results:\n";
    for (const auto& [candidate, votes] : results) {
        std::cout << candidate << ": " << votes << " votes\n";
    }

    election.print_election_summary();
}
```

## Exercises

### Exercise 1: Multi-Set Operations

Implement a function that computes (A ∪ B) ∩ (C ∪ D) efficiently.

```cpp
// TODO: Implement multi_set_operation function
// Input: Four boolean sets A, B, C, D
// Output: (A ∪ B) ∩ (C ∪ D)
// Challenge: Minimize the number of operations
```

[Solution](exercises/tutorial3_ex1_solution.cpp)

### Exercise 2: Private Analytics

Build a system that computes statistics on the union of multiple private datasets.

```cpp
// TODO: Implement PrivateAnalytics class
// - Aggregate data from multiple sources
// - Compute union cardinality
// - Find most common elements (top-k)
// - All without revealing individual datasets
```

[Solution](exercises/tutorial3_ex2_solution.cpp)

### Exercise 3: Threshold Set Operations

Create a system where an element is considered "in the set" only if it appears in at least k out of n sets.

```cpp
// TODO: Implement ThresholdSet class
// - Add multiple sets
// - Query with threshold k
// - Return elements appearing in >= k sets
```

[Solution](exercises/tutorial3_ex3_solution.cpp)

## Performance Considerations

### Operation Complexity

| Operation | Time Complexity | Space Complexity |
|-----------|-----------------|------------------|
| Union (∪) | O(1) | O(1) |
| Intersection (∩) | O(1) | O(1) |
| Complement (~) | O(1) | O(1) |
| Contains | O(1) | O(1) |
| Build from n elements | O(n) | O(1)* |

*Space is constant for the set itself, regardless of elements

### Optimization Tips

1. **Batch Operations**: Combine multiple elements into one set operation
2. **Lazy Evaluation**: Defer complex operations until needed
3. **Caching**: Store frequently used sets
4. **Parallel Processing**: Operations are embarrassingly parallel
5. **Hash Size Trade-offs**: Smaller hashes = faster ops but higher FPR

## Common Pitfalls

### 1. Misunderstanding Complement

**Wrong:**
```cpp
// Complement doesn't give you "all other elements"
auto not_alice = ~set_alice;
// This is just bit-flipped hash, not meaningful set
```

**Right:**
```cpp
// Use difference from universal set
auto universal = /* all possible elements */;
auto not_alice = universal - set_alice;
```

### 2. Ignoring False Positives in Critical Operations

**Wrong:**
```cpp
if (medical_records.contains(patient).value()) {
    administer_medication();  // Dangerous!
}
```

**Right:**
```cpp
auto result = medical_records.contains(patient);
if (result.value() && result.false_positive_rate() < 0.0001) {
    // Additional verification for critical operations
    verify_and_administer();
}
```

### 3. Set Building Inefficiency

**Wrong:**
```cpp
// Inefficient: Multiple factory calls
for (const auto& elem : elements) {
    set = set | boolean_set(factory.create(elem));
}
```

**Right:**
```cpp
// Efficient: Batch creation
auto batch = batch_operations::batch_create_trapdoors(elements, key);
for (const auto& td : batch) {
    set = set | boolean_set(td.hash(), td.key_fingerprint());
}
```

## Key Takeaways

✅ **Boolean operations preserve privacy**: Work directly on hashes without revealing values

✅ **Homomorphic properties**: Operations on encrypted sets produce valid encrypted results

✅ **Approximate membership is probabilistic**: Always consider false positive rates

✅ **Oblivious structures hide access patterns**: Important for preventing side-channel attacks

✅ **Real applications require careful design**: Balance privacy, accuracy, and performance

## What's Next?

In [Tutorial 4: Privacy-Preserving Analytics](04_privacy_analytics.md), we'll explore:
- Advanced cardinality estimation
- Similarity metrics (Jaccard, Cosine)
- Multi-party secure computation
- Real-time analytics on encrypted data

---

*Continue to [Tutorial 4: Privacy-Preserving Analytics](04_privacy_analytics.md) →*