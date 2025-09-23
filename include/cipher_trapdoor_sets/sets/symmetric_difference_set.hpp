#pragma once

#include "../core/hash_types.hpp"
#include "../core/approximate_value.hpp"
#include "../trapdoor.hpp"
#include <cmath>
#include <stdexcept>
#include <vector>

namespace cts::sets {

/**
 * Symmetric Difference Set - Uses XOR for set operations
 * 
 * This implements a group under symmetric difference (XOR) operation.
 * Properties:
 * - Commutative: A ⊕ B = B ⊕ A
 * - Associative: (A ⊕ B) ⊕ C = A ⊕ (B ⊕ C)
 * - Identity: A ⊕ ∅ = A
 * - Inverse: A ⊕ A = ∅
 *
 * Supports only:
 * - Equality testing (approximate)
 * - Union of disjoint sets (via XOR)
 * - Emptiness testing (approximate)
 */
template <typename T, std::size_t N = 32>
class symmetric_difference_set {
public:
    using value_type = T;
    using hash_type = core::hash_value<N>;
    
    // Construct empty set
    constexpr symmetric_difference_set() = default;
    
    // Construct from hash (internal use)
    explicit constexpr symmetric_difference_set(hash_type h, std::size_t kf)
        : hash_(h), key_fingerprint_(kf) {}
    
    // Check if set is empty (approximate)
    core::approximate_bool empty() const noexcept {
        bool is_zero = hash_.is_zero();
        double fpr = std::pow(2.0, -static_cast<double>(N * 8));
        return core::approximate_bool(is_zero, fpr, 0.0);
    }
    
    // Symmetric difference (XOR) - for disjoint sets this is union
    symmetric_difference_set operator^(symmetric_difference_set const& other) const {
        verify_compatible(other);
        return symmetric_difference_set(hash_ ^ other.hash_, key_fingerprint_);
    }
    
    // Add single element (symmetric difference with singleton)
    symmetric_difference_set& operator^=(trapdoor<T, N> const& elem) {
        if (elem.key_fingerprint() != key_fingerprint_ && key_fingerprint_ != 0) {
            throw std::invalid_argument("Incompatible key");
        }
        if (key_fingerprint_ == 0) {
            key_fingerprint_ = elem.key_fingerprint();
        }
        hash_ = hash_ ^ elem.hash();
        return *this;
    }
    
    // Equality testing (approximate)
    core::approximate_bool equals(symmetric_difference_set const& other) const {
        verify_compatible(other);
        bool same = (hash_ == other.hash_);
        double fpr = std::pow(2.0, -static_cast<double>(N * 8));
        return core::approximate_bool(same, fpr, 0.0);
    }
    
    hash_type const& hash() const noexcept { return hash_; }
    std::size_t key_fingerprint() const noexcept { return key_fingerprint_; }
    
private:
    void verify_compatible(symmetric_difference_set const& other) const {
        if (key_fingerprint_ != 0 && other.key_fingerprint_ != 0 &&
            key_fingerprint_ != other.key_fingerprint_) {
            throw std::invalid_argument("Incompatible keys in set operation");
        }
    }
    
    hash_type hash_{};
    std::size_t key_fingerprint_{0};
};

// Factory for creating symmetric difference sets
template <typename T, std::size_t N = 32>
class symmetric_difference_set_factory {
    trapdoor_factory<N> trapdoor_factory_;
    
public:
    explicit symmetric_difference_set_factory(std::string_view secret)
        : trapdoor_factory_(secret) {}
    
    // Create empty set
    symmetric_difference_set<T, N> empty() const {
        return symmetric_difference_set<T, N>();
    }
    
    // Create singleton set
    symmetric_difference_set<T, N> singleton(T const& value) const {
        auto td = trapdoor_factory_.create(value);
        return symmetric_difference_set<T, N>(td.hash(), td.key_fingerprint());
    }
    
    // Create set from collection (must be unique elements)
    template <typename Container>
    symmetric_difference_set<T, N> from_unique(Container const& values) const {
        symmetric_difference_set<T, N> result;
        for (auto const& v : values) {
            auto td = trapdoor_factory_.create(v);
            result ^= td;
        }
        return result;
    }
};

} // namespace cts::sets