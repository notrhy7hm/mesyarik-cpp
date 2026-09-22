#include <iostream>
#include <vector>


template <typename Alloc>
struct allocator_traits {

    template <typename U, typename... Args>
    static void construct(Alloc& alloc, U* ptr, const Args&... args) {
        if constexpr ( /* Alloc has method construct*/) {
            alloc.construct(ptr, args...);
        } else {
            new (ptr) U(args...);
        }
    }
};
i

template <typename T>
T&& forward(std::remove_reference_t<T>& value) {
    return static_cast<T&&>(value);
}

template <typename T>
T&& forward(std::remove_reference_t<T>&& value) {
    static_assert(!std::is_lvalue_reference_v<T>);
    return static_cast<T&&>(value);
}

template <typename T>
auto move(T&& value) -> std::remove_reference_t<T>&&
{
    return static_cast<std::remove_reference_t<T>&&>(value);
}

template <typename T>
struct allocator {
    T* allocate(size_t count) {
        return operator new(count * sizeof(T));
    }
    void deallocate(T* ptr, size_t) {
        operator delete(ptr);
    }

    template <typename U, typename... Args>
    void construct(U* ptr, Args&&... args) {
        new (ptr) U(std::forward<Args>(args)...);
    }
    
    template <typename U>
    void destroy(U* ptr) {
        ptr->~U();
    }

    template <typename U>
    allocator(allocator<U>) {}

    template <typename U>
    struct rebind {
        using other = allocator<U>;
    };
};

template <typename T, typename Alloc = std::allocator<T>>
class list {
    struct BaseNode {
        BaseNode* prev;
        BaseNode* next;
    };
    struct Node : BaseNode {
        T value;
    };

    BaseNode fakeNode;
    size_t count;
    typename Alloc::template rebind<Node>::other alloc;

    list(const Alloc& alloc): fakeNode(), count(), alloc(alloc) {}
    
};




int main() {
    allocator<int> a;
    std::vector<int> v;
    static_assert((sizeof(v) == 24));
}
