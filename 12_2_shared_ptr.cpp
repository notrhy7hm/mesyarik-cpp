#include <iostream>
#include <memory>

template <typename T>
class shared_ptr {
private:
    T* ptr;
    BaseControlBlock* cb;
    
    struct BaseControlBlock {
        size_t shared_count;
        size_t weak_count;
        virtual ~BaseControlBlock() = 0;
    };

    template <typename U, typename Deleter, typename Alloc>
    struct ControlBlockRegular : BaseControlBlock{
        Deleter del;
        Alloc alloc;
    };

    template <typename U, typename Alloc>
    struct ControlBlockMakeShared: BaseControlBlock {
        U value;
        Alloc alloc;
    };

    template <typename U>
    struct ControlBlockWithObject: ControlBlock {
        U value;
    };

    template <typename U, typename... Args>
    friend shared_ptr<U> make_shared(Args&&...);

    shared_ptr(ControlBlock* cp); //TODO

public:
    shared_ptr(T* ptr)
            : ptr(ptr), count(new size_t(1)) {
        
        if constepxr(std::is_base_of_v<enable_shared_from_this<T>, T>) {
            ptr->sptr = *this;
        }
    }

    shared_ptr(const shared_ptr* other)
        : ptr(ptr), count(count) {
        ++*count;
    }
    
    template <typename Deleter>
    shared_ptr(T* ptr, Deleter del) {

    }

    ~shared_ptr() {
        if (!count) {
            return;
        }
        --*count;
        if (!*count) {
            // delete count;
            delete ptr;
        }
    }
};

template <typename T, typename... Args>
shared_ptr<T> make_shared(Args&&... args) {
    auto* p = new typename shared_ptr<T>::ControlBlock{T(std::forward<Args>(args)...), 1};
    return shared_ptr<T>(p); 
}

template <typename T, typename Alloc, typename... Args>
shared_ptr<T> allocate_shared(Alloc& alloc, Args&&... args) {
    using BlockAlloc = typename std::allocator_traits<Alloc>
        ::rebind_alloc<ControlBlockMakeShared<T, Alloc>>;
    BlockAlloc ba = alloc;
    auto* ptr = ba.allocate(1);
    ba.construct(ptr, 1, 0, std::forward<Args>(args)..., alloc);
    return ...; //TODO
}


template <typename T>
struct enable_shared_from_this {
    weak_ptr<T> wptr;

    shared_ptr<T> shared_from_this() const {
        return sptr;
    }

    enable_shared_from_this() {}
    
    template <typename U>
    friend class shared_ptr;
};

struct S: public std::enable_shared_from_this<S> {
    std::shared_ptr<S> getObject() {
        return shared_from_this();
    }
};

struct Base {};
struct Derived: Base {};

int main() {
    
    auto p = make_shared<Derived>();

    shared_ptr<Base> bp = p;

}
