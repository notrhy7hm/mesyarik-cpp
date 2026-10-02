#pragma once

#include <cstddef>
#include <iterator>
#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <limits>

template <typename T>
class Deque {
private:
    static constexpr std::size_t kBlockSize = 64;
    using index_type = std::ptrdiff_t;

    struct Block {
        alignas(T) std::byte storage[sizeof(T) * kBlockSize];

        void* slot(std::size_t i) noexcept {
            return static_cast<void*>(storage + i * sizeof(T));
        }

        const void* slot(std::size_t i) const noexcept {
            return static_cast<const void*>(storage + i * sizeof(T));
        }

        T* ptr(std::size_t i) noexcept {
            return std::launder(reinterpret_cast<T*>(storage + i * sizeof(T)));
        }

        const T* ptr(std::size_t i) const noexcept {
            return std::launder(reinterpret_cast<const T*>(storage + i * sizeof(T)));
        }
    };

public:
    template <bool IsConst>
    class BasicIterator {
        friend class Deque;
        template <bool>
        friend class BasicIterator;

        using owner_type = std::conditional_t<IsConst, const Deque, Deque>;

        owner_type* owner_ = nullptr;
        index_type index_ = 0;

        BasicIterator(owner_type* owner, index_type index) noexcept
            : owner_(owner), index_(index) {}

    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using reference = std::conditional_t<IsConst, const T&, T&>;
        using pointer = std::conditional_t<IsConst, const T*, T*>;

        BasicIterator() noexcept = default;

        template <bool B = IsConst, typename = std::enable_if_t<B>>
        BasicIterator(const BasicIterator<false>& other) noexcept
            : owner_(other.owner_), index_(other.index_) {}

        reference operator*() const {
            return owner_->element_at_abs(index_);
        }

        pointer operator->() const {
            return std::addressof(owner_->element_at_abs(index_));
        }

        reference operator[](difference_type n) const {
            return *(*this + n);
        }

        BasicIterator& operator++() noexcept {
            ++index_;
            return *this;
        }

        BasicIterator operator++(int) noexcept {
            BasicIterator tmp(*this);
            ++(*this);
            return tmp;
        }

        BasicIterator& operator--() noexcept {
            --index_;
            return *this;
        }

        BasicIterator operator--(int) noexcept {
            BasicIterator tmp(*this);
            --(*this);
            return tmp;
        }

        BasicIterator& operator+=(difference_type n) noexcept {
            index_ += n;
            return *this;
        }

        BasicIterator& operator-=(difference_type n) noexcept {
            index_ -= n;
            return *this;
        }

        BasicIterator operator+(difference_type n) const noexcept {
            BasicIterator tmp(*this);
            tmp += n;
            return tmp;
        }

        BasicIterator operator-(difference_type n) const noexcept {
            BasicIterator tmp(*this);
            tmp -= n;
            return tmp;
        }

        friend BasicIterator operator+(difference_type n, BasicIterator it) noexcept {
            it += n;
            return it;
        }

        difference_type operator-(const BasicIterator& other) const noexcept {
            return index_ - other.index_;
        }

        bool operator==(const BasicIterator& other) const noexcept {
            return owner_ == other.owner_ && index_ == other.index_;
        }

        bool operator!=(const BasicIterator& other) const noexcept {
            return !(*this == other);
        }

        bool operator<(const BasicIterator& other) const noexcept {
            return index_ < other.index_;
        }

        bool operator<=(const BasicIterator& other) const noexcept {
            return index_ <= other.index_;
        }

        bool operator>(const BasicIterator& other) const noexcept {
            return index_ > other.index_;
        }

        bool operator>=(const BasicIterator& other) const noexcept {
            return index_ >= other.index_;
        }
    };

    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;

    using iterator = BasicIterator<false>;
    using const_iterator = BasicIterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    Deque() {
        initialize_map();
    }

    explicit Deque(size_type count) : Deque() {
        try {
            for (size_type i = 0; i < count; ++i) {
                emplace_back();
            }
        } catch (...) {
            clear();
            throw;
        }
    }

    Deque(size_type count, const T& value) : Deque() {
        try {
            for (size_type i = 0; i < count; ++i) {
                push_back(value);
            }
        } catch (...) {
            clear();
            throw;
        }
    }

    Deque(const Deque& other) : Deque() {
        try {
            for (const auto& value : other) {
                push_back(value);
            }
        } catch (...) {
            clear();
            throw;
        }
    }

    Deque(Deque&& other) noexcept
        : first_(other.first_),
          size_(other.size_),
          map_(std::move(other.map_)),
          map_capacity_(other.map_capacity_),
          map_base_block_(other.map_base_block_) {
        other.first_ = 0;
        other.size_ = 0;
        other.map_capacity_ = 0;
        other.map_base_block_ = 0;
    }

    ~Deque() {
        clear();
    }

    Deque& operator=(const Deque& other) {
        if (this != &other) {
            Deque tmp(other);
            swap(tmp);
        }
        return *this;
    }

    Deque& operator=(Deque&& other) noexcept {
        if (this != &other) {
            clear();
            first_ = other.first_;
            size_ = other.size_;
            map_ = std::move(other.map_);
            map_capacity_ = other.map_capacity_;
            map_base_block_ = other.map_base_block_;

            other.first_ = 0;
            other.size_ = 0;
            other.map_capacity_ = 0;
            other.map_base_block_ = 0;
        }
        return *this;
    }

    void swap(Deque& other) noexcept {
        using std::swap;
        swap(first_, other.first_);
        swap(size_, other.size_);
        swap(map_, other.map_);
        swap(map_capacity_, other.map_capacity_);
        swap(map_base_block_, other.map_base_block_);
    }

    friend void swap(Deque& lhs, Deque& rhs) noexcept {
        lhs.swap(rhs);
    }

    [[nodiscard]] bool empty() const noexcept {
        return size_ == 0;
    }

    [[nodiscard]] size_type size() const noexcept {
        return size_;
    }

    [[nodiscard]] size_type max_size() const noexcept {
        return static_cast<size_type>(std::numeric_limits<difference_type>::max());
    }

    reference operator[](size_type pos) {
        return element_at_abs(first_ + static_cast<index_type>(pos));
    }

    const_reference operator[](size_type pos) const {
        return element_at_abs(first_ + static_cast<index_type>(pos));
    }

    reference at(size_type pos) {
        if (pos >= size_) {
            throw std::out_of_range("Deque::at: index out of range");
        }
        return (*this)[pos];
    }

    const_reference at(size_type pos) const {
        if (pos >= size_) {
            throw std::out_of_range("Deque::at: index out of range");
        }
        return (*this)[pos];
    }

    reference front() {
        return (*this)[0];
    }

    const_reference front() const {
        return (*this)[0];
    }

    reference back() {
        return (*this)[size_ - 1];
    }

    const_reference back() const {
        return (*this)[size_ - 1];
    }

    iterator begin() noexcept {
        return iterator(this, first_);
    }

    const_iterator begin() const noexcept {
        return const_iterator(this, first_);
    }

    const_iterator cbegin() const noexcept {
        return begin();
    }

    iterator end() noexcept {
        return iterator(this, first_ + static_cast<index_type>(size_));
    }

    const_iterator end() const noexcept {
        return const_iterator(this, first_ + static_cast<index_type>(size_));
    }

    const_iterator cend() const noexcept {
        return end();
    }

    reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(cend());
    }

    reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }

    const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(cbegin());
    }

    template <typename... Args>
    reference emplace_back(Args&&... args) {
        if (!map_) {
            initialize_map();
        }

        const index_type abs_index = first_ + static_cast<index_type>(size_);
        bool created_block = false;
        Block* block = ensure_block(abs_index, created_block);
        const auto [block_id, offset] = split_index(abs_index);
        (void)block_id;

        try {
            ::new (block->slot(offset)) T(std::forward<Args>(args)...);
        } catch (...) {
            if (created_block && !block_is_live(block_id)) {
                release_block(block_id);
            }
            throw;
        }

        ++size_;
        return *block->ptr(offset);
    }

    template <typename... Args>
    reference emplace_front(Args&&... args) {
        if (!map_) {
            initialize_map();
        }

        const index_type abs_index = empty() ? first_ : first_ - 1;
        bool created_block = false;
        Block* block = ensure_block(abs_index, created_block);
        const auto [block_id, offset] = split_index(abs_index);

        try {
            ::new (block->slot(offset)) T(std::forward<Args>(args)...);
        } catch (...) {
            if (created_block && !block_is_live(block_id)) {
                release_block(block_id);
            }
            throw;
        }

        first_ = abs_index;
        ++size_;
        return *block->ptr(offset);
    }

    void push_back(const T& value) {
        emplace_back(value);
    }
    
    void push_back(const T&& value) {
        emplace_back(std::move(value));
    }

    void push_front(const T& value) {
        emplace_front(value);
    }

    void push_front(const T&& value) {
        emplace_front(std::move(value));
    }

    void pop_back() {
        const index_type abs_index = first_ + static_cast<index_type>(size_) - 1;
        const auto [block_id, offset] = split_index(abs_index);
        std::destroy_at(block_ptr(block_id)->ptr(offset));
        --size_;
        release_block_if_unused(block_id);
        normalize_empty():
    }

    void pop_front() {
        const index_type abs_index = first_;
        const auto [block_id, offset] = split_index(abs_index);
        std::destroy_at(block_ptr(block_id)->ptr(offset));
        ++first_;
        --size_;
        release_block_if_unused(block_id);
        normalize_empty();
    }   

    iterator insert(const_iterator pos, const T& value) {
        const size_type logical = checked_position(pos, true);

        if (logical == 0) {
            push_front(value);
            return begin();
        }

        if (logical == size_) {
            push_back(value);
            return end() - 1;
        }

        Deque tmp(*this);
        tmp.insert_middle_inplace(logical, value);
        swap(tmp);
        return begin() + static_cast<difference_type>(logical);
    }

    iterator insert(const_iterator pos, T&& value) {
        const size_type logical = checked_position(pos, true);

        if (logical == 0) {
            push_front(std::move(value));
            return begin();
        }

        if (logical == size_) {
            push_back(std::move(value));
            return end() - 1;
        }

        T saved(std::move(value));
        emplace_back(std::move(back()));
        for (size_type i = size_ - 2; i  > logical; --i) {
            (*this)[i] = std::move((*this)[i-1]);
        }
        (*this)[logical] = std::move(saved);
        return begin() + static_cast<difference_type>(logical);
    }

    template <typename... Args>
    iterator emplace (const_iterator pos, Args&&... args) {
        T value(std::forward<Args>(args)...);
        return insert(pos, std::move(value));
    }   

    iterator erase(const_iterator pos) {
        const size_type logical = checked_position(pos, false);

        if (logical == 0) {
            pop_front();
            return begin();
        }

        if (logical + 1 == size_) {
            pop_back();
            return end();
        }

        for (size_type i = logical; i + 1 < size_; ++i) {
            (*this)[i] = (*this)[i+1];
        }
        pop_back();
        return begin() + static_cast<difference_type>(logical);
    }

    iterator erase(const_iterator first, const_iterator last) {
        const size_type from = checked_position(first, true);
        const size_type to = checked_position(last, true);
        if (to < from) {
            throw std::out_of_range("Deque::erase: invalid range");
        }

        const size_type count = to - from;
        if (count == 0) {
            return begin() + static_cast<difference_type>(from);
        }

        for (size_type i = from; i + count < size_; ++i) {
            (*this)[i] = (*this)[i + count];
        }
        for (size_type i = 0; i < count; ++i) {
            pop_back();
        }
        return begin() + static_cast<difference_type>(from);
    }

    void clear() noexcept(std::is_nothrow_destructible_v<T>) {
        for (size_type i = 0; i < size_; ++i) {
            std::destroy_at(std::addressof((*this)[i]));
        }
        size_ = 0;
        first_ = 0;
        release_all_blocks();
    }

    void resize(size_type count) {
        if (count < size_) {
            while(size_ > count) {
                pop_back();
            }
            return;
        }
    

        const size_type old_size = size_;
        try {
            while (size_ < count) {
                emplace_back();
            }
        } catch(...) {
            while(size_ > old_size) {
                pop_back();
            }
            throw;
        }
    }

    void resize(size_type count, const T& value) {
        if (count < size_) {
            while(size_ > count) {
                pop_back();
            }
            return;
        }

        const size_type old_size = size_;
        try {
            while (size_ < count) {
                push_back(value);
            }
        } catch (...) {
            while (size_ > old_size) {
                pop_back();
            }
            throw;
        }
    }

private:
    index_type first_ = 0;
    size_type size_ = 0;

    std::unique_ptr<Block*[]> map_;
    size_type map_capacity_ = 0;
    index_type map_base_block_ = 0;

    static std::pair<index_type, std::size_t> split_index(index_type index) noexcept {
        index_type block = index / static_cast<index_type>(kBlockSize);
        index_type offset = index % static_cast<index_type>(kBlockSize);
        if (offset < 0) {
            --block;
            offset += static_cast<index_type>(kBlockSize);
        }
        return {block, static_cast<std::size_t>(offset)};
    }

    void initialize_map() {
        map_capacity_ = 8;
        map_base_block_ = -static_cast<index_type>(map_capacity_ / 2);
        map_ = std::make_unique<Block*[]>(map_capacity_);
    }

    bool block_in_map(index_type block_id) const noexcept {
        return map_ && block_id >= map_base_block_
            && block_id < map_base_block_ + static_cast<index_type>(map_capacity_);
    }

    void ensure_map_contains(index_type block_id) {
        if (!map_) {
            initialize_map();
        }
        if (block_in_map(block_id)) {
            return;
        }

        index_type old_left = map_base_block_;
        index_type old_right = map_base_block_ + static_cast<index_type>(map_capacity_) - 1;
        index_type needed_left = block_id < old_left ? block_id : old_left;
        index_type needed_right = block_id > old_right ? block_id : old_right;

        size_type new_capacity = map_capacity_ == 0 ? 8 : map_capacity_ * 2;
        const auto needed_width = static_cast<size_type>(needed_right - needed_left + 1);
        while (new_capacity < needed_width) {
            new_capacity *= 2;
        }

        const index_type spare = static_cast<index_type>(new_capacity - needed_width);
        const index_type new_base = needed_left - spare/2;


        auto new_map = std::make_unique<Block*[]>(new_capacity);
        for (size_type i = 0; i < map_capacity_; ++i) {
            if (map_[i] != nullptr) {
                const index_type old_block = map_base_block_ + static_cast<index_type>(i);
                const size_type new_index = static_cast<size_type>(old_block - new_base);
                new_map[new_index] = map_[i];
            }
        }

        map_ = std::move(new_map);
        map_capacity_ = new_capacity;
        map_base_block_ = new_base;
    }

    Block*& block_slot(index_type block_id) {
        ensure_map_contains(block_id);
        return map_[static_cast<size_type>(block_id - map_base_block_)];
    }

    Block* block_ptr(index_type block_id) noexcept {
        if (!block_in_map(block_id)) {
            return nullptr;
        }
        return map_[static_cast<size_type>(block_id - map_base_block_)];
    }

    const Block* block_ptr(index_type block_id) const noexcept {
        if (!block_in_map(block_id)) {
            return nullptr;
        }
        return map_[static_cast<size_type>(block_id - map_base_block_)];
    }

    Block* ensure_block(index_type abs_index, bool& created) {
        const auto [block_id, offset] = split_index(abs_index);
        (void)offset;
        Block*& slot = block_slot(block_id);
        if (slot == nullptr) {
            slot = new Block();
            created = true;
        } else {
            created = false;
        }
        return slot;
    }

    reference element_at_abs(index_type abs_index) {
        const auto [block_id, offset] = split_index(abs_index);
        return *block_ptr(block_id)->ptr(offset);
    }

    const_reference element_at_abs(index_type abs_index) const {
        const auto [block_id, offset] = split_index(abs_index);
        return *block_ptr(block_id)->ptr(offset);
    }

    bool block_is_live(index_type block_id) const noexcept {
        if (size_ == 0) {
            return false;
        }

        const auto [first_block, first_offset] = split_index(first_);
        const auto [last_block, last_offset] = split_index(
                first_ + static_cast<index_type>(size_) - 1);
        (void)first_offset;
        (void)last_offset;
        return block_id >= first_block && block_id <= last_block;
    }

    void release_block(index_type block_id) noexcept {
        if (!block_in_map(block_id)) {
            return;
        }
        Block*& slot = map_[static_cast<size_type>(block_id - map_base_block_)];
        delete slot;
        slot = nullptr;
    }

    void release_block_if_unused(index_type block_id) noexcept {
        if (!block_is_live(block_id)) {
            release_block(block_id);
        }
    }

    void release_all_blocks() noexcept {
        if (!map_) {
            return;
        }
        for (size_type i = 0; i < map_capacity_; ++i) {
            delete map_[i];
            map_[i] = nullptr;
        }
    }

    void normalize_empty() noexcept {
        if (size_ == 0) {
            first_ = 0;
        }
    }

    size_type checked_position(const_iterator pos, bool allow_end) const {
        if (pos.owner_ != this) {
            throw std::out_of_range("Deque: iterator belongs to another container");    
        }

        const index_type end_index = first_ + static_cast<index_type>(size_);
        const bool ok = allow_end
            ? (pos.index_ >= first_ && pos.index_ <= end_index)
            : (pos.index_ >= first_ && pos.index_ < end_index);
        if (!ok) {
            throw std::out_of_range("Deque: invalid iterator position");
        }
        return static_cast<size_type>(pos.index_ - first_);
    }

    void insert_middle_inplace(size_type logical, const T& value) {
        T saved(value);
        push_back(back());

        for (size_type i = size_ - 2; i > logical; --i) {
            (*this)[i] = (*this)[i - 1];
        }
        (*this)[logical] = saved;
    }
};
