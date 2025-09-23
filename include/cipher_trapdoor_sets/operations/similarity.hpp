#pragma once

#include "../core/hash_types.hpp"
#include "../core/approximate_value.hpp"
#include "../sets/boolean_set.hpp"
#include "../sets/symmetric_difference_set.hpp"
#include "cardinality.hpp"
#include <vector>
#include <algorithm>
#include <limits>
#include <random>

namespace cts::operations {

/**
 * Set similarity operations with privacy preservation
 *
 * Implements various similarity metrics for trapdoored sets
 * including Jaccard similarity, MinHash, and LSH.
 */

/**
 * Jaccard similarity estimation
 * J(A, B) = |A ∩ B| / |A ∪ B|
 */
template <std::size_t N = 32>
class jaccard_similarity {
public:
    /**
     * Estimate Jaccard similarity between two boolean sets
     */
    template <typename T>
    static core::approximate_value<double> estimate(
        sets::boolean_set<T, N> const& a,
        sets::boolean_set<T, N> const& b) {

        // Get intersection and union
        auto intersection = a & b;
        auto union_set = a | b;

        // Estimate cardinalities
        auto card_inter = cardinality_estimator<N>::estimate(intersection);
        auto card_union = cardinality_estimator<N>::estimate(union_set);

        if (card_union.value() == 0) {
            // Both sets are empty
            return core::approximate_value<double>(1.0, 0.0);
        }

        double similarity = static_cast<double>(card_inter.value()) /
                           static_cast<double>(card_union.value());

        // Propagate error from cardinality estimates
        double error_rate = std::max(card_inter.error_rate(),
                                     card_union.error_rate());

        return core::approximate_value<double>(similarity, error_rate);
    }

    /**
     * Estimate similarity for symmetric difference sets
     */
    template <typename T>
    static core::approximate_value<double> estimate(
        sets::symmetric_difference_set<T, N> const& a,
        sets::symmetric_difference_set<T, N> const& b) {

        // For symmetric difference sets, we use hash overlap
        auto const& hash_a = a.hash();
        auto const& hash_b = b.hash();

        std::size_t common_bits = 0;
        std::size_t total_bits = 0;

        for (std::size_t i = 0; i < N; ++i) {
            std::uint8_t common = hash_a.data[i] & hash_b.data[i];
            std::uint8_t either = hash_a.data[i] | hash_b.data[i];

            common_bits += std::popcount(common);
            total_bits += std::popcount(either);
        }

        if (total_bits == 0) {
            return core::approximate_value<double>(1.0, 0.0);
        }

        double similarity = static_cast<double>(common_bits) / total_bits;
        double error_rate = 1.0 / std::sqrt(total_bits);  // Error decreases with more bits

        return core::approximate_value<double>(similarity, error_rate);
    }
};

/**
 * MinHash for set similarity
 * Creates compact signatures for similarity estimation
 */
template <typename T, std::size_t N = 32, std::size_t K = 128>
class minhash {
    trapdoor_factory<N> factory_;
    std::size_t num_hashes_;  // Number of hash functions

public:
    explicit minhash(std::string_view secret, std::size_t num_hashes = K)
        : factory_(secret), num_hashes_(num_hashes) {}

    /**
     * MinHash signature
     */
    struct signature {
        std::vector<std::uint32_t> values;
        std::size_t key_fingerprint;

        signature(std::size_t k, std::size_t kf)
            : values(k, std::numeric_limits<std::uint32_t>::max())
            , key_fingerprint(kf) {}
    };

    /**
     * Generate MinHash signature for a set of values
     */
    template <typename Container>
    signature generate_signature(Container const& values) {
        signature sig(num_hashes_, factory_.key_fingerprint());

        for (auto const& val : values) {
            // Generate multiple hash values
            for (std::size_t i = 0; i < num_hashes_; ++i) {
                // Create hash with salt
                auto salted_val = std::to_string(i) + std::to_string(std::hash<T>{}(val));
                auto td = factory_.create(salted_val);

                // Extract 32-bit value from hash
                std::uint32_t hash_val = 0;
                for (std::size_t j = 0; j < 4 && j < N; ++j) {
                    hash_val = (hash_val << 8) | td.hash().data[j];
                }

                // Keep minimum
                sig.values[i] = std::min(sig.values[i], hash_val);
            }
        }

        return sig;
    }

    /**
     * Estimate Jaccard similarity from signatures
     */
    core::approximate_value<double> estimate_similarity(
        signature const& sig_a,
        signature const& sig_b) {

        if (sig_a.key_fingerprint != sig_b.key_fingerprint) {
            throw std::invalid_argument("Incompatible signatures");
        }

        if (sig_a.values.size() != sig_b.values.size()) {
            throw std::invalid_argument("Signature size mismatch");
        }

        // Count matching minimum values
        std::size_t matches = 0;
        for (std::size_t i = 0; i < sig_a.values.size(); ++i) {
            if (sig_a.values[i] == sig_b.values[i]) {
                matches++;
            }
        }

        double similarity = static_cast<double>(matches) / sig_a.values.size();

        // Standard error for MinHash
        double error = std::sqrt(similarity * (1 - similarity) / num_hashes_);

        return core::approximate_value<double>(similarity, error);
    }
};

/**
 * Locality-Sensitive Hashing (LSH) for similarity search
 */
template <typename T, std::size_t N = 32>
class lsh_index {
    struct hash_family {
        std::vector<std::size_t> projection_indices;
        std::size_t band_size;
    };

    trapdoor_factory<N> factory_;
    std::vector<hash_family> families_;
    std::size_t num_bands_;
    std::size_t band_size_;

public:
    lsh_index(std::string_view secret,
              std::size_t num_bands = 20,
              std::size_t band_size = 5)
        : factory_(secret)
        , num_bands_(num_bands)
        , band_size_(band_size) {

        // Initialize hash families
        std::mt19937 rng(std::hash<std::string_view>{}(secret));
        for (std::size_t i = 0; i < num_bands_; ++i) {
            hash_family family;
            family.band_size = band_size_;

            // Random projections
            for (std::size_t j = 0; j < band_size_; ++j) {
                family.projection_indices.push_back(rng() % (N * 8));
            }

            families_.push_back(family);
        }
    }

    /**
     * LSH signature for a trapdoor
     */
    struct lsh_signature {
        std::vector<std::uint64_t> bands;
        std::size_t key_fingerprint;
    };

    /**
     * Generate LSH signature
     */
    lsh_signature generate_signature(trapdoor<T, N> const& td) {
        lsh_signature sig;
        sig.key_fingerprint = td.key_fingerprint();

        auto const& hash = td.hash();

        for (auto const& family : families_) {
            std::uint64_t band_hash = 0;

            for (auto idx : family.projection_indices) {
                std::size_t byte_idx = idx / 8;
                std::size_t bit_idx = idx % 8;

                if (byte_idx < N) {
                    bool bit = (hash.data[byte_idx] >> bit_idx) & 1;
                    band_hash = (band_hash << 1) | bit;
                }
            }

            sig.bands.push_back(band_hash);
        }

        return sig;
    }

    /**
     * Check if two items are likely similar
     */
    core::approximate_bool are_similar(
        lsh_signature const& sig_a,
        lsh_signature const& sig_b,
        double threshold = 0.5) {

        if (sig_a.key_fingerprint != sig_b.key_fingerprint) {
            return core::approximate_bool(false, 0.0, 1.0);
        }

        // Count matching bands
        std::size_t matches = 0;
        for (std::size_t i = 0; i < sig_a.bands.size(); ++i) {
            if (sig_a.bands[i] == sig_b.bands[i]) {
                matches++;
            }
        }

        // Probability of being similar
        double match_ratio = static_cast<double>(matches) / num_bands_;

        // LSH probability formula
        // P(match) ≈ 1 - (1 - s^r)^b where s is similarity, r is band size, b is num bands
        // Inverse: s ≈ (1 - (1 - match_ratio)^(1/b))^(1/r)

        double estimated_similarity = std::pow(
            1.0 - std::pow(1.0 - match_ratio, 1.0 / num_bands_),
            1.0 / band_size_
        );

        bool is_similar = estimated_similarity >= threshold;

        // Error based on number of bands
        double error_rate = 1.0 / std::sqrt(num_bands_);

        return core::approximate_bool(is_similar, error_rate, 0.0);
    }

    /**
     * Find approximate nearest neighbors
     */
    template <typename Container>
    std::vector<std::size_t> find_similar(
        trapdoor<T, N> const& query,
        Container const& candidates,
        double threshold = 0.5) {

        auto query_sig = generate_signature(query);
        std::vector<std::size_t> similar_indices;

        for (std::size_t i = 0; i < candidates.size(); ++i) {
            auto cand_sig = generate_signature(candidates[i]);
            auto is_similar = are_similar(query_sig, cand_sig, threshold);

            if (is_similar.value()) {
                similar_indices.push_back(i);
            }
        }

        return similar_indices;
    }
};

/**
 * Cosine similarity for hash vectors
 */
template <std::size_t N = 32>
class cosine_similarity {
public:
    /**
     * Compute cosine similarity between hash vectors
     */
    static core::approximate_value<double> compute(
        core::hash_value<N> const& a,
        core::hash_value<N> const& b) {

        double dot_product = 0.0;
        double norm_a = 0.0;
        double norm_b = 0.0;

        for (std::size_t i = 0; i < N; ++i) {
            double val_a = static_cast<double>(a.data[i]);
            double val_b = static_cast<double>(b.data[i]);

            dot_product += val_a * val_b;
            norm_a += val_a * val_a;
            norm_b += val_b * val_b;
        }

        if (norm_a == 0.0 || norm_b == 0.0) {
            return core::approximate_value<double>(0.0, 0.0);
        }

        double similarity = dot_product / (std::sqrt(norm_a) * std::sqrt(norm_b));

        // Error based on hash size
        double error_rate = 1.0 / (N * 8);

        return core::approximate_value<double>(similarity, error_rate);
    }

    /**
     * Compute similarity for trapdoors
     */
    template <typename T>
    static core::approximate_value<double> compute(
        trapdoor<T, N> const& a,
        trapdoor<T, N> const& b) {

        if (a.key_fingerprint() != b.key_fingerprint()) {
            throw std::invalid_argument("Incompatible trapdoors");
        }

        return compute(a.hash(), b.hash());
    }
};

} // namespace cts::operations