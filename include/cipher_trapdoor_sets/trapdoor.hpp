#pragma once

#include "core/hash_types.hpp"
#include "core/approximate_value.hpp"
#include <algorithm>
#include <concepts>
#include <cmath>
#include <functional>
#include <stdexcept>
#include <string_view>
#include <type_traits>

namespace cts {

/**
 * Trapdoor: A one-way transformation providing privacy-preserving operations
 *
 * A trapdoor<T, N> represents a one-way hash-based transformation of values
 * of type T into N-byte hash values. This provides:
 *
 * 1. Preimage resistance: Given hash h, finding x where h = H(x) is hard
 * 2. Collision resistance: Finding x != y where H(x) = H(y) is hard
 * 3. Privacy preservation: Operations reveal minimal information about T
 *
 * Template parameters:
 *   T - The value type being protected
 *   N - Size of hash in bytes (default 32 for 256-bit security)
 */
template <typename T, std::size_t N = 32>
class trapdoor {
public:
    using value_type = T;
    using hash_type = core::hash_value<N>;

    static constexpr std::size_t hash_bytes = N;
    static constexpr std::size_t hash_bits = N * 8;

    // Construct empty/null trapdoor
    constexpr trapdoor() = default;

    // Construct from hash values (for internal use)
    constexpr trapdoor(hash_type value_hash, std::size_t key_fingerprint)
        : value_hash_(value_hash)
        , key_fingerprint_(key_fingerprint) {}

    // Hash accessors
    hash_type const& hash() const noexcept { return value_hash_; }
    std::size_t key_fingerprint() const noexcept { return key_fingerprint_; }

    // Check compatibility (same key)
    bool compatible_with(trapdoor const& other) const noexcept {
        return key_fingerprint_ == other.key_fingerprint_;
    }

    // Approximate equality with explicit error rate
    core::approximate_bool equals(trapdoor const& other) const {
        if (!compatible_with(other)) {
            throw std::invalid_argument("Incompatible trapdoor keys");
        }

        bool same = (value_hash_ == other.value_hash_);
        double fpr = std::pow(2.0, -static_cast<double>(hash_bits));

        // If hashes match, could be collision (false positive)
        // If hashes differ, definitely different (no false negative)
        return core::approximate_bool(same, fpr, 0.0);
    }

private:
    hash_type value_hash_{};
    std::size_t key_fingerprint_{0};
};

/**
 * Factory for creating trapdoor values
 * Encapsulates the key and provides consistent trapdoor generation
 */
template <std::size_t N = 32>
class trapdoor_factory {
    core::key_derivation<> key_;

public:
    explicit trapdoor_factory(std::string_view secret_key)
        : key_(secret_key) {}

    // Create trapdoor from any hashable value
    template <typename T>
    requires requires(T t) { std::hash<T>{}(t); }
    trapdoor<T, N> create(T const& value) const {
        auto hash = key_.template derive<N>(value);
        return trapdoor<T, N>(hash, key_.key_fingerprint());
    }

    // Get key fingerprint for verification
    std::size_t key_fingerprint() const noexcept {
        return key_.key_fingerprint();
    }
};

// Convenience function for one-off trapdoor creation
template <std::size_t N = 32, typename T>
trapdoor<T, N> make_trapdoor(T const& value, std::string_view secret) {
    trapdoor_factory<N> factory(secret);
    return factory.create(value);
}

// Operators for approximate equality
template <typename T, std::size_t N>
core::approximate_bool operator==(trapdoor<T, N> const& lhs,
                                 trapdoor<T, N> const& rhs) {
    return lhs.equals(rhs);
}

template <typename T, std::size_t N>
core::approximate_bool operator!=(trapdoor<T, N> const& lhs,
                                 trapdoor<T, N> const& rhs) {
    return !lhs.equals(rhs);
}

} // namespace cts

// Hash support for standard containers
namespace std {
    template <typename T, std::size_t N>
    struct hash<cts::trapdoor<T, N>> {
        std::size_t operator()(cts::trapdoor<T, N> const& t) const noexcept {
            // Simple hash combination of the internal hash bytes
            std::size_t result = 0;
            auto const& h = t.hash();
            for (std::size_t i = 0; i < std::min(N, sizeof(std::size_t)); ++i) {
                result = (result << 8) | h.data[i];
            }
            return result;
        }
    };
}

