#pragma once

#include "core/hash_types.hpp"
#include "trapdoor.hpp"
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <memory>
#include <algorithm>

namespace cts::key_management {

/**
 * Key derivation functions for trapdoor generation
 */
class key_derivation {
public:
    /**
     * PBKDF2-style key derivation
     * Derives a key from password and salt
     */
    static std::string derive_key(
        std::string_view password,
        std::string_view salt,
        std::size_t iterations = 10000) {

        std::string derived = std::string(password);

        for (std::size_t i = 0; i < iterations; ++i) {
            std::string combined = derived + std::string(salt) + std::to_string(i);
            std::hash<std::string> hasher;
            auto hash_val = hasher(combined);
            derived = std::to_string(hash_val);
        }

        return derived;
    }

    /**
     * HKDF-style key expansion
     * Expands a key into multiple derived keys
     */
    static std::vector<std::string> expand_key(
        std::string_view master_key,
        std::size_t num_keys,
        std::string_view info = "") {

        std::vector<std::string> keys;

        for (std::size_t i = 0; i < num_keys; ++i) {
            std::string context = std::string(master_key) +
                                 std::string(info) +
                                 std::to_string(i);
            keys.push_back(derive_key(context, "expand", 1000));
        }

        return keys;
    }
};

/**
 * Hierarchical key structure for multi-level access control
 */
template <std::size_t N = 32>
class hierarchical_keys {
public:
    struct key_node {
        std::string key;
        std::string label;
        std::vector<std::unique_ptr<key_node>> children;
        key_node* parent;

        key_node(std::string_view k, std::string_view l, key_node* p = nullptr)
            : key(k), label(l), parent(p) {}
    };

private:
    std::unique_ptr<key_node> root_;
    std::map<std::string, key_node*> label_map_;

public:
    /**
     * Initialize with master key
     */
    explicit hierarchical_keys(std::string_view master_key) {
        root_ = std::make_unique<key_node>(master_key, "root");
        label_map_["root"] = root_.get();
    }

    /**
     * Derive child key from parent
     */
    std::string derive_child_key(
        std::string_view parent_label,
        std::string_view child_label) {

        auto it = label_map_.find(std::string(parent_label));
        if (it == label_map_.end()) {
            throw std::invalid_argument("Parent key not found");
        }

        key_node* parent = it->second;

        // Derive child key from parent
        std::string child_key = key_derivation::derive_key(
            parent->key,
            child_label,
            5000
        );

        // Add to hierarchy
        auto child = std::make_unique<key_node>(child_key, child_label, parent);
        key_node* child_ptr = child.get();
        parent->children.push_back(std::move(child));
        label_map_[std::string(child_label)] = child_ptr;

        return child_key;
    }

    /**
     * Get key by label
     */
    std::string get_key(std::string_view label) const {
        auto it = label_map_.find(std::string(label));
        if (it == label_map_.end()) {
            throw std::invalid_argument("Key not found");
        }
        return it->second->key;
    }

    /**
     * Check if one key is ancestor of another
     */
    bool is_ancestor(std::string_view ancestor_label,
                     std::string_view descendant_label) const {
        auto anc_it = label_map_.find(std::string(ancestor_label));
        auto desc_it = label_map_.find(std::string(descendant_label));

        if (anc_it == label_map_.end() || desc_it == label_map_.end()) {
            return false;
        }

        key_node* current = desc_it->second;
        while (current != nullptr) {
            if (current == anc_it->second) {
                return true;
            }
            current = current->parent;
        }

        return false;
    }

    /**
     * Create trapdoor factory with hierarchical key
     */
    trapdoor_factory<N> create_factory(std::string_view label) const {
        return trapdoor_factory<N>(get_key(label));
    }
};

/**
 * Key rotation support for periodic key updates
 */
template <std::size_t N = 32>
class key_rotation {
public:
    struct versioned_key {
        std::string key;
        std::size_t version;
        std::chrono::system_clock::time_point created_at;
        std::chrono::system_clock::time_point expires_at;
        bool is_active;
    };

private:
    std::vector<versioned_key> keys_;
    std::size_t current_version_;
    std::chrono::seconds rotation_period_;

public:
    explicit key_rotation(
        std::string_view initial_key,
        std::chrono::seconds rotation_period = std::chrono::hours(24 * 30))
        : current_version_(0)
        , rotation_period_(rotation_period) {

        auto now = std::chrono::system_clock::now();
        keys_.push_back({
            std::string(initial_key),
            0,
            now,
            now + rotation_period_,
            true
        });
    }

    /**
     * Rotate to new key
     */
    std::string rotate_key() {
        auto now = std::chrono::system_clock::now();

        // Deactivate current key
        if (!keys_.empty()) {
            keys_.back().is_active = false;
        }

        // Generate new key from current
        std::string new_key = key_derivation::derive_key(
            keys_.back().key,
            std::to_string(++current_version_),
            10000
        );

        keys_.push_back({
            new_key,
            current_version_,
            now,
            now + rotation_period_,
            true
        });

        return new_key;
    }

    /**
     * Get current active key
     */
    std::string current_key() const {
        for (auto it = keys_.rbegin(); it != keys_.rend(); ++it) {
            if (it->is_active) {
                return it->key;
            }
        }
        throw std::runtime_error("No active key found");
    }

    /**
     * Get key by version
     */
    std::string get_key(std::size_t version) const {
        auto it = std::find_if(keys_.begin(), keys_.end(),
            [version](const versioned_key& k) {
                return k.version == version;
            });

        if (it == keys_.end()) {
            throw std::invalid_argument("Key version not found");
        }

        return it->key;
    }

    /**
     * Check if rotation is needed
     */
    bool needs_rotation() const {
        if (keys_.empty()) return true;

        auto now = std::chrono::system_clock::now();
        return now >= keys_.back().expires_at;
    }

    /**
     * Create trapdoor with versioned key
     */
    template <typename T>
    trapdoor<T, N> create_trapdoor(T const& value) const {
        trapdoor_factory<N> factory(current_key());
        return factory.create(value);
    }

    /**
     * Verify trapdoor with any valid key version
     */
    template <typename T>
    bool verify_trapdoor(trapdoor<T, N> const& td, T const& value) const {
        for (const auto& key : keys_) {
            trapdoor_factory<N> factory(key.key);
            auto test_td = factory.create(value);
            if (test_td == td) {
                return true;
            }
        }
        return false;
    }
};

/**
 * Key escrow for recovery scenarios
 */
class key_escrow {
private:
    struct escrow_share {
        std::string share;
        std::string holder_id;
        std::size_t threshold_index;
    };

    std::vector<escrow_share> shares_;
    std::size_t threshold_;
    std::size_t total_shares_;

public:
    /**
     * Split key into shares using Shamir's secret sharing
     * (simplified implementation)
     */
    std::vector<std::string> split_key(
        std::string_view key,
        std::size_t threshold,
        std::size_t total_shares) {

        if (threshold > total_shares) {
            throw std::invalid_argument("Threshold cannot exceed total shares");
        }

        threshold_ = threshold;
        total_shares_ = total_shares;

        std::vector<std::string> shares;

        // Simplified: XOR-based secret sharing
        // In production, use proper Shamir's secret sharing
        std::string key_str(key);

        for (std::size_t i = 0; i < total_shares - 1; ++i) {
            std::string share;
            for ([[maybe_unused]] char c : key_str) {
                share += char(std::rand() & 0xFF);
            }
            shares.push_back(share);
        }

        // Last share ensures XOR reconstruction
        std::string last_share = key_str;
        for (std::size_t i = 0; i < threshold - 1 && i < shares.size(); ++i) {
            for (std::size_t j = 0; j < last_share.size(); ++j) {
                last_share[j] ^= shares[i][j];
            }
        }
        shares.push_back(last_share);

        return shares;
    }

    /**
     * Reconstruct key from shares
     */
    std::string reconstruct_key(std::vector<std::string> const& shares) {
        if (shares.size() < threshold_) {
            throw std::invalid_argument("Insufficient shares for reconstruction");
        }

        // XOR first threshold shares
        std::string key = shares[0];
        for (std::size_t i = 1; i < threshold_; ++i) {
            for (std::size_t j = 0; j < key.size(); ++j) {
                key[j] ^= shares[i][j];
            }
        }

        return key;
    }

    /**
     * Store share with holder ID
     */
    void store_share(std::string_view share,
                     std::string_view holder_id,
                     std::size_t index) {
        shares_.push_back({
            std::string(share),
            std::string(holder_id),
            index
        });
    }

    /**
     * Retrieve shares by holder IDs
     */
    std::vector<std::string> retrieve_shares(
        std::vector<std::string> const& holder_ids) const {

        std::vector<std::string> result;

        for (const auto& id : holder_ids) {
            auto it = std::find_if(shares_.begin(), shares_.end(),
                [&id](const escrow_share& s) {
                    return s.holder_id == id;
                });

            if (it != shares_.end()) {
                result.push_back(it->share);
            }
        }

        return result;
    }
};

} // namespace cts::key_management