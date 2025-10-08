#include <cipher_trapdoor_sets/core/approximate_value.hpp>
#include <cassert>
#include <iostream>
#include <cmath>
#include <limits>
#include <vector>
#include <string>

using namespace cts::core;

// Test basic construction and value access
void test_construction() {
    std::cout << "Testing approximate_value construction...";

    // Test exact value (no error)
    {
        approximate_value<int> exact(42);
        assert(exact.value() == 42);
        assert(exact.false_positive_rate() == 0.0);
        assert(exact.false_negative_rate() == 0.0);
        assert(exact.error_rate() == 0.0);
        assert(exact.is_exact());
    }

    // Test with false positive rate only
    {
        approximate_value<double> with_fpr(3.14, 0.1, 0.0);
        assert(with_fpr.value() == 3.14);
        assert(with_fpr.false_positive_rate() == 0.1);
        assert(with_fpr.false_negative_rate() == 0.0);
        assert(with_fpr.error_rate() == 0.1);
        assert(!with_fpr.is_exact());
    }

    // Test with false negative rate only
    {
        approximate_value<std::string> with_fnr("test", 0.0, 0.2);
        assert(with_fnr.value() == "test");
        assert(with_fnr.false_positive_rate() == 0.0);
        assert(with_fnr.false_negative_rate() == 0.2);
        assert(with_fnr.error_rate() == 0.2);
        assert(!with_fnr.is_exact());
    }

    // Test with both error rates
    {
        approximate_value<bool> with_both(true, 0.1, 0.2);
        assert(with_both.value() == true);
        assert(with_both.false_positive_rate() == 0.1);
        assert(with_both.false_negative_rate() == 0.2);
        assert(with_both.error_rate() == 0.2);  // max of the two
        assert(!with_both.is_exact());
    }

    // Test mutable value access
    {
        approximate_value<int> mutable_val(10, 0.05);
        mutable_val.value() = 20;
        assert(mutable_val.value() == 20);
    }

    std::cout << " PASSED\n";
}

// Test approximate_bool specialization
void test_approximate_bool() {
    std::cout << "Testing approximate_bool...";

    // Test basic bool operations
    {
        approximate_bool true_exact(true, 0.0, 0.0);
        approximate_bool false_exact(false, 0.0, 0.0);

        assert(true_exact.value() == true);
        assert(false_exact.value() == false);
        assert(true_exact.is_exact());
        assert(false_exact.is_exact());
    }

    // Test logical AND
    {
        approximate_bool a(true, 0.1, 0.0);
        approximate_bool b(true, 0.2, 0.0);

        auto result = a && b;
        assert(result.value() == true);
        // Error should be composed: 0.1 + 0.2 - 0.1*0.2 = 0.28
        double expected_error = 0.1 + 0.2 - 0.1 * 0.2;
        assert(std::abs(result.false_positive_rate() - expected_error) < 0.001);

        // AND with false
        approximate_bool false_val(false, 0.0, 0.3);
        auto and_false = a && false_val;
        assert(and_false.value() == false);
    }

    // Test logical OR
    {
        approximate_bool a(false, 0.1, 0.2);
        approximate_bool b(true, 0.05, 0.1);

        auto result = a || b;
        assert(result.value() == true);

        // OR with exact false
        approximate_bool false_exact(false, 0.0, 0.0);
        auto or_false = a || false_exact;
        assert(or_false.value() == false);
    }

    // Test logical NOT
    {
        approximate_bool val(true, 0.1, 0.2);
        auto negated = !val;

        assert(negated.value() == false);
        // FPR and FNR should be swapped
        assert(negated.false_positive_rate() == 0.2);
        assert(negated.false_negative_rate() == 0.1);

        // Double negation
        auto double_neg = !(!val);
        assert(double_neg.value() == val.value());
        assert(double_neg.false_positive_rate() == val.false_positive_rate());
        assert(double_neg.false_negative_rate() == val.false_negative_rate());
    }

    std::cout << " PASSED\n";
}

// Test error composition
void test_error_composition() {
    std::cout << "Testing error composition...";

    // Test compose_error_rates static function
    {
        double e1 = 0.1;
        double e2 = 0.2;
        double composed = approximate_value<int>::compose_error_rates(e1, e2);
        double expected = e1 + e2 - e1 * e2;  // 0.28
        assert(std::abs(composed - expected) < 0.001);
    }

    // Test with zero errors
    {
        double composed = approximate_value<int>::compose_error_rates(0.0, 0.0);
        assert(composed == 0.0);

        composed = approximate_value<int>::compose_error_rates(0.5, 0.0);
        assert(composed == 0.5);

        composed = approximate_value<int>::compose_error_rates(0.0, 0.5);
        assert(composed == 0.5);
    }

    // Test with maximum errors
    {
        double composed = approximate_value<int>::compose_error_rates(1.0, 1.0);
        assert(composed == 1.0);  // 1 + 1 - 1*1 = 1

        composed = approximate_value<int>::compose_error_rates(0.9, 0.9);
        double expected = 0.9 + 0.9 - 0.9 * 0.9;  // 0.99
        assert(std::abs(composed - expected) < 0.001);
    }

    // Test chain composition
    {
        approximate_bool a(true, 0.1, 0.0);
        approximate_bool b(true, 0.1, 0.0);
        approximate_bool c(true, 0.1, 0.0);

        auto result = (a && b) && c;
        // Error compounds through operations
        assert(result.error_rate() > 0.2);
    }

    std::cout << " PASSED\n";
}

// Test make_exact utility function
void test_make_exact() {
    std::cout << "Testing make_exact utility...";

    // Test with various types
    {
        auto exact_int = make_exact(42);
        assert(exact_int.value() == 42);
        assert(exact_int.is_exact());

        auto exact_string = make_exact(std::string("hello"));
        assert(exact_string.value() == "hello");
        assert(exact_string.is_exact());

        auto exact_double = make_exact(3.14159);
        assert(exact_double.value() == 3.14159);
        assert(exact_double.is_exact());
    }

    // Test with rvalue
    {
        std::vector<int> vec = {1, 2, 3};
        auto exact_vec = make_exact(std::move(vec));
        assert(exact_vec.value().size() == 3);
        assert(exact_vec.is_exact());
    }

    std::cout << " PASSED\n";
}

// Test edge cases and boundary conditions
void test_edge_cases() {
    std::cout << "Testing edge cases...";

    // Test with extreme error rates
    {
        approximate_value<int> max_error(100, 1.0, 1.0);
        assert(max_error.error_rate() == 1.0);
        assert(!max_error.is_exact());

        approximate_value<int> min_error(100, 0.0, 0.0);
        assert(min_error.error_rate() == 0.0);
        assert(min_error.is_exact());
    }

    // Test with very small error rates
    {
        double tiny_error = std::numeric_limits<double>::min();
        approximate_value<double> tiny(1.0, tiny_error, tiny_error);
        assert(!tiny.is_exact());  // Still not exact
        assert(tiny.error_rate() == tiny_error);
    }

    // Test boolean edge cases
    {
        // All true with errors
        approximate_bool t1(true, 0.5, 0.5);
        approximate_bool t2(true, 0.5, 0.5);
        auto and_result = t1 && t2;
        assert(and_result.value() == true);

        // Mix of true and false
        approximate_bool f1(false, 0.5, 0.5);
        and_result = t1 && f1;
        assert(and_result.value() == false);
    }

    // Test error rate ordering
    {
        approximate_value<int> higher_fpr(1, 0.8, 0.2);
        assert(higher_fpr.error_rate() == 0.8);

        approximate_value<int> higher_fnr(1, 0.3, 0.7);
        assert(higher_fnr.error_rate() == 0.7);

        approximate_value<int> equal_rates(1, 0.5, 0.5);
        assert(equal_rates.error_rate() == 0.5);
    }

    std::cout << " PASSED\n";
}

// Test complex logical expressions
void test_complex_expressions() {
    std::cout << "Testing complex logical expressions...";

    // Test De Morgan's laws with approximate values
    {
        approximate_bool a(true, 0.1, 0.05);
        approximate_bool b(false, 0.05, 0.1);

        // !(a && b) should equal (!a || !b)
        auto left = !(a && b);
        auto right = (!a) || (!b);
        assert(left.value() == right.value());

        // !(a || b) should equal (!a && !b)
        left = !(a || b);
        right = (!a) && (!b);
        assert(left.value() == right.value());
    }

    // Test associativity
    {
        approximate_bool a(true, 0.1, 0.0);
        approximate_bool b(false, 0.0, 0.1);
        approximate_bool c(true, 0.05, 0.05);

        // (a && b) && c == a && (b && c)
        auto left = (a && b) && c;
        auto right = a && (b && c);
        assert(left.value() == right.value());

        // (a || b) || c == a || (b || c)
        left = (a || b) || c;
        right = a || (b || c);
        assert(left.value() == right.value());
    }

    // Test error propagation in chains
    {
        approximate_bool start(true, 0.01, 0.0);
        auto result = start;

        // Chain multiple operations
        for (int i = 0; i < 10; ++i) {
            approximate_bool next(true, 0.01, 0.0);
            result = result && next;
        }

        // Error should have accumulated
        assert(result.error_rate() > 0.05);
        assert(result.value() == true);
    }

    std::cout << " PASSED\n";
}

// Test with custom types
void test_custom_types() {
    std::cout << "Testing with custom types...";

    struct Point {
        double x, y;
        bool operator==(const Point& other) const {
            return x == other.x && y == other.y;
        }
    };

    // Test with custom struct
    {
        Point p{3.14, 2.71};
        approximate_value<Point> approx_point(p, 0.1);
        assert(approx_point.value().x == 3.14);
        assert(approx_point.value().y == 2.71);
        assert(approx_point.false_positive_rate() == 0.1);
    }

    // Test with containers
    {
        std::vector<int> vec = {1, 2, 3, 4, 5};
        approximate_value<std::vector<int>> approx_vec(vec, 0.01, 0.02);
        assert(approx_vec.value().size() == 5);
        assert(approx_vec.value()[2] == 3);
        assert(approx_vec.error_rate() == 0.02);
    }

    std::cout << " PASSED\n";
}

// Test practical usage patterns
void test_practical_usage() {
    std::cout << "Testing practical usage patterns...";

    // Simulate membership test with false positives
    {
        auto membership_test = [](int value) -> approximate_bool {
            // Simulate Bloom filter-like behavior
            bool is_member = (value % 7 == 0);  // Multiples of 7
            double fpr = 0.05;  // 5% false positive rate
            return approximate_bool(is_member, fpr, 0.0);
        };

        auto result1 = membership_test(14);
        assert(result1.value() == true);
        assert(result1.false_positive_rate() == 0.05);

        auto result2 = membership_test(15);
        assert(result2.value() == false);
    }

    // Simulate cardinality estimation
    {
        approximate_value<size_t> estimated_count(1000, 0.1);
        assert(!estimated_count.is_exact());

        // Simulate error bounds
        double actual_error = estimated_count.error_rate();
        size_t lower_bound = static_cast<size_t>(1000 * (1.0 - actual_error));
        size_t upper_bound = static_cast<size_t>(1000 * (1.0 + actual_error));

        assert(lower_bound <= estimated_count.value());
        assert(estimated_count.value() <= upper_bound);
    }

    std::cout << " PASSED\n";
}

int main() {
    std::cout << "\n=== Running Comprehensive Approximate Value Tests ===\n\n";

    test_construction();
    test_approximate_bool();
    test_error_composition();
    test_make_exact();
    test_edge_cases();
    test_complex_expressions();
    test_custom_types();
    test_practical_usage();

    std::cout << "\nAll approximate value tests passed!\n\n";
    return 0;
}