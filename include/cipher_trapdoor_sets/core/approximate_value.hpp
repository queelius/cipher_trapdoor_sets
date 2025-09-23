#pragma once

#include <cmath>
#include <concepts>
#include <optional>

namespace cts::core {

/**
 * Represents a value with associated error probability
 * Following the principle of making approximation explicit
 */
template <typename T>
class approximate_value {
    T value_;
    double false_positive_rate_;
    double false_negative_rate_;
    
public:
    constexpr approximate_value(T value, double fpr = 0.0, double fnr = 0.0)
        : value_(std::move(value))
        , false_positive_rate_(fpr)
        , false_negative_rate_(fnr) {}
    
    // Access the value (user acknowledges approximation)
    T const& value() const noexcept { return value_; }
    T& value() noexcept { return value_; }
    
    // Error rates
    double false_positive_rate() const noexcept { return false_positive_rate_; }
    double false_negative_rate() const noexcept { return false_negative_rate_; }
    
    // Total error probability
    double error_rate() const noexcept {
        return std::max(false_positive_rate_, false_negative_rate_);
    }
    
    // Check if this is exact (no approximation)
    bool is_exact() const noexcept {
        return false_positive_rate_ == 0.0 && false_negative_rate_ == 0.0;
    }
    
    // Compose error rates when combining approximate values
    static double compose_error_rates(double e1, double e2) noexcept {
        // Conservative: assume worst case
        // For independent events: 1 - (1-e1)(1-e2)
        return e1 + e2 - e1 * e2;
    }
};

// Specialization for bool with clearer semantics
class approximate_bool : public approximate_value<bool> {
public:
    using approximate_value::approximate_value;
    
    // Logical operations that propagate error
    approximate_bool operator&&(approximate_bool const& other) const {
        bool result = value() && other.value();
        double fpr = compose_error_rates(false_positive_rate(), 
                                        other.false_positive_rate());
        double fnr = compose_error_rates(false_negative_rate(),
                                        other.false_negative_rate());
        return approximate_bool(result, fpr, fnr);
    }
    
    approximate_bool operator||(approximate_bool const& other) const {
        bool result = value() || other.value();
        double fpr = compose_error_rates(false_positive_rate(),
                                        other.false_positive_rate());
        double fnr = compose_error_rates(false_negative_rate(),
                                        other.false_negative_rate());
        return approximate_bool(result, fpr, fnr);
    }
    
    approximate_bool operator!() const {
        return approximate_bool(!value(), false_negative_rate(), false_positive_rate());
    }
};

/**
 * Utility to lift exact values to approximate values
 */
template <typename T>
auto make_exact(T&& value) {
    return approximate_value<std::decay_t<T>>(std::forward<T>(value), 0.0, 0.0);
}

} // namespace cts::core