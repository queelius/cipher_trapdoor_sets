#pragma once

#include "../core/hash_types.hpp"
#include "../core/approximate_value.hpp"
#include "../trapdoor.hpp"
#include "../sets/symmetric_difference_set.hpp"
#include "../sets/boolean_set.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <bit>

namespace cts::operations {

/**
 * HyperLogLog-style cardinality estimation for trapdoor sets
 *
 * Provides privacy-preserving cardinality estimation with configurable precision.
 * Uses the hash structure of trapdoor sets to estimate unique element counts
 * without revealing the actual elements.
 */
template <std::size_t N = 32>
class cardinality_estimator {
    static constexpr std::size_t precision_bits = 6;  // 2^6 = 64 buckets
    static constexpr std::size_t num_buckets = 1 << precision_bits;
    static constexpr double alpha = 0.7213 / (1.0 + 1.079 / num_buckets); // HLL bias correction

public:
    /**
     * Estimate cardinality from a symmetric difference set
     * Returns approximate count with error bounds
     */
    template <typename T>
    static core::approximate_value<std::size_t> estimate(
        sets::symmetric_difference_set<T, N> const& set) {

        return estimate_from_hash(set.hash());
    }

    /**
     * Estimate cardinality from a boolean set
     */
    template <typename T>
    static core::approximate_value<std::size_t> estimate(
        sets::boolean_set<T, N> const& set) {

        return estimate_from_hash(set.hash());
    }

    /**
     * Compare cardinalities of two sets without revealing exact counts
     * Returns: -1 if |a| < |b|, 0 if |a| ≈ |b|, 1 if |a| > |b|
     */
    template <typename SetType>
    static core::approximate_value<int> compare_cardinality(
        SetType const& a, SetType const& b) {

        auto card_a = estimate(a);
        auto card_b = estimate(b);

        // Use relative difference threshold
        double threshold = 0.1;  // 10% difference threshold
        double rel_diff = std::abs(static_cast<double>(card_a.value()) -
                                  static_cast<double>(card_b.value())) /
                         std::max(card_a.value(), card_b.value());

        int result;
        if (rel_diff < threshold) {
            result = 0;  // Approximately equal
        } else if (card_a.value() < card_b.value()) {
            result = -1;
        } else {
            result = 1;
        }

        // Combine error rates
        double combined_error = 1.0 - (1.0 - card_a.error_rate()) *
                                      (1.0 - card_b.error_rate());
        return core::approximate_value<int>(result, combined_error);
    }

    /**
     * Estimate union cardinality using inclusion-exclusion principle
     * |A ∪ B| = |A| + |B| - |A ∩ B|
     */
    template <typename T>
    static core::approximate_value<std::size_t> estimate_union(
        sets::boolean_set<T, N> const& a,
        sets::boolean_set<T, N> const& b) {

        auto card_a = estimate(a);
        auto card_b = estimate(b);
        auto card_intersection = estimate(a & b);

        std::size_t union_size = card_a.value() + card_b.value() -
                                 card_intersection.value();

        // Propagate error bounds
        double combined_error = 1.0 - (1.0 - card_a.error_rate()) *
                                      (1.0 - card_b.error_rate()) *
                                      (1.0 - card_intersection.error_rate());

        return core::approximate_value<std::size_t>(union_size, combined_error);
    }

private:
    /**
     * Core estimation logic using hash structure
     */
    static core::approximate_value<std::size_t> estimate_from_hash(
        core::hash_value<N> const& hash) {

        // Use different parts of hash for different buckets
        std::vector<std::uint8_t> buckets(num_buckets, 0);

        // Process hash bytes to fill buckets
        for (std::size_t i = 0; i < N && i < num_buckets; ++i) {
            // Count leading zeros (simulating HLL's rho function)
            std::uint8_t byte = hash.data[i];
            if (byte == 0) {
                buckets[i % num_buckets] = 8;  // All zeros
            } else {
                buckets[i % num_buckets] = std::countr_zero(byte);
            }
        }

        // Harmonic mean calculation (HLL formula)
        double raw_estimate = alpha * num_buckets * num_buckets /
                             std::accumulate(buckets.begin(), buckets.end(), 0.0,
                                           [](double sum, std::uint8_t m) {
                                               return sum + std::pow(2.0, -m);
                                           });

        // Apply bias correction for small cardinalities
        std::size_t estimate = static_cast<std::size_t>(raw_estimate);
        if (estimate < 2.5 * num_buckets) {
            std::size_t zeros = std::count(buckets.begin(), buckets.end(), 0);
            if (zeros != 0) {
                estimate = num_buckets * std::log(static_cast<double>(num_buckets) / zeros);
            }
        }

        // Standard error for HyperLogLog: ~1.04/sqrt(m) where m is num_buckets
        double error_rate = 1.04 / std::sqrt(num_buckets);

        return core::approximate_value<std::size_t>(estimate, error_rate);
    }
};

/**
 * Linear Counting for small cardinalities
 * More accurate than HLL for small sets
 */
template <std::size_t N = 32>
class linear_counter {
    static constexpr std::size_t bit_array_size = N * 8;  // Total bits available

public:
    /**
     * Estimate cardinality using linear counting algorithm
     * Best for small sets (< 1000 elements)
     */
    template <typename T>
    static core::approximate_value<std::size_t> estimate(
        sets::symmetric_difference_set<T, N> const& set) {

        return estimate_from_hash(set.hash());
    }

    /**
     * Estimate cardinality from boolean set
     */
    template <typename T>
    static core::approximate_value<std::size_t> estimate(
        sets::boolean_set<T, N> const& set) {

        return estimate_from_hash(set.hash());
    }

private:
    static core::approximate_value<std::size_t> estimate_from_hash(
        core::hash_value<N> const& hash) {

        // Count set bits
        std::size_t set_bits = 0;
        for (std::size_t i = 0; i < N; ++i) {
            set_bits += std::popcount(hash.data[i]);
        }

        // Linear counting formula: -m * ln(V_n / m)
        // where m is total bits, V_n is number of zero bits
        std::size_t zero_bits = bit_array_size - set_bits;

        if (zero_bits == 0) {
            // All bits set, likely overflow
            return core::approximate_value<std::size_t>(
                bit_array_size, 0.5);  // High uncertainty
        }

        double estimate = -bit_array_size *
                         std::log(static_cast<double>(zero_bits) / bit_array_size);

        // Error rate depends on load factor
        double load_factor = static_cast<double>(set_bits) / bit_array_size;
        double error_rate = std::sqrt(std::exp(load_factor) - load_factor - 1) /
                           std::sqrt(bit_array_size);

        return core::approximate_value<std::size_t>(
            static_cast<std::size_t>(estimate), error_rate);
    }
};

/**
 * Adaptive cardinality estimator
 * Automatically selects best algorithm based on estimated size
 */
template <std::size_t N = 32>
class adaptive_cardinality {
public:
    template <typename SetType>
    static core::approximate_value<std::size_t> estimate(SetType const& set) {
        // First, get a rough estimate using linear counting
        auto linear_est = linear_counter<N>::estimate(set);

        // Choose algorithm based on estimate
        if (linear_est.value() < 100) {
            // Small cardinality: use linear counting
            return linear_est;
        } else {
            // Large cardinality: use HyperLogLog
            return cardinality_estimator<N>::estimate(set);
        }
    }
};

} // namespace cts::operations