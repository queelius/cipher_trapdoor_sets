// Tutorial 3: Private Set Operations Demo
// Compile: g++ -std=c++17 -I../../include tutorial3_private_set_operations.cpp -o set_ops
// Run: ./set_ops

#include <cipher_trapdoor_sets/cts.hpp>
#include <cipher_trapdoor_sets/sets/boolean_set.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <algorithm>

using namespace cts;
using namespace cts::sets;

// Helper to build a boolean set from multiple elements
template<typename T>
boolean_set<T, 32> build_set(const std::vector<T>& elements,
                              trapdoor_factory<32>& factory) {
    boolean_set<T, 32> result;
    for (const auto& elem : elements) {
        auto td = factory.create(elem);
        boolean_set<T, 32> elem_set(td.hash(), td.key_fingerprint());
        result = result | elem_set;
    }
    return result;
}

// Visualize set operations with ASCII art
void visualize_sets() {
    std::cout << "═══════════════════════════════════════════════════\n";
    std::cout << "     VISUALIZING SET OPERATIONS                   \n";
    std::cout << "═══════════════════════════════════════════════════\n\n";

    std::cout << "Set A: {Alice, Bob, Charlie}\n";
    std::cout << "Set B: {Bob, Diana, Eve}\n\n";

    std::cout << "Visual Representation:\n";
    std::cout << "─────────────────────\n\n";

    std::cout << "     Set A          Set B\n";
    std::cout << "   ┌───────┐      ┌───────┐\n";
    std::cout << "   │ Alice │      │       │\n";
    std::cout << "   │   ┌───┼──────┼─Bob   │\n";
    std::cout << "   │   │   │      │       │\n";
    std::cout << "   │Charlie│      │ Diana │\n";
    std::cout << "   │       │      │  Eve  │\n";
    std::cout << "   └───────┘      └───────┘\n\n";

    std::cout << "Union (A ∪ B): {Alice, Bob, Charlie, Diana, Eve}\n";
    std::cout << "Intersection (A ∩ B): {Bob}\n";
    std::cout << "Difference (A - B): {Alice, Charlie}\n";
    std::cout << "Symmetric Diff (A ⊕ B): {Alice, Charlie, Diana, Eve}\n\n";
}

void demonstrate_union() {
    std::cout << "═══════════════════════════════════════════════════\n";
    std::cout << "     UNION OPERATION (A ∪ B)                      \n";
    std::cout << "═══════════════════════════════════════════════════\n\n";

    trapdoor_factory<32> factory("union-demo-key");

    // Two teams merging
    std::vector<std::string> team_alpha = {
        "alice@company.com",
        "bob@company.com",
        "charlie@company.com"
    };

    std::vector<std::string> team_beta = {
        "bob@company.com",    // Already in team_alpha
        "diana@company.com",
        "eve@company.com"
    };

    std::cout << "Team Alpha: ";
    for (const auto& member : team_alpha) {
        std::cout << member.substr(0, member.find('@')) << " ";
    }
    std::cout << "\n";

    std::cout << "Team Beta:  ";
    for (const auto& member : team_beta) {
        std::cout << member.substr(0, member.find('@')) << " ";
    }
    std::cout << "\n\n";

    // Build sets
    auto set_alpha = build_set(team_alpha, factory);
    auto set_beta = build_set(team_beta, factory);

    // Union operation
    auto merged_team = set_alpha | set_beta;

    std::cout << "Merged Team (Union):\n";
    std::cout << "───────────────────\n";

    // Test all possible members
    std::set<std::string> all_members;
    all_members.insert(team_alpha.begin(), team_alpha.end());
    all_members.insert(team_beta.begin(), team_beta.end());

    for (const auto& member : all_members) {
        auto td = factory.create(member);
        auto in_merged = merged_team.contains(td);

        if (in_merged.value()) {
            std::cout << "  ✓ " << member.substr(0, member.find('@')) << "\n";
        }
    }

    std::cout << "\nTotal unique members: " << all_members.size() << "\n\n";
}

void demonstrate_intersection() {
    std::cout << "═══════════════════════════════════════════════════\n";
    std::cout << "     INTERSECTION OPERATION (A ∩ B)               \n";
    std::cout << "═══════════════════════════════════════════════════\n\n";

    trapdoor_factory<32> factory("intersect-demo-key");

    // Find common interests between users
    std::vector<std::string> user1_interests = {
        "programming", "music", "hiking", "photography", "cooking"
    };

    std::vector<std::string> user2_interests = {
        "music", "sports", "cooking", "travel", "gaming"
    };

    std::cout << "User 1 interests: ";
    for (const auto& interest : user1_interests) std::cout << interest << " ";
    std::cout << "\n";

    std::cout << "User 2 interests: ";
    for (const auto& interest : user2_interests) std::cout << interest << " ";
    std::cout << "\n\n";

    // Build sets
    auto set1 = build_set(user1_interests, factory);
    auto set2 = build_set(user2_interests, factory);

    // Find common interests
    auto common = set1 & set2;

    std::cout << "Common Interests (Intersection):\n";
    std::cout << "────────────────────────────────\n";

    // Test all interests
    std::set<std::string> all_interests;
    all_interests.insert(user1_interests.begin(), user1_interests.end());
    all_interests.insert(user2_interests.begin(), user2_interests.end());

    int common_count = 0;
    for (const auto& interest : all_interests) {
        auto td = factory.create(interest);
        if (common.contains(td).value()) {
            std::cout << "  ✓ " << interest << "\n";
            common_count++;
        }
    }

    std::cout << "\nCompatibility Score: "
              << (common_count * 100.0 / all_interests.size()) << "%\n\n";
}

void demonstrate_difference() {
    std::cout << "═══════════════════════════════════════════════════\n";
    std::cout << "     DIFFERENCE OPERATION (A - B)                 \n";
    std::cout << "═══════════════════════════════════════════════════\n\n";

    trapdoor_factory<32> factory("diff-demo-key");

    // Permissions management
    std::vector<std::string> admin_permissions = {
        "read", "write", "delete", "modify", "admin", "backup"
    };

    std::vector<std::string> user_permissions = {
        "read", "write", "modify"
    };

    std::cout << "Admin permissions: ";
    for (const auto& perm : admin_permissions) std::cout << perm << " ";
    std::cout << "\n";

    std::cout << "User permissions:  ";
    for (const auto& perm : user_permissions) std::cout << perm << " ";
    std::cout << "\n\n";

    // Build sets
    auto admin_set = build_set(admin_permissions, factory);
    auto user_set = build_set(user_permissions, factory);

    // Find admin-only permissions
    auto admin_only = admin_set - user_set;

    std::cout << "Admin-Only Permissions (Difference):\n";
    std::cout << "────────────────────────────────────\n";

    for (const auto& perm : admin_permissions) {
        auto td = factory.create(perm);
        if (admin_only.contains(td).value()) {
            std::cout << "  ⚠️  " << perm << " (restricted)\n";
        }
    }

    std::cout << "\n";
}

void demonstrate_symmetric_difference() {
    std::cout << "═══════════════════════════════════════════════════\n";
    std::cout << "     SYMMETRIC DIFFERENCE (A ⊕ B)                 \n";
    std::cout << "═══════════════════════════════════════════════════\n\n";

    trapdoor_factory<32> factory("sym-diff-key");

    // File system changes
    std::vector<std::string> snapshot_before = {
        "file1.txt", "file2.txt", "file3.txt", "config.json", "data.csv"
    };

    std::vector<std::string> snapshot_after = {
        "file1.txt", "file3.txt", "config.json", "data.csv", "report.pdf", "backup.zip"
    };

    std::cout << "Files Before: ";
    for (const auto& file : snapshot_before) std::cout << file << " ";
    std::cout << "\n";

    std::cout << "Files After:  ";
    for (const auto& file : snapshot_after) std::cout << file << " ";
    std::cout << "\n\n";

    // Build sets
    auto before_set = build_set(snapshot_before, factory);
    auto after_set = build_set(snapshot_after, factory);

    // Find changes
    auto changes = before_set ^ after_set;

    std::cout << "File System Changes (Symmetric Difference):\n";
    std::cout << "───────────────────────────────────────────\n";

    // Check all files
    std::set<std::string> all_files;
    all_files.insert(snapshot_before.begin(), snapshot_before.end());
    all_files.insert(snapshot_after.begin(), snapshot_after.end());

    for (const auto& file : all_files) {
        auto td = factory.create(file);
        if (changes.contains(td).value()) {
            // Determine if added or removed
            bool was_before = std::find(snapshot_before.begin(),
                                         snapshot_before.end(), file) != snapshot_before.end();
            bool is_after = std::find(snapshot_after.begin(),
                                       snapshot_after.end(), file) != snapshot_after.end();

            if (was_before && !is_after) {
                std::cout << "  ❌ Deleted: " << file << "\n";
            } else if (!was_before && is_after) {
                std::cout << "  ✅ Added: " << file << "\n";
            }
        }
    }

    std::cout << "\n";
}

void demonstrate_complex_operations() {
    std::cout << "═══════════════════════════════════════════════════\n";
    std::cout << "     COMPLEX SET OPERATIONS                       \n";
    std::cout << "═══════════════════════════════════════════════════\n\n";

    trapdoor_factory<32> factory("complex-key");

    // Multiple department access control
    std::vector<std::string> dept_engineering = {"alice", "bob", "charlie"};
    std::vector<std::string> dept_sales = {"diana", "eve", "frank"};
    std::vector<std::string> dept_marketing = {"george", "helen", "alice"};  // Alice in both
    std::vector<std::string> managers = {"alice", "diana", "george"};

    std::cout << "Engineering: ";
    for (const auto& p : dept_engineering) std::cout << p << " ";
    std::cout << "\nSales: ";
    for (const auto& p : dept_sales) std::cout << p << " ";
    std::cout << "\nMarketing: ";
    for (const auto& p : dept_marketing) std::cout << p << " ";
    std::cout << "\nManagers: ";
    for (const auto& p : managers) std::cout << p << " ";
    std::cout << "\n\n";

    // Build sets
    auto set_eng = build_set(dept_engineering, factory);
    auto set_sales = build_set(dept_sales, factory);
    auto set_marketing = build_set(dept_marketing, factory);
    auto set_managers = build_set(managers, factory);

    // Complex query: Find all managers who are in technical departments
    // (Engineering ∪ Marketing) ∩ Managers
    auto technical = set_eng | set_marketing;
    auto tech_managers = technical & set_managers;

    std::cout << "Technical Managers ((Eng ∪ Marketing) ∩ Managers):\n";
    std::cout << "──────────────────────────────────────────────────\n";

    std::vector<std::string> all_people;
    all_people.insert(all_people.end(), dept_engineering.begin(), dept_engineering.end());
    all_people.insert(all_people.end(), dept_sales.begin(), dept_sales.end());
    all_people.insert(all_people.end(), dept_marketing.begin(), dept_marketing.end());

    // Remove duplicates
    std::sort(all_people.begin(), all_people.end());
    all_people.erase(std::unique(all_people.begin(), all_people.end()), all_people.end());

    for (const auto& person : all_people) {
        auto td = factory.create(person);
        if (tech_managers.contains(td).value()) {
            std::cout << "  ✓ " << person << "\n";
        }
    }

    std::cout << "\n";

    // Another complex query: Non-manager employees
    // (All Employees) - Managers
    auto all_employees = (set_eng | set_sales) | set_marketing;
    auto non_managers = all_employees - set_managers;

    std::cout << "Non-Manager Employees (All - Managers):\n";
    std::cout << "───────────────────────────────────────\n";

    for (const auto& person : all_people) {
        auto td = factory.create(person);
        if (non_managers.contains(td).value()) {
            std::cout << "  • " << person << "\n";
        }
    }

    std::cout << "\n";
}

void demonstrate_privacy_preserving_analytics() {
    std::cout << "═══════════════════════════════════════════════════\n";
    std::cout << "     PRIVACY-PRESERVING ANALYTICS                 \n";
    std::cout << "═══════════════════════════════════════════════════\n\n";

    trapdoor_factory<32> factory("analytics-key");

    // Simulate customer data from multiple stores
    std::map<std::string, std::vector<std::string>> store_customers = {
        {"Store_A", {"cust001", "cust002", "cust003", "cust004", "cust005"}},
        {"Store_B", {"cust003", "cust004", "cust006", "cust007", "cust008"}},
        {"Store_C", {"cust001", "cust005", "cust008", "cust009", "cust010"}}
    };

    // Build encrypted sets for each store
    std::map<std::string, boolean_set<std::string, 32>> store_sets;

    for (const auto& [store, customers] : store_customers) {
        store_sets[store] = build_set(customers, factory);
        std::cout << store << " has " << customers.size() << " customers\n";
    }

    std::cout << "\nPrivacy-Preserved Analytics:\n";
    std::cout << "────────────────────────────\n";

    // 1. Find customers shopping at multiple stores
    auto multi_store = (store_sets["Store_A"] & store_sets["Store_B"]) |
                        (store_sets["Store_A"] & store_sets["Store_C"]) |
                        (store_sets["Store_B"] & store_sets["Store_C"]);

    // 2. Find exclusive customers for each store
    auto exclusive_a = store_sets["Store_A"] -
                       (store_sets["Store_B"] | store_sets["Store_C"]);

    // 3. Find customers shopping at all stores
    auto all_stores = store_sets["Store_A"] &
                       store_sets["Store_B"] &
                       store_sets["Store_C"];

    // Count results (in practice, we'd use cardinality estimation)
    std::set<std::string> all_customers;
    for (const auto& [_, custs] : store_customers) {
        all_customers.insert(custs.begin(), custs.end());
    }

    int multi_count = 0, exclusive_count = 0, all_count = 0;

    for (const auto& cust : all_customers) {
        auto td = factory.create(cust);

        if (multi_store.contains(td).value()) multi_count++;
        if (exclusive_a.contains(td).value()) exclusive_count++;
        if (all_stores.contains(td).value()) all_count++;
    }

    std::cout << "• Customers shopping at 2+ stores: ~" << multi_count << "\n";
    std::cout << "• Store A exclusive customers: ~" << exclusive_count << "\n";
    std::cout << "• Customers at all 3 stores: ~" << all_count << "\n";

    std::cout << "\n✓ All analytics computed without revealing individual customers!\n\n";
}

int main() {
    std::cout << "\n╔═══════════════════════════════════════════════════╗\n";
    std::cout << "║    PRIVATE SET OPERATIONS TUTORIAL               ║\n";
    std::cout << "╚═══════════════════════════════════════════════════╝\n\n";

    // Visualize concepts
    visualize_sets();

    // Demonstrate each operation
    demonstrate_union();
    demonstrate_intersection();
    demonstrate_difference();
    demonstrate_symmetric_difference();

    // Advanced examples
    demonstrate_complex_operations();
    demonstrate_privacy_preserving_analytics();

    std::cout << "╔═══════════════════════════════════════════════════╗\n";
    std::cout << "║                 KEY TAKEAWAYS                    ║\n";
    std::cout << "╚═══════════════════════════════════════════════════╝\n\n";

    std::cout << "✓ Set operations work on encrypted data\n";
    std::cout << "✓ Original values never exposed\n";
    std::cout << "✓ Complex queries possible with Boolean algebra\n";
    std::cout << "✓ Perfect for privacy-preserving analytics\n";
    std::cout << "✓ Homomorphic: operations preserve encryption\n\n";

    std::cout << "Next: Try Tutorial 4 for advanced analytics\n\n";

    return 0;
}