#pragma once

#include <cstddef>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

template <std::size_t N>
class StackStorage {
public:
    StackStorage() noexcept = default;
    StackStorage(const StackStorage&) = delete;
    StackStorage& operator=(const StackStorage&) = delete;

    void* allocate_bytes(std::size_t bytes, std::size_t alignment) {
        void* current = static_cast<void*>(buffer_ + offset_);
        std::size_t space = N - offset_;

        void* aligned = std::align(alignment, bytes, current, space);
        if (aligned == nullptr) {
            throw std::bad_alloc();
        }

        const auto aligned_offset = static_cast<std::size_t>(static_cast<std::byte*>(aligned) - buffer_);
        offset_ = aligned_offset + bytes;
        return aligned;
    }

    [[nodiscard]] std::size_t used() const noexcept {
        return offset_;
    }

    [[nodiscard]] static constexpr std::size_t capacity() noexcept {
        return N;
    }

private:
    alignas(std::max_align_t) std::byte buffer_[N];
    std::size_t offset_ = 0;
};

template <typename T, std::size_t N>
class StackAllocator {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    using propagate_on_container_copy_assignment = std::false_type;
    using propagate_on_container_move_assignment = std::false_type;
    using propagate_on_container_swap = std::false_type;
    using is_always_equal = std::false_type;

    template <typename U>
    struct rebind {
        using other = StackAllocator<U, N>;
    };

    StackAllocator() = delete;

    explicit StackAllocator(StackStorage<N>& storage) noexcept
        : storage_(std::addressof(storage)) {}

    template <typename U>
    StackAllocator(const StackAllocator<U, N>& other) noexcept
        : storage_(other.storage_) {}

    [[nodiscard]] T* allocate(size_type n) {
        if (n > N / sizeof(T)) {
            throw std::bad_alloc();
        }
        return static_cast<T*>(storage_->allocate_bytes(n * sizeof(T), alignof(T)));
    }

    void deallocate(T*, size_type) noexcept {
    
    }

    [[nodiscard]] StackStorage<N>* storage() const noexcept {
        return storage_;
    }

    template <typename U>
    bool operator==(const StackAllocator<U, N>& other) const noexcept {
        return storage_ == other.storage_;
    }

    template <typename U>
    bool operator!=(const StackAllocator<U, N>& other) const noexcept {
        return !(*this == other);
    }

private:
    StackStorage<N>* storage_ = nullptr;

    template <typename, std::size_t>
    friend class StackAllocator;
};

#include "list.h"
