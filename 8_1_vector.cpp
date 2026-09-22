#include <iostream>
#include <iterator>
#include <type_traits>
#include <cstdint>
#include <cstddef>
#include <new>


template <typename T, typename Alloc = std::allocator<T>>
class vector {
    T* arr_;
    size_t sz_;
    size_t cap_;
    Alloc alloc_;

    using AllocTraits = std::allocator_traits<Alloc>;
private:
    template <bool IsConst>
    class base_iterator {
    public:
        using pointer_type = std::conditional_t<IsConst, const T*, T*>;
        using reference_type = std::conditional_t<IsConst, const T&, T&>;
        using value_type = T;
    private:
        pointer_type ptr;
        base_iterator(T* ptr): ptr(ptr) {}
        friend class vector<T>;
        template <bool B>
        friend class base_iterator;
    public:
        base_iterator(const base_iterator&) = default;
        base_iterator& operator=(const base_iterator&) = default;
        reference_type operator*() const { return *ptr; }
        pointer_type operator->() const { return ptr; }
        base_iterator& operator++() {
            ++ptr;
            return *this;
        }
        base_iterator operator++(int) {
            base_iterator copy = *this;
            ++ptr;
            return copy;
        }
        operator base_iterator<true>() const {
            return {ptr};
        }
    };
public:
    using iterator = base_iterator<false>;
    using const_iterator = base_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    vector& operator=(const vector& other) {i
        Alloc newalloc = AllocTraits::propogate_on_container_copy_assignment::value
            ? other.alloc_ : alloc_;
        
        T* newarr = AllocTraits::allocate(newalloc, other.cap_);
        size_t i = 0;
        try {
            for (; i < other.sz_; ++i) {
                AllocTraits::construct(newalloc, newarr + i, other[i]);
            }
        } catch (...) {
            for (size_t j = 0; j < i; ++j) {
                AllocTraits::destroy(newalloc, newarr + j);
            }
            AllocTraits::deallocate(newalloc, newarr, other.cap_);
            throw;
        }

        for (size_t i = 0; i < sz_; ++i) {
            AllocTraits::destroy(alloc_, arr_ + i);
        }
        AllocTraits::deallocate(alloc_, arr_, cap_);
        
        alloc_ = newalloc;
        arr_ = newarr;
        sz_ = other.sz_;
        cap_ = other.cap_; 
    }

    iterator begin() { return {arr_}; }
    iterator end() { return {arr_ + sz_}; }
    const_iterator begin() const { return {arr_}; }
    const_iterator end() const { return {arr_ + sz_}; }
    const_iterator cbegin() const { return {arr_}; }
    const_iterator cend() const { return {arr_ + sz_}; }

    void reserve(size_t newcap) {
        if (newcap <= cap_) {
            return;
        }
        T* newarr = AllocTraits::allocate(alloc_, newcap);

        size_t index = 0;
        try {
            for (; index < sz_; ++index) {
                AllocTraits::construct(alloc_, newarr+index, 
                        std::move_if_noexcept(arr_[index]));
            }
        } catch (...) {
            for (size_t oldindex = 0; oldindex < index; ++oldindex) {
                AllocTraits::destroy(alloc_, newarr+oldindex);    
            }
            AllocTraits::deallocate(alloc_, newarr, newcap);
            throw;
        }
        for (size_t i = 0; i < sz_; ++i) {
            AllocTraits::destroy(alloc_, arr_+index);
        }
        AllocTraits::deallocate(alloc_, arr_, cap_);
        arr_ = newarr;
        cap_ = newcap;
    }

    void push_back(const T& value) {
        emplace_back(value);
    }
    void push_back(T&& value) {
        emplace_back(std::move(value));
    }

    void emplace_back(auto&&... args) {
        if (sz_ == cap_) {
            reserve(cap_ > 0 ? cap_ * 2 : 1);
        }
        AllocTraits::construct(alloc_, arr_ + sz_,
                std::forward<decltype(args)>(args)...);
        ++sz_;
    }
};

struct S {
    int x;
    S(int x): x(x) {}
};

template <>
class vector<bool> {
    char* arr_;
    size_t sz_;
    size_t cap_;
    struct BitReference {
        char* cell;
        uint8_t index;
        BitReference(char* cell, uint8_t index)
            : cell(cell), index(index) {}
        void operator=(bool b) {
            if (b) {
                *cell |= (1 << index);
            } else {
                *cell &= ~(1 << index);
            }
        }
        operator bool() const {
            return *cell & (1 << index);
        }
    };
public:
    BitReference operator[](size_t index) {
        return BitReference(arr_ + index / 8, static_cast<uint8_t>(index % 8));
    }
};

template <typename T>
class Debug {
    Debug(T) = delete;
};

int main() {
    vector<int> v;
    vector<int>::iterator it = v.begin();
    vector<int>::const_iterator cit = it;
}
