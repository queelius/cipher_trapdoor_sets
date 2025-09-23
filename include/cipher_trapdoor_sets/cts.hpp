#pragma once

/**
 * Cipher Trapdoor Sets (CTS) - Main Header
 *
 * A modern C++ library for privacy-preserving set operations using
 * cryptographic trapdoor functions.
 *
 * Core Concepts:
 * - Trapdoor: One-way hash transformation preserving privacy
 * - Approximate operations: Explicit error rates for all comparisons
 * - Composable design: Simple components that combine powerfully
 *
 * Usage:
 *   #include <cipher_trapdoor_sets/cts.hpp>
 *   using namespace cts;
 *
 * See examples/ directory for comprehensive usage examples.
 */

// Core types
#include "core/hash_types.hpp"
#include "core/approximate_value.hpp"

// Main trapdoor type
#include "trapdoor.hpp"

// Set implementations
#include "sets/symmetric_difference_set.hpp"
#include "sets/boolean_set.hpp"

// Operations
#include "operations/batch_ops.hpp"
#include "operations/cardinality.hpp"
#include "operations/homomorphic.hpp"
#include "operations/analytics.hpp"
#include "operations/similarity.hpp"

// Key management
#include "key_management.hpp"

// Serialization
#include "serialization/binary_format.hpp"

namespace cts {

// Version information
constexpr char const* version = "2.0.0";
constexpr int version_major = 2;
constexpr int version_minor = 0;
constexpr int version_patch = 0;

// Default hash size (256 bits)
constexpr std::size_t default_hash_size = 32;

// Convenience aliases
template <typename T>
using default_trapdoor = trapdoor<T, default_hash_size>;

template <typename T>
using default_sym_diff_set = sets::symmetric_difference_set<T, default_hash_size>;

template <typename T>
using default_boolean_set = sets::boolean_set<T, default_hash_size>;

} // namespace cts