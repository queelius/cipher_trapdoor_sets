#pragma once

#include "../trapdoor.hpp"
#include "../sets/symmetric_difference_set.hpp"
#include "../sets/boolean_set.hpp"
#include <vector>
#include <algorithm>
#include <numeric>

namespace cts::operations {

/**
 * Batch operations for efficient processing of multiple trapdoors
 * Leverages parallel execution when available
 */
class batch_operations {
public:
    // Batch create trapdoors from collection
    template <typename Container, std::size_t N = 32>
    static auto batch_create_trapdoors(Container const& values, 
                                      std::string_view secret) {
        using T = typename Container::value_type;
        trapdoor_factory<N> factory(secret);
        
        std::vector<trapdoor<T, N>> results;
        results.reserve(values.size());
        
        std::transform(values.begin(), values.end(),
                      std::back_inserter(results),
                      [&factory](auto const& v) { return factory.create(v); });
        
        return results;
    }
    
    // Batch equality testing
    template <typename T, std::size_t N>
    static std::vector<core::approximate_bool> batch_equals(
        std::vector<trapdoor<T, N>> const& left,
        std::vector<trapdoor<T, N>> const& right) {
        
        if (left.size() != right.size()) {
            throw std::invalid_argument("Size mismatch in batch comparison");
        }
        
        std::vector<core::approximate_bool> results;
        results.reserve(left.size());
        
        std::transform(left.begin(), left.end(), right.begin(),
                      std::back_inserter(results),
                      [](auto const& l, auto const& r) { return l.equals(r); });
        
        return results;
    }
    
    // Batch union of symmetric difference sets
    template <typename T, std::size_t N>
    static sets::symmetric_difference_set<T, N> batch_union(
        std::vector<sets::symmetric_difference_set<T, N>> const& sets) {
        
        if (sets.empty()) {
            return sets::symmetric_difference_set<T, N>();
        }
        
        return std::reduce(sets.begin() + 1, sets.end(),
                         sets[0],
                         [](auto const& a, auto const& b) { return a ^ b; });
    }
    
    // Batch intersection of boolean sets
    template <typename T, std::size_t N>
    static sets::boolean_set<T, N> batch_intersection(
        std::vector<sets::boolean_set<T, N>> const& sets) {
        
        if (sets.empty()) {
            return sets::boolean_set<T, N>();
        }
        
        return std::reduce(sets.begin() + 1, sets.end(),
                         sets[0],
                         [](auto const& a, auto const& b) { return a & b; });
    }
    
    // Batch membership testing
    template <typename T, std::size_t N>
    static std::vector<core::approximate_bool> batch_contains(
        sets::boolean_set<T, N> const& set,
        std::vector<trapdoor<T, N>> const& elements) {
        
        std::vector<core::approximate_bool> results;
        results.reserve(elements.size());
        
        std::transform(elements.begin(), elements.end(),
                      std::back_inserter(results),
                      [&set](auto const& elem) { return set.contains(elem); });
        
        return results;
    }
};

/**
 * Pipeline operations for composing transformations
 */
template <typename T, std::size_t N = 32>
class pipeline {
    trapdoor_factory<N> factory_;
    
public:
    explicit pipeline(std::string_view secret) : factory_(secret) {}
    
    // Transform and create trapdoors
    template <typename Container, typename Fn>
    auto transform_create(Container const& values, Fn transform) {
        using U = std::invoke_result_t<Fn, typename Container::value_type>;
        std::vector<trapdoor<U, N>> results;
        results.reserve(values.size());
        
        for (auto const& v : values) {
            auto transformed = transform(v);
            results.push_back(factory_.create(transformed));
        }
        
        return results;
    }
    
    // Filter and create set
    template <typename Container, typename Predicate>
    sets::symmetric_difference_set<typename Container::value_type, N>
    filter_to_set(Container const& values, Predicate pred) {
        using ValueType = typename Container::value_type;
        sets::symmetric_difference_set<ValueType, N> result;
        
        for (auto const& v : values) {
            if (pred(v)) {
                auto td = factory_.create(v);
                result ^= td;
            }
        }
        
        return result;
    }
};

} // namespace cts::operations