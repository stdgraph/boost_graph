// BGL Modern - C++20 Feature Verification Test
// Verifies that concepts and ranges work correctly

#include <bgl/modern/version.hpp>

#include <concepts>
#include <ranges>
#include <vector>
#include <type_traits>
#include <functional>
#include <iostream>

// Test C++20 concepts
template<typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
};

static_assert(Addable<int>);
static_assert(Addable<double>);

// Test C++20 ranges
static_assert(std::ranges::range<std::vector<int>>);
static_assert(std::ranges::forward_range<std::vector<int>>);

// Test range views
void test_ranges() {
    std::vector<int> v = {1, 2, 3, 4, 5};
    
    // Test filter view
    auto filtered = v | std::views::filter([](int x) { return x > 2; });
    int count = 0;
    for (auto x : filtered) {
        (void)x;
        ++count;
    }
    if (count != 3) {
        std::cerr << "Filter view failed: expected 3, got " << count << std::endl;
        std::exit(1);
    }
    
    // Test transform view
    auto transformed = v | std::views::transform([](int x) { return x * 2; });
    int sum = 0;
    for (auto x : transformed) {
        sum += x;
    }
    if (sum != 30) {
        std::cerr << "Transform view failed: expected 30, got " << sum << std::endl;
        std::exit(1);
    }
}

// Test subrange
void test_subrange() {
    std::vector<int> v = {1, 2, 3, 4, 5};
    auto sub = std::ranges::subrange(v.begin() + 1, v.end() - 1);
    
    int count = 0;
    for (auto x : sub) {
        (void)x;
        ++count;
    }
    if (count != 3) {
        std::cerr << "Subrange failed: expected 3, got " << count << std::endl;
        std::exit(1);
    }
}

// Test requires clause
template<typename T>
    requires std::integral<T>
T double_value(T x) {
    return x * 2;
}

void test_requires_clause() {
    auto result = double_value(21);
    if (result != 42) {
        std::cerr << "Requires clause test failed" << std::endl;
        std::exit(1);
    }
}

// Test std::invocable concept
template<std::invocable<int> F>
int apply_to_10(F&& f) {
    return std::invoke(std::forward<F>(f), 10);
}

void test_invocable() {
    auto result = apply_to_10([](int x) { return x * 3; });
    if (result != 30) {
        std::cerr << "Invocable test failed" << std::endl;
        std::exit(1);
    }
}

int main() {
    std::cout << "BGL Modern Version: " 
              << bgl::modern::version_major << "."
              << bgl::modern::version_minor << "."
              << bgl::modern::version_patch << std::endl;
    
    std::cout << "Testing C++20 features..." << std::endl;
    
    test_ranges();
    std::cout << "  [PASS] Ranges (filter, transform)" << std::endl;
    
    test_subrange();
    std::cout << "  [PASS] std::ranges::subrange" << std::endl;
    
    test_requires_clause();
    std::cout << "  [PASS] requires clause" << std::endl;
    
    test_invocable();
    std::cout << "  [PASS] std::invocable concept" << std::endl;
    
    std::cout << "\nAll C++20 feature tests passed!" << std::endl;
    return 0;
}
