#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <string_view>

namespace cts::core {

/**
 * Strong type wrapper for hash values
 * Provides type safety and clear intent
 */
template <std::size_t N>
struct hash_value {
    static constexpr std::size_t byte_size = N;
    static constexpr std::size_t bit_size = N * 8;
    
    using storage_type = std::array<std::uint8_t, N>;
    
    storage_type data{};
    
    constexpr hash_value() = default;
    explicit constexpr hash_value(storage_type const& d) : data(d) {}
    
    // Bitwise operations for composability
    hash_value operator^(hash_value const& other) const noexcept {
        hash_value result;
        for (std::size_t i = 0; i < N; ++i) {
            result.data[i] = data[i] ^ other.data[i];
        }
        return result;
    }
    
    hash_value operator&(hash_value const& other) const noexcept {
        hash_value result;
        for (std::size_t i = 0; i < N; ++i) {
            result.data[i] = data[i] & other.data[i];
        }
        return result;
    }
    
    hash_value operator|(hash_value const& other) const noexcept {
        hash_value result;
        for (std::size_t i = 0; i < N; ++i) {
            result.data[i] = data[i] | other.data[i];
        }
        return result;
    }
    
    hash_value operator~() const noexcept {
        hash_value result;
        for (std::size_t i = 0; i < N; ++i) {
            result.data[i] = ~data[i];
        }
        return result;
    }
    
    bool operator==(hash_value const&) const noexcept = default;
    bool operator!=(hash_value const&) const noexcept = default;
    
    // Check if all bits are zero
    bool is_zero() const noexcept {
        for (auto byte : data) {
            if (byte != 0) return false;
        }
        return true;
    }
    
    // Check if all bits are one
    bool is_ones() const noexcept {
        for (auto byte : data) {
            if (byte != 0xFF) return false;
        }
        return true;
    }
};

// Common hash sizes
using hash_128 = hash_value<16>;
using hash_256 = hash_value<32>;
using hash_512 = hash_value<64>;

/**
 * Key derivation for trapdoor functions
 * Uses HMAC-like construction for security
 */
template <typename Hash = std::hash<std::string_view>>
class key_derivation {
    std::size_t master_key_hash;
    
public:
    explicit key_derivation(std::string_view master_key)
        : master_key_hash(Hash{}(master_key)) {}
    
    // Derive a hash from input and master key
    template <std::size_t N, typename T>
    hash_value<N> derive(T const& input) const {
        // Simplified derivation - in production use proper KDF
        std::size_t h = master_key_hash;
        hash_combine(h, input);
        
        hash_value<N> result;
        // Fill hash_value from size_t hash
        // This is simplified - use proper cryptographic hash in production
        for (std::size_t i = 0; i < N; ++i) {
            result.data[i] = static_cast<std::uint8_t>(h >> (i * 8));
            if (i % sizeof(std::size_t) == sizeof(std::size_t) - 1) {
                h = Hash{}(std::to_string(h));
            }
        }
        return result;
    }
    
    std::size_t key_fingerprint() const noexcept {
        return master_key_hash;
    }
    
private:
    template<typename T>
    static void hash_combine(std::size_t& seed, T const& v) {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
};

} // namespace cts::core