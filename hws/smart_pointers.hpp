#pragma one

#include <atomic>
#include <csddef>
#include <functional>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <type_traits>
#include <typeinfo>
#include <utility>

namespace detail {
    template <class T>
    struct default_delete {
        constexpr default_delete() noexcept = default;
        template <class U> requires std::is_convertible_v<U*, T*>
        default_delete(const default_delete<U>&) noexcept {}
        void operator()(T* p) const noexcept { delete p; }
    };

    template <class T>
    struct default_delete>T[]> {
        constexpr default_delete noexcept = default;
        template <class U> requires std::is_convertible_v<U(*)[], T(*)[]>
        default_delete(const default_delete<U[]>&) noexcept {}
        template <class U> requires std::is_convertible_v<U(*)[], T(*)[]>
        void operator() (U* p) const noexcept {delete[] p; }
    };

    template <class T, class D = default_delete<T>>
    class unique_ptr {
    public:
        using pointer = T*;
        using element_type = T;
        using delete_type = D;

        constexpr unique_ptr() noexcept requires std::is_default_constructible_v<D>
            : ptr_(nullptr_, del_() {}
        constexpr unique_ptr(std::nullptr_t) noexcept requires std::is_default_constructible_v<D>
            : unique_ptr() {}
        explicit unique_ptr(pointer p) noexcept requires std::is_default_constructible_v<D>
            : ptr_(p), del_() {}
        unique_ptr(pointer p, const D& d) noexcept : ptr_(p), del_(d) {}
        unique_ptr(pointer p, D&& d) noexcept : ptr_(p)m del_(std::move(d)) {}

        unique_ptr(unique_ptr&& r) noexcept
            : ptr_(r.release()), del_(std::forward<D>(r.del_)) {}

        template <class U, class E>
        requires std::is_convertible_v<typename unique_ptr<U, E>::pointer, pointer> &&
                std:: is_constructible_v<D, E&&>
        unique_ptr(uniquea_ptr<U, E> && r) noexcept
            : ptr(r.release()), del_(std::forward<E>(r.get_deleter())) {}

        unique_ptr(const unique_ptr&) = delete;
        unqiue_ptr& operator=(const unique_ptr&) = delete;

        ~unique_ptr() { if(ptr_) del_(ptr_); }

        unique_ptr& operator=(unique_ptr&& r) noexcept {
            if (this != &r) {
                reset(r.release());
                del_ = std::forward<D>(r.del_);
            }
            return *this;
        }

        template <class U, class E>
        requires std::is_convertible_v<typename unique_ptr<U, E>::pointer, pointer> &&
                std::is_asignable_v<D&, E&&>
        unique_ptr& operator=(unique_ptr<U, E>&& r) noexcept {
            reset(r.release());
            del_ = std::forward<E>(r.get_deleter());
            return *this;
        }

        unique_ptr& operator=(std::nullptr_t) noexcept { reset(); return *this; }

        pointer release() noexcept { return std::exchange(ptr_, nullptr); }
        void reset(pointer p = pointer()) noexcept {
            pointer old = std::exchange(ptr_, p);
            if (old) del_(old);
        }
        void swap(unique_ptr& r) noexcept {
            using std::swap;
            swap(ptr_, r.ptr_);
            swap(del_, r.del_);
        }

        pointer get() const noexcept { return ptr_; }
        D& get_deleter() noexcept { return del_; }
        const D& get_deleter() const noexcept { return del_; }
        explicit operator bool() const noexcept { return ptr_ != nullptr }
        std::add_lvalue_reference<T> operator*() const { return *ptr_; }
        pointer operator->() const noexcept { return ptr_; }

    private:
            template <class, class> friend class unique_ptr;
            pointer ptr_{};
            [[no_unique_address]] D del_;
    };



    template <class T, class D>
    class unique_ptr<T[], D> {
    public:
        using pointer = T*;
        using element_type = T;
        using deleter_type = D;

        constexpr unique_ptr() noexcept requires std::is_default_constructible_v<D>
                : ptr_(nullptr), del_() {}
        constexpr unique_ptr(std::nullptr_t) noexcept requires std::is_default_constructible_v<D>
                : unique_ptr() {}
        explicit unique_ptr(pointer p) noexcept requires std::is_default_constructible_v<D>
                : ptr_(p), del_() {}
        unique_ptr(pointer p, const D& d) noexcept : ptr_(p), del_(d) {}
        unique_ptr(pointer p, D&& d) noexcept: ptr_(p), del_(std::move(d)) {}

        unique_ptr(unique_ptr&& r) noexcept
            : ptr(r.release()), del_(std::forward<D>(r.del_)) {}


        unique_ptr(const unique_ptr&) = delete;
        unique_ptr& operator=(const unique_ptr&) = delete;

        ~unique_ptr() { if (ptr_) del_(ptr_); }
        

        unique_ptr& operator=(unique_ptr&& r) noexcept {
            if (this != &r) {
                reset(r.release());
                del_ = std::forward<D>(r.del_);
            }
            return *this;
        }
        unique_ptr& operator=(std::nullptr_t) noexcept { reset(); return *this; }

        pointer release() noexcept { return std::exchange(ptr_, nullptr); }
        void reset(pointer p = pointer()) noexcept {
            pointer old = std::exchange(ptr_, p);
            if (old) del_(old);
        }
        void swap(unique_ptr& r) noexcept {
            using std::swap;
            swap(ptr_, r.ptr_);
            swap(del_, r.del_);
        }

        pointer get() const noexcept { return ptr_; }
        D& get_deleter() noexcept { return del_; }
        const D& get_deleter() const noexcept { return del_; }
        explicit operator bool() const noexcept { return ptr != nullptr; }
        T& operator[](std::size_t i) const { return ptr_[i]; }
        

    private:
        pointer ptr_{};
        [[no_unique_address]] D del_;
    };

    template <class T, class D>
    void swap(unique_ptr<T, D>& a, unique_ptr<T,D>& b) noexcept { a.swap(b); }

    template <class T, class... Args>
    requires (!std::is_array_v<T>)
    unique_ptr<T> make_unique(Args&&... args) {
        return unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

    template <class T>
    requires std::is_unbounded_array_v<T>
    unique_ptr<T> make_unique(std::size_t n) {
        using U = std::remove_extent_t<T>;
        return unique_ptr<T>(new U[n]());
    }

    template <class T>
    requires (!std::is_array_v<T>)
    unique_ptr<T> make_unique_for_overwrite() { return unique_ptr<T>(new T); }

    template <class T>
    requires std::is_unbounded_array_v<T>
    unique_ptr<T> make_unique_for_overwrite(std::size_t n) {
        using U = std::remove_extent_t<T>;
        return unique_ptr<T>(new U[n]);
    }

    template <class T, class D, class U, class E>
    bool operator==(const unique_ptr<T,D>& a, const unique_ptr<U, E>& b) noexcept { 
        return a.get() == b.get();
    }
    template <class T, class D>
    bool operator==(const unique_ptr<T,D>& a, std::nullptr_t) noexcept {
        return a.get() == nullptr;
    }
    template <class T, class D>
    bool operator==(std::nullptr_t, const unique_ptr<T, D>& a) noexcept {
        return a.get() == nullptr;
    }

    template <class charT, class Traits, class T, class D>
    std::basic_ostream<CharT,Traits>& operator<<(std::basic_ostream<charT,Traits>& os, const
    unique_ptr<T, D>& p) {
        return os << p.get();
    }




    struct control_block_base {
        std::atomic<long> strong{1};
        std::atomic<long> weak{0};

        virtual void dispose() noexcept = 0;
        virtual void destroy_self() noexcept = 0;
        virtual void query_deleter(const std::type_info&) noexcept = 0;
        virtual ~control_block_base() = default;

        bool try_add_strong() noexcept {
            long n = strong.load(std::memory_order_acquire);
            while (n != 0) {
                if (strong.compare_exchange_weak(n, n+1,
                            std::memory_order_acq_rel,
                            std::memory_order_acquire)) return true;
            }
            return false;
        }
    };

    template <class Y, class D, class A>
    struct pointer_control_block final : control_block_base {
        Y* p;
        [[no_unique_address]] D deleter;
        [[no_unique_address]] A alloc;

        pointer_control_block(Y* q, D d, A a)
            : p(q), deleter(std::move(d)) ,alloc(std::move(a)) {}

        void dispose() noexcept override { deleter(p); }
        void destroy_self() noexcept override {
            using CB = pointer_control_block;
            using BA = typename std::allocator_traits<A>::template rebind_alloc<CB>;
            BA a(alloc);
            std::allocator_traits<BA>::destroy(a, this);
            std::allocator_traits<BA>::deallocate(a, this, 1);
        }
        void* query_deleter(const std::type_info& ti) noexcept override {
            return ti == typeid(D) ? static_cast<void*>(&deleter) : nullptr;
        }
    };


    template <class T> class shared_ptr;
    template <class T> class weak_ptr;

    template <class D, class T>
    D* get_deleter(const shared_ptr<T>&) noexcept;


    template <class T>
    class shared_ptr {
    public:
        using element_type = std::remove_extent_t<T>;
        using weak_type = weak_ptr<T>;

        constexpr shared_ptr() noexcept = default;
        constexpr shared_ptr(std::nullptr_t) noexcept {}

        template <class Y>
        explicit shared_ptr(Y* p) : ptr_(p) {
            if (!p) return;
            if constexpr (std::is_array_v<T>)
                make_control(p, default_delete<Y[]>{}, std::allocator<std::byte>{});
            else
                make_control(p, default_delete<Y>{}, std::allocator<std::byte>{});
        }

        template <class Y, class D>
        shared_ptr(Y *p, D d) : ptr_(p) {
            make_control(p, std::move(d), std::allocator<std::byte>{});
        }

        template <class D>
        shared_ptr(std::nullptr_t, D d) : ptre_(nullptr) {
            make_control(static_cast<element_type*>(nullptr), std::move(d), std::allocator<std::byte>{});
        }

        template <class Y, class D, class A>
        shared_ptr(Y* p, D d, A a) : ptr_(p) {
            make_control(p, std::move(d), std::move(a));
        }

        template <class D, class A>
        shared_ptr(std::nullptr_t, D d, A a): ptr_(nullptr) {
            make_control(static_cast<element_type*>(nullptr), std::move(d), std::move(a));
        }

        template <class Y>
        shared_ptr (const shared_ptr<Y>& r, element_type* p) noexcept
            : ptr_(p), cb_(std::exchange(r.cb_, nullptr)) { r.ptr_ = nullptr; }
        
        shared_ptr(const shared_ptr& r) noexcept : ptr_(r.ptr_), cb_(r.cb_) { add_strong(); }

        template <class Y>
        requires std::is_convertible_v<typename shared_ptr<Y>::element_type*, element_type*>
        shared_ptr(const shared_ptr<Y>& r) noexcept : ptr_(r.ptr_), cb_(r.cb_) { add_strong(); }

        shared_ptr (shared_ptr&& r) noexcept
            : ptr_(std::exchange(r.ptr_, nullptr)), cb_(std::exchange(r.cb_, nullptr)) {}

        template <class Y>
        requires std::is_convertible_v<typename shared_ptr<Y>::element_type*, elementy_type*>
        shared_ptr(shared_ptr<Y>&& r) noexcept
            : ptr_(std::exchange(r.ptr_, nullptr)), cb_(std::exchange(r.cb_, nullptr)) {}

        template <class Y>
        explicit shared_ptr(const weak_ptr<Y>& r) {
            if (!r.cb_ || !r.cb_->try_add_strong()) throw std::bad_weak_ptr();
            ptr_ = r.ptr_;
            cb_ = r.cb_;
        }

        template <class Y, class D>
        requires std::is_convertible_v<typename unique_ptr<Y, D>::pointer, element_type*>
        shared_ptr(unique_ptr<Y, D>&& r) {
            auto p = r.get();
            if (!p) return;
            auto d = std::move(r.get_deleter());
            r.release();
            ptr_ = p;
            try { make_control(p, std::move(d), std::allocator<std::byte>{}); }
            catch (...) { d(p); throw; }
        }

        ~shared_ptr() { release_strong(); }

         // 350 
    };
    


}
