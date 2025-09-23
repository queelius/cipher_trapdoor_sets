#pragma once

#include "../core/hash_types.hpp"
#include "../core/approximate_value.hpp"
#include "../trapdoor.hpp"
#include <cmath>
#include <stdexcept>
#include <string>

namespace cts::sets {

/**
 * Boolean Set - Full Boolean algebra operations
 * 
 * Implements Boolean algebra with operations:
 * - Union (OR)
 * - Intersection (AND)
 * - Complement (NOT)
 * - Difference
 * - Symmetric difference (XOR)
 * 
 * Provides approximate membership and subset testing
 */
template <typename T, std::size_t N = 32>
class boolean_set {
public:
    using value_type = T;
    using hash_type = core::hash_value<N>;
    
    // Construct empty set
    constexpr boolean_set() = default;
    
    // Construct from hash
    explicit constexpr boolean_set(hash_type h, std::size_t kf)
        : hash_(h), key_fingerprint_(kf) {}
    
    // Boolean operations
    boolean_set operator|(boolean_set const& other) const {  // Union
        verify_compatible(other);
        return boolean_set(hash_ | other.hash_, key_fingerprint_);
    }
    
    boolean_set operator&(boolean_set const& other) const {  // Intersection
        verify_compatible(other);
        return boolean_set(hash_ & other.hash_, key_fingerprint_);
    }
    
    boolean_set operator^(boolean_set const& other) const {  // Symmetric difference
        verify_compatible(other);
        return boolean_set(hash_ ^ other.hash_, key_fingerprint_);
    }
    
    boolean_set operator~() const {  // Complement
        return boolean_set(~hash_, key_fingerprint_);
    }
    
    boolean_set operator-(boolean_set const& other) const {  // Difference
        return *this & ~other;
    }
    
    // Approximate membership test
    core::approximate_bool contains(trapdoor<T, N> const& elem) const {
        if (elem.key_fingerprint() != key_fingerprint_) {
            throw std::invalid_argument("Incompatible key");
        }
        
        // Check if elem's bits are all set in our hash
        auto masked = hash_ & elem.hash();
        bool all_bits_set = (masked == elem.hash());
        
        // False positive rate depends on hash size and set density
        double fpr = 0.5;  // Conservative estimate
        return core::approximate_bool(all_bits_set, fpr, 0.0);
    }
    
    // Approximate subset test
    core::approximate_bool subset_of(boolean_set const& other) const {
        verify_compatible(other);
        
        // A ⊆ B iff A ∩ B = A
        bool is_subset = ((hash_ & other.hash_) == hash_);
        double fpr = 0.5;  // Conservative estimate
        return core::approximate_bool(is_subset, fpr, 0.0);
    }
    
    // Test if empty
    core::approximate_bool empty() const noexcept {
        bool is_zero = hash_.is_zero();
        double fpr = std::pow(2.0, -static_cast<double>(N * 8));
        return core::approximate_bool(is_zero, fpr, 0.0);
    }
    
    // Test if universal set
    core::approximate_bool universal() const noexcept {
        bool is_ones = hash_.is_ones();
        double fpr = std::pow(2.0, -static_cast<double>(N * 8));
        return core::approximate_bool(is_ones, fpr, 0.0);
    }
    
    // Equality
    core::approximate_bool equals(boolean_set const& other) const {
        verify_compatible(other);
        bool same = (hash_ == other.hash_);
        double fpr = std::pow(2.0, -static_cast<double>(N * 8));
        return core::approximate_bool(same, fpr, 0.0);
    }
    
    hash_type const& hash() const noexcept { return hash_; }
    std::size_t key_fingerprint() const noexcept { return key_fingerprint_; }
    
private:
    void verify_compatible(boolean_set const& other) const {
        if (key_fingerprint_ != other.key_fingerprint_) {
            throw std::invalid_argument("Incompatible keys in set operation");
        }
    }
    
    hash_type hash_{};
    std::size_t key_fingerprint_{0};
};

// Factory for Boolean sets with Bloom filter-like construction
template <typename T, std::size_t N = 32>
class boolean_set_factory {
    trapdoor_factory<N> trapdoor_factory_;
    std::size_t num_hashes_;  // Number of hash functions to use
    
public:
    explicit boolean_set_factory(std::string_view secret, std::size_t num_hashes = 3)
        : trapdoor_factory_(secret), num_hashes_(num_hashes) {}
    
    // Create empty set
    boolean_set<T, N> empty() const {
        core::hash_value<N> h;
        return boolean_set<T, N>(h, trapdoor_factory_.key_fingerprint());
    }
    
    // Create universal set
    boolean_set<T, N> universal() const {
        core::hash_value<N> h;
        for (auto& byte : h.data) byte = 0xFF;
        return boolean_set<T, N>(h, trapdoor_factory_.key_fingerprint());
    }
    
    // Create singleton with multiple hash functions (Bloom filter style)
    boolean_set<T, N> singleton(T const& value) const {
        core::hash_value<N> result;
        
        // Use multiple hash functions to set bits
        for (std::size_t i = 0; i < num_hashes_; ++i) {
            auto modified_value = std::to_string(i) + std::to_string(std::hash<T>{}(value));
            auto td = trapdoor_factory_.create(modified_value);
            
            // Set specific bits based on hash
            for (std::size_t j = 0; j < N; ++j) {
                if (td.hash().data[j] & 0x01) {
                    result.data[j] |= (1 << (i % 8));
                }
            }
        }
        
        return boolean_set<T, N>(result, trapdoor_factory_.key_fingerprint());
    }
    
    // Create set from collection
    template <typename Container>
    boolean_set<T, N> from_collection(Container const& values) const {
        boolean_set<T, N> result = empty();
        for (auto const& v : values) {
            result = result | singleton(v);
        }
        return result;
    }
};

} // namespace cts::sets