#pragma once

#include "../core/hash_types.hpp"
#include "../trapdoor.hpp"
#include "../sets/symmetric_difference_set.hpp"
#include "../sets/boolean_set.hpp"
#include <vector>
#include <span>
#include <cstring>

namespace cts::serialization {

/**
 * Binary serialization for trapdoor types
 * Simple, efficient format for storage and transmission
 */
class binary_serializer {
public:
    // Serialize hash_value to bytes
    template <std::size_t N>
    static std::vector<std::uint8_t> serialize(core::hash_value<N> const& h) {
        std::vector<std::uint8_t> result(N);
        std::memcpy(result.data(), h.data.data(), N);
        return result;
    }
    
    // Serialize trapdoor
    template <typename T, std::size_t N>
    static std::vector<std::uint8_t> serialize(trapdoor<T, N> const& td) {
        std::vector<std::uint8_t> result;
        result.reserve(N + sizeof(std::size_t));
        
        // Serialize hash
        auto hash_bytes = serialize(td.hash());
        result.insert(result.end(), hash_bytes.begin(), hash_bytes.end());
        
        // Serialize key fingerprint
        auto kf = td.key_fingerprint();
        auto kf_bytes = reinterpret_cast<std::uint8_t const*>(&kf);
        result.insert(result.end(), kf_bytes, kf_bytes + sizeof(kf));
        
        return result;
    }
    
    // Serialize symmetric_difference_set
    template <typename T, std::size_t N>
    static std::vector<std::uint8_t> serialize(sets::symmetric_difference_set<T, N> const& s) {
        std::vector<std::uint8_t> result;
        result.reserve(N + sizeof(std::size_t));

        auto hash_bytes = serialize(s.hash());
        result.insert(result.end(), hash_bytes.begin(), hash_bytes.end());

        auto kf = s.key_fingerprint();
        auto kf_bytes = reinterpret_cast<std::uint8_t const*>(&kf);
        result.insert(result.end(), kf_bytes, kf_bytes + sizeof(kf));

        return result;
    }

    // Serialize boolean_set
    template <typename T, std::size_t N>
    static std::vector<std::uint8_t> serialize(sets::boolean_set<T, N> const& s) {
        std::vector<std::uint8_t> result;
        result.reserve(N + sizeof(std::size_t));

        auto hash_bytes = serialize(s.hash());
        result.insert(result.end(), hash_bytes.begin(), hash_bytes.end());

        auto kf = s.key_fingerprint();
        auto kf_bytes = reinterpret_cast<std::uint8_t const*>(&kf);
        result.insert(result.end(), kf_bytes, kf_bytes + sizeof(kf));

        return result;
    }
};

/**
 * Binary deserialization
 */
class binary_deserializer {
public:
    // Deserialize hash_value
    template <std::size_t N>
    static core::hash_value<N> deserialize_hash(std::span<std::uint8_t const> bytes) {
        if (bytes.size() < N) {
            throw std::invalid_argument("Insufficient bytes for hash deserialization");
        }
        
        core::hash_value<N> result;
        std::memcpy(result.data.data(), bytes.data(), N);
        return result;
    }
    
    // Deserialize trapdoor
    template <typename T, std::size_t N>
    static trapdoor<T, N> deserialize_trapdoor(std::span<std::uint8_t const> bytes) {
        if (bytes.size() < N + sizeof(std::size_t)) {
            throw std::invalid_argument("Insufficient bytes for trapdoor deserialization");
        }
        
        auto hash = deserialize_hash<N>(bytes.subspan(0, N));
        
        std::size_t kf;
        std::memcpy(&kf, bytes.data() + N, sizeof(kf));
        
        return trapdoor<T, N>(hash, kf);
    }
    
    // Deserialize symmetric_difference_set
    template <typename T, std::size_t N>
    static sets::symmetric_difference_set<T, N> deserialize_sym_diff_set(
        std::span<std::uint8_t const> bytes) {

        if (bytes.size() < N + sizeof(std::size_t)) {
            throw std::invalid_argument("Insufficient bytes for set deserialization");
        }

        auto hash = deserialize_hash<N>(bytes.subspan(0, N));

        std::size_t kf;
        std::memcpy(&kf, bytes.data() + N, sizeof(kf));

        return sets::symmetric_difference_set<T, N>(hash, kf);
    }

    // Deserialize boolean_set
    template <typename T, std::size_t N>
    static sets::boolean_set<T, N> deserialize_boolean_set(
        std::span<std::uint8_t const> bytes) {

        if (bytes.size() < N + sizeof(std::size_t)) {
            throw std::invalid_argument("Insufficient bytes for boolean set deserialization");
        }

        auto hash = deserialize_hash<N>(bytes.subspan(0, N));

        std::size_t kf;
        std::memcpy(&kf, bytes.data() + N, sizeof(kf));

        return sets::boolean_set<T, N>(hash, kf);
    }
};

} // namespace cts::serialization