// BGL Modern - C++20 Generator Coroutine
// Lazy sequence generation for graph traversals
//
// Copyright 2024 Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BGL_MODERN_GENERATOR_HPP
#define BGL_MODERN_GENERATOR_HPP

#include <bgl/modern/version.hpp>

#include <coroutine>
#include <iterator>
#include <utility>
#include <memory>
#include <exception>
#include <type_traits>

namespace bgl {

// =============================================================================
// generator<T> - C++20 Coroutine Generator
// =============================================================================

/// A lazy generator that yields values on-demand using C++20 coroutines.
///
/// This is a simplified implementation suitable for graph traversals.
/// For C++23, prefer std::generator when available.
///
/// Example:
/// @code
///     generator<int> count_to(int n) {
///         for (int i = 0; i < n; ++i) {
///             co_yield i;
///         }
///     }
///     
///     for (int x : count_to(5)) {
///         std::cout << x << " ";  // 0 1 2 3 4
///     }
/// @endcode
///
template<typename T>
class generator {
public:
    // -------------------------------------------------------------------------
    // Promise Type
    // -------------------------------------------------------------------------
    
    struct promise_type {
        T current_value;
        std::exception_ptr exception;
        
        generator get_return_object() {
            return generator{handle_type::from_promise(*this)};
        }
        
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        
        std::suspend_always yield_value(T value) noexcept(std::is_nothrow_move_constructible_v<T>) {
            current_value = std::move(value);
            return {};
        }
        
        // Disallow co_await in generator
        void await_transform() = delete;
        
        void return_void() noexcept {}
        
        void unhandled_exception() {
            exception = std::current_exception();
        }
        
        void rethrow_if_exception() {
            if (exception) {
                std::rethrow_exception(exception);
            }
        }
    };
    
    using handle_type = std::coroutine_handle<promise_type>;
    
    // -------------------------------------------------------------------------
    // Iterator
    // -------------------------------------------------------------------------
    
    class iterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using reference = T&;
        using pointer = T*;
        
        iterator() noexcept = default;
        
        explicit iterator(handle_type handle) noexcept 
            : handle_(handle) {}
        
        iterator& operator++() {
            handle_.resume();
            if (handle_.done()) {
                handle_.promise().rethrow_if_exception();
            }
            return *this;
        }
        
        void operator++(int) {
            ++*this;
        }
        
        [[nodiscard]] reference operator*() const noexcept {
            return handle_.promise().current_value;
        }
        
        [[nodiscard]] pointer operator->() const noexcept {
            return std::addressof(handle_.promise().current_value);
        }
        
        [[nodiscard]] bool operator==(std::default_sentinel_t) const noexcept {
            return !handle_ || handle_.done();
        }
        
    private:
        handle_type handle_;
    };
    
    // -------------------------------------------------------------------------
    // Constructors / Destructor
    // -------------------------------------------------------------------------
    
    generator() noexcept = default;
    
    explicit generator(handle_type handle) noexcept 
        : handle_(handle) {}
    
    generator(generator&& other) noexcept 
        : handle_(std::exchange(other.handle_, nullptr)) {}
    
    generator& operator=(generator&& other) noexcept {
        if (this != &other) {
            if (handle_) {
                handle_.destroy();
            }
            handle_ = std::exchange(other.handle_, nullptr);
        }
        return *this;
    }
    
    generator(const generator&) = delete;
    generator& operator=(const generator&) = delete;
    
    ~generator() {
        if (handle_) {
            handle_.destroy();
        }
    }
    
    // -------------------------------------------------------------------------
    // Range Interface
    // -------------------------------------------------------------------------
    
    [[nodiscard]] iterator begin() {
        if (handle_) {
            handle_.resume();
            if (handle_.done()) {
                handle_.promise().rethrow_if_exception();
            }
        }
        return iterator{handle_};
    }
    
    [[nodiscard]] std::default_sentinel_t end() const noexcept {
        return std::default_sentinel;
    }
    
    // -------------------------------------------------------------------------
    // Queries
    // -------------------------------------------------------------------------
    
    [[nodiscard]] explicit operator bool() const noexcept {
        return handle_ && !handle_.done();
    }
    
private:
    handle_type handle_;
};

// =============================================================================
// recursive_generator<T> - For recursive graph traversals
// =============================================================================

/// A generator that supports yielding from nested generators.
///
/// Useful for recursive algorithms like DFS that naturally want to
/// yield from recursive calls.
///
/// Example:
/// @code
///     recursive_generator<int> dfs(const Graph& g, Vertex v, std::set<Vertex>& visited) {
///         if (visited.count(v)) co_return;
///         visited.insert(v);
///         co_yield v;
///         for (auto u : adjacent_vertices(v, g)) {
///             co_yield dfs(g, u, visited);  // Yield from nested generator
///         }
///     }
/// @endcode
///
template<typename T>
class recursive_generator {
public:
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;
    
    struct promise_type {
        T current_value;
        std::exception_ptr exception;
        std::coroutine_handle<> inner_handle;
        std::coroutine_handle<> root_or_parent;
        
        recursive_generator get_return_object() {
            return recursive_generator{handle_type::from_promise(*this)};
        }
        
        std::suspend_always initial_suspend() noexcept { return {}; }
        
        struct final_awaiter {
            bool await_ready() noexcept { return false; }
            
            std::coroutine_handle<> await_suspend(handle_type h) noexcept {
                auto& promise = h.promise();
                if (promise.root_or_parent) {
                    return promise.root_or_parent;
                }
                return std::noop_coroutine();
            }
            
            void await_resume() noexcept {}
        };
        
        final_awaiter final_suspend() noexcept { return {}; }
        
        std::suspend_always yield_value(T value) {
            current_value = std::move(value);
            return {};
        }
        
        // Yield from nested generator
        struct nested_awaiter {
            recursive_generator nested;
            std::exception_ptr exception;
            
            bool await_ready() noexcept {
                return !nested.handle_;
            }
            
            std::coroutine_handle<> await_suspend(handle_type h) noexcept {
                nested.handle_.promise().root_or_parent = h;
                return nested.handle_;
            }
            
            void await_resume() {
                if (exception) {
                    std::rethrow_exception(exception);
                }
            }
        };
        
        nested_awaiter yield_value(recursive_generator&& nested) noexcept {
            return nested_awaiter{std::move(nested)};
        }
        
        void await_transform() = delete;
        void return_void() noexcept {}
        
        void unhandled_exception() {
            exception = std::current_exception();
        }
        
        T& value() noexcept {
            return current_value;
        }
    };
    
    class iterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using reference = T&;
        using pointer = T*;
        
        iterator() noexcept = default;
        explicit iterator(handle_type handle) noexcept : handle_(handle) {}
        
        iterator& operator++() {
            // Find the innermost active handle
            auto h = handle_;
            while (h) {
                h.resume();
                if (!h.done()) break;
                // Move up to parent
                h = h.promise().root_or_parent ? 
                    handle_type::from_address(h.promise().root_or_parent.address()) : 
                    handle_type{};
            }
            return *this;
        }
        
        void operator++(int) { ++*this; }
        
        reference operator*() const noexcept {
            return handle_.promise().current_value;
        }
        
        pointer operator->() const noexcept {
            return std::addressof(handle_.promise().current_value);
        }
        
        bool operator==(std::default_sentinel_t) const noexcept {
            return !handle_ || handle_.done();
        }
        
    private:
        handle_type handle_;
    };
    
    recursive_generator() noexcept = default;
    explicit recursive_generator(handle_type h) noexcept : handle_(h) {}
    
    recursive_generator(recursive_generator&& other) noexcept 
        : handle_(std::exchange(other.handle_, nullptr)) {}
    
    recursive_generator& operator=(recursive_generator&& other) noexcept {
        if (this != &other) {
            if (handle_) handle_.destroy();
            handle_ = std::exchange(other.handle_, nullptr);
        }
        return *this;
    }
    
    recursive_generator(const recursive_generator&) = delete;
    recursive_generator& operator=(const recursive_generator&) = delete;
    
    ~recursive_generator() {
        if (handle_) handle_.destroy();
    }
    
    iterator begin() {
        if (handle_) {
            handle_.resume();
        }
        return iterator{handle_};
    }
    
    std::default_sentinel_t end() const noexcept {
        return std::default_sentinel;
    }
    
private:
    handle_type handle_;
};

} // namespace bgl

#endif // BGL_MODERN_GENERATOR_HPP
