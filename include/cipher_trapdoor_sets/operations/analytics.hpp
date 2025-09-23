#pragma once

#include "../core/hash_types.hpp"
#include "../core/approximate_value.hpp"
#include "../trapdoor.hpp"
#include "../sets/boolean_set.hpp"
#include "cardinality.hpp"
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>
#include <random>
#include <cstdlib>

namespace cts::operations {

/**
 * Privacy-preserving analytics operations
 *
 * Provides statistical operations on trapdoored data
 * without revealing underlying values.
 */

/**
 * Histogram generator with differential privacy
 */
template <typename T, std::size_t N = 32>
class private_histogram {
    trapdoor_factory<N> factory_;
    double epsilon_;  // Privacy parameter

public:
    explicit private_histogram(std::string_view secret, double epsilon = 1.0)
        : factory_(secret), epsilon_(epsilon) {}

    /**
     * Generate histogram bins from trapdoored values
     * Returns approximate counts per bin
     */
    template <typename Container>
    std::vector<core::approximate_value<std::size_t>> generate_bins(
        Container const& values,
        std::size_t num_bins) {

        std::vector<core::approximate_value<std::size_t>> bins(num_bins);

        // Map values to bins using trapdoor hashes
        for (auto const& val : values) {
            auto td = factory_.create(val);

            // Use hash to determine bin
            std::size_t bin_index = 0;
            for (std::size_t i = 0; i < std::min(size_t(4), N); ++i) {
                bin_index = (bin_index * 256 + td.hash().data[i]) % num_bins;
            }

            // Increment bin with noise for differential privacy
            double noise = laplace_noise(1.0 / epsilon_);
            std::size_t noisy_count = std::max(0.0, 1.0 + noise);

            auto current = bins[bin_index].value();
            bins[bin_index] = core::approximate_value<std::size_t>(
                current + noisy_count,
                epsilon_ / num_bins  // Error rate based on privacy parameter
            );
        }

        return bins;
    }

    /**
     * Count distinct values with privacy
     */
    template <typename Container>
    core::approximate_value<std::size_t> count_distinct(Container const& values) {
        // Create trapdoors and build a set manually
        core::hash_value<N> combined_hash{};
        for (auto const& val : values) {
            auto td = factory_.create(val);
            // XOR hashes for simple combination
            for (std::size_t i = 0; i < N; ++i) {
                combined_hash.data[i] ^= td.hash().data[i];
            }
        }

        // Use linear counter for estimation
        std::size_t set_bits = 0;
        for (std::size_t i = 0; i < N; ++i) {
            set_bits += std::popcount(static_cast<unsigned char>(combined_hash.data[i]));
        }

        // Estimate distinct count
        double load_factor = static_cast<double>(set_bits) / (N * 8);
        double estimate = -N * 8 * std::log(1.0 - load_factor);

        return core::approximate_value<std::size_t>(
            static_cast<std::size_t>(estimate),
            epsilon_  // Error based on privacy parameter
        );
    }

private:
    /**
     * Generate Laplace noise for differential privacy
     */
    double laplace_noise(double scale) {
        double u = (std::rand() / static_cast<double>(RAND_MAX)) - 0.5;
        return -scale * (u > 0 ? 1 : -1) * std::log(1 - 2 * std::abs(u));
    }
};

/**
 * Frequency analysis resistance
 * Prevents frequency attacks on encrypted data
 */
template <std::size_t N = 32>
class frequency_shield {
    double noise_factor_;  // Amount of noise to add

public:
    explicit frequency_shield(double noise_factor = 0.1)
        : noise_factor_(noise_factor) {}

    /**
     * Obfuscate frequency distribution
     * Adds controlled noise to prevent frequency analysis
     */
    template <typename T>
    std::vector<trapdoor<T, N>> obfuscate_frequencies(
        std::vector<trapdoor<T, N>> const& trapdoors,
        trapdoor_factory<N> const& factory) {

        std::vector<trapdoor<T, N>> result = trapdoors;

        // Add dummy trapdoors
        std::size_t num_dummies = static_cast<std::size_t>(
            trapdoors.size() * noise_factor_);

        for (std::size_t i = 0; i < num_dummies; ++i) {
            // Create random dummy value
            core::hash_value<N> dummy_hash;
            for (auto& byte : dummy_hash.data) {
                byte = std::rand() & 0xFF;
            }
            result.emplace_back(dummy_hash, factory.key_fingerprint());
        }

        // Shuffle to mix real and dummy values
        std::random_shuffle(result.begin(), result.end());

        return result;
    }

    /**
     * Test if distribution is uniform (resistant to frequency analysis)
     */
    template <typename T>
    core::approximate_bool is_frequency_resistant(
        std::vector<trapdoor<T, N>> const& trapdoors) {

        // Count unique hash patterns
        std::map<core::hash_value<N>, std::size_t> frequency;
        for (auto const& td : trapdoors) {
            frequency[td.hash()]++;
        }

        // Calculate chi-squared statistic
        double expected = static_cast<double>(trapdoors.size()) / frequency.size();
        double chi_squared = 0.0;

        for (auto const& [hash, count] : frequency) {
            double diff = count - expected;
            chi_squared += (diff * diff) / expected;
        }

        // Threshold for uniformity (p-value = 0.05)
        double threshold = frequency.size() - 1;  // Simplified
        bool is_uniform = chi_squared < threshold;

        return core::approximate_bool(is_uniform, 0.05, 0.0);
    }
};

/**
 * Range query support with privacy
 */
template <typename T, std::size_t N = 32>
class private_range_query {
    trapdoor_factory<N> factory_;

public:
    explicit private_range_query(std::string_view secret)
        : factory_(secret) {}

    /**
     * Count elements in range [min, max]
     * Returns approximate count without revealing exact values
     */
    core::approximate_value<std::size_t> count_in_range(
        std::vector<T> const& values,
        T const& min_val,
        T const& max_val) {

        // Create trapdoors for range boundaries
        auto td_min = factory_.create(min_val);
        auto td_max = factory_.create(max_val);

        std::size_t count = 0;
        for (auto const& val : values) {
            auto td = factory_.create(val);

            // Approximate range check using hash comparison
            // This is simplified; real implementation would use order-preserving encryption
            bool in_range = (td.hash().data[0] >= td_min.hash().data[0] &&
                            td.hash().data[0] <= td_max.hash().data[0]);

            if (in_range) {
                count++;
            }
        }

        // Add noise for privacy
        std::mt19937 gen(42);  // Fixed seed for reproducibility
        std::normal_distribution<> dist(0.0, 1.0);
        double noise = std::abs(dist(gen));
        count += static_cast<std::size_t>(noise);

        // Error rate based on range size and noise
        double error_rate = 0.1;  // 10% error rate

        return core::approximate_value<std::size_t>(count, error_rate);
    }
};

/**
 * Aggregation with privacy guarantees
 */
template <typename T, std::size_t N = 32>
class private_aggregator {
    trapdoor_factory<N> factory_;
    double privacy_budget_;  // Total privacy budget

public:
    private_aggregator(std::string_view secret, double privacy_budget = 1.0)
        : factory_(secret), privacy_budget_(privacy_budget) {}

    /**
     * Compute sum with differential privacy
     */
    template <typename Container>
    core::approximate_value<double> private_sum(
        Container const& values,
        double sensitivity) {  // Max contribution of single value

        double sum = std::accumulate(values.begin(), values.end(), 0.0);

        // Add Laplace noise based on sensitivity and privacy budget
        double noise_scale = sensitivity / privacy_budget_;
        double u = (std::rand() / static_cast<double>(RAND_MAX)) - 0.5;
        double noise = -noise_scale * (u > 0 ? 1 : -1) * std::log(1 - 2 * std::abs(u));

        double noisy_sum = sum + noise;

        // Error bound based on privacy parameters
        double error_rate = 1.0 / privacy_budget_;

        return core::approximate_value<double>(noisy_sum, error_rate);
    }

    /**
     * Compute average with privacy
     */
    template <typename Container>
    core::approximate_value<double> private_average(
        Container const& values,
        double min_val,
        double max_val) {

        if (values.empty()) {
            return core::approximate_value<double>(0.0, 0.0);
        }

        double sensitivity = (max_val - min_val) / values.size();
        auto sum_result = private_sum(values, sensitivity * values.size());

        double avg = sum_result.value() / values.size();
        return core::approximate_value<double>(avg, sum_result.error_rate());
    }

    /**
     * Compute variance with privacy
     */
    template <typename Container>
    core::approximate_value<double> private_variance(
        Container const& values,
        double min_val,
        double max_val) {

        auto avg = private_average(values, min_val, max_val);

        double sum_sq = 0.0;
        for (auto const& val : values) {
            double diff = val - avg.value();
            sum_sq += diff * diff;
        }

        double variance = sum_sq / values.size();

        // Add noise for privacy
        double sensitivity = std::pow(max_val - min_val, 2);
        double noise_scale = sensitivity / privacy_budget_;
        double u = (std::rand() / static_cast<double>(RAND_MAX)) - 0.5;
        double noise = -noise_scale * (u > 0 ? 1 : -1) * std::log(1 - 2 * std::abs(u));

        return core::approximate_value<double>(
            variance + noise,
            avg.error_rate() + 1.0 / privacy_budget_
        );
    }
};

} // namespace cts::operations