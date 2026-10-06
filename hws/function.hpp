#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <new> 
#include <type_traits>
#include <utility>

namespace function_detail {

template <typename Signature, bool Copyable>
class BasicFunction;

template <typename R, bool Copyable, typename... Args>
class BasicFunction<R(Args...), Copyable> {
private:
    static constexpr std::size_t kStorageSize = 32;
    static constexpr std::size_t kStorageAlign = alignof(std::max_align_t);

    using Storage = std::aligned_storage_t<kStorageSize, kStorageAlign>;

    struct Ops {
        R (*invoke)(void*, Args&&...);
        void (*destroy)(BasicFunction&) noexcept;
        void (*move)(BasicFunction&, BasicFunction&) noexcept;
        void (*copy)(const BasicFunction&, BasicFunction&);
    };

    template <typename F>
    static constexpr bool kFitsInline =
        sizeof(F) <= kStorageSize &&
        alignof(F) <= kStorageAlign &&
        std::is_nothrow_move_constructible_v<F>;

    Storage storage_;
    void* heap_ptr_ = nullptr;
    const Ops* ops_ = nullptr;
    bool heap_ = false;

    void* object_ptr() noexcept {
        return heap_ ? heap_ptr_ : static_cast<void*>(std::addressof(storage_));
    }

    const void* object_ptr() const noexcept {
        return heap_ ? heap_ptr_ : static_cast<const void*>(std::addressof(storage_));
    }

    template <typename F>
    static F* as(void* p) noexcept {
        return std::launder(reinterpret_cast<F*>(p));
    }

    template <typename F>
    static const F* as(const void* p) noexcept {
        return std::launder(reinterpret_cast<const F*>(p));
    }

    template <typename F>
    static R invoke_impl(void* p, Args&&... args) {
        F& callable = *as<F>(p);
        if constexpr (std::is_void_v<R>) {
            std::invoke(callable, std::forward<Args>(args)...);
        } else {
            return static_cast<R>(std::invoke(callable, std::forward<Args>(args)...));
        }
    }

    template <typename F>
    static void destroy_impl(BasicFunction& self) noexcept {
        if (self.heap_) {
            delete as<F>(self.heap_ptr_);
            self.heap_ptr_ = nullptr;
        } else {
            std::destroy_at(as<F>(std::addressof(self.storage_)));
        }
        self.ops_ = nullptr;
        self.heap_ = false;
    }

    template <typename F>
    static void move_impl(BasicFunction& src, BasicFunction& dst) noexcept {
        if (src.heap_) {
            dst.heap_ptr_ = src.heap_ptr_;
            dst.heap_ = true;
            dst.ops_ = src.ops_;

            src.heap_ptr_ = nullptr;
            src.heap_ = false;
            src.ops_ = nullptr;
        } else {
            F* source = as<F>(std::addressof(src.storage_));
            ::new (static_cast<void*>(std::addressof(dst.storage_))) F(std::move(*source));
            std::destroy_at(source);

            dst.heap_ = false;
            dst.ops_ = src.ops_;
            src.ops_ = nullptr;
            src.heap_ = false;
        }
    }

    template <typename F>
    static void copy_impl(const BasicFunction& src, BasicFunction& dst) {
        if constexpr (!Copyable) {
            (void)src;
            (void)dst;
        } else {
            const F& source = *as<F>(src.object_ptr());
            if constexpr (kFitsInline<F>) {
                ::new (static_cast<void*>(std::addressof(dst.storage_))) F(source);
                dst.heap_ = false;
                dst.ops_ = src.ops_;
            } else {
                F* p = new F(source);
                dst.heap_ptr_ = p;
                dst.heap_ = true;
                dst.ops_= src.ops_;
            }
        }
    }
    
    template <typename F>
    static const Ops* ops_for() noexcept {
        static const Ops table {
            &invoke_impl<F>,
            &destroy_impl<F>,
            &move_impl<F>,
            &copy_impl<F>
        };
        return &table;
    }

    template <typename F, typename U>
    void construct(U&& value) {
        if constexpr (kFitsInline<F>) {
            ::new (static_cast<void*>(std::addressof(storage_))) F(std::forward<U>(value));
            heap_ = false;
        } else {
            heap_ptr_ = new F(std::forward<U>(value));
            heap_ = true;
        }   
        ops_ = ops_for<F>();
    }

protected:
    BasicFunction() noexcept = default;
    BasicFunction(std::nullptr_t) noexcept {}

    BasicFunction(BasicFunction&& other) noexcept {
        if (other.ops_ != nullptr) {
            other.ops_->move(other, *this);
        }
    }
    
    BasicFunction& move_assign(BasicFunction&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        reset();
        if (other.ops_ != nullptr) {
            other.ops_->move(other, *this);
        }
        return *this;
    }

    BasicFunction(const BasicFunction& other) requires (Copyable) {
        if (other.ops_ != nullptr) {
            other.ops_->copy(other, *this);
        }
    }

    BasicFunction& copy_assign(const BasicFunction& other) requires (Copyable) {
        if (this == &other) {
            return *this;
        }
        BasicFunction tmp(other);
        swap(tmp);
        return *this;
    }

    template <typename F>
    static constexpr bool acceptable_callable =
        !std::is_same_v<std::remove_cvref_t<F>, BasicFunction> &&
        std::is_invocable_r_v<R, std::decay_t<F>&, Args...> &&
        std::is_constructible_v<std::decay_t<F>, F> &&
        (!Copyable || std::is_copy_constructible_v<std::decay_t<F>>);

    template <typename F>
        requires acceptable_callable<F>
    void assign_callable(F&& f) {
        using D = std::decay_t<F>;
        BasicFunction tmp;
        tmp.template construct<D>(std::forward<F>(f));
        swap(tmp);
    }

    ~BasicFunction() {
        reset();
    }

public:
    explicit operator bool() const noexcept {
        return ops_ != nullptr;
    }

    bool operator==(std::nullptr_t) const noexcept {
        return ops_ == nullptr;
    }

    bool operator!=(std::nullptr_t) const noexcept {
        return ops_ != nullptr;
    }

    R operator()(Args... args) const {
        if (ops_ == nullptr) {
            throw std::bad_function_call();
        }

        void* p = const_cast<void*>(object_ptr());
        if constexpr (std::is_void_v<R>) {
            ops_->invoke(p, std::forward<Args>(args)...);
        } else {
            return ops_->invoke(p, std::forward<Args>(args)...);
        }
    }

    void reset() noexcept {
        if (ops_ != nullptr) {
            ops_->destroy(*this);
        }
    }

    void swap (BasicFunction& other) noexcept {
        if (this == &other) {
            return;
        }

        BasicFunction tmp(std::move(other));
        if (ops_ != nullptr) {
            ops_->move(*this, other);
        }
        if (tmp.ops_ != nullptr) {
            tmp.ops_->move(tmp, *this);
        }
    }
};

template <typename T>
struct callable_signature;

template <typename R, typename... Args>
struct callable_signature<R(*)(Args...)> {
    using type = R(Args...);
};

template <typename R, typename... Args>
struct callable_signature<R(&)(Args...)> {
    using type = R(Args...);
};

template <typename C, typename R, typename... Args>
struct callable_signature<R(C::*)(Args...)> {
    using type = R(Args...);
};

template <typename C, typename R, typename... Args>
struct callable_signature<R(C::*)(Args...) const> {
    using type = R(Args...);
};

template <typename C, typename R, typename... Args>
struct callable_signature<R(C::*)(Args...) &> {
    using type = R(Args...);
};

template <typename C, typename R, typename... Args>
struct callable_signature<R(C::*)(Args...) const &> {
    using type = R(Args...);  
};

template <typename C, typename R, typename... Args>
struct callable_signature<R(C::*)(Args...) &&> {
    using type = R(Args...);
};

template <typename C, typename R, typename... Args>
struct callable_signature<R(C::*)(Args...) const &&> {
    using type = R(Args...);
};

template <typename F>
struct functor_signature : callable_signature<decltype(&std::remove_reference_t<F>::operator())> {};

template <typename F>
using deducted_signature_t = typename std::conditional_t<
    std::is_pointer_v<std::decay_t<F>>,
    callable_signature<std::decay_t<F>>,
    functor_signature<std::decay_t<F>>
>::type;
}

template <typename Signature>
class Function;

template <typename R, typename... Args>
class Function<R(Args...)> : public function_detail::BasicFunction<R(Args...), true> {
private:
    using Base = function_detail::BasicFunction<R(Args...), true>;

    template <typename>
    friend class Function;

public:
    Function() noexcept = default;
    Function(std::nullptr_t) noexcept : Base(nullptr) {}

    Function(const Function& other) : Base(static_cast<const Base&>(other)) {}
    Function(Function&& other) noexcept : Base(std::move(static_cast<Base&>(other))) {}

    template <typename F>
        requires (
            !std::is_same_v<std::remove_cvref_t<F>, Function> &&
            std::is_invocable_r_v<R, std::decay_t<F>&, Args...> &&
            std::is_constructible_v<std::decay_t<F>, F> &&
            std::is_copy_constructible_v<std::decay_t<F>>)
    Function(F&& f) {
        this->template assign_callable<F>(std::forward<F>(f));
    }

    Function& operator=(const Function& other) {
        this->copy_assign(static_cast<const Base&>(other));
        return *this;
    }

    Function& operator= (Function&& other) {
        this->move_assign(std::move(static_cast<Base&>(other)));
        return *this;
    }

    Function& operator=(std::nullptr_t) noexcept {
        this->reset();
        return *this;
    }

    template <typename F>
        requires (
            !std::is_same_v<std::remove_cvref_t<F>, Function> &&
            std::is_invocable_r_v<R, std::decay_t<F>&, Args...> &&
            std::is_constructible_v<std::decay_t<F>, F> &&
            std::is_copy_constructible_v<std::decay_t<F>>)
    Function& operator=(F&& f) {
        this->template assign_callable<F>(std::forward<F>(f));
        return *this;
    }

    using Base::operator();
    using Base::operator bool;
    using Base::operator==;
    using Base::operator!=;
    using Base::reset;
    using Base::swap;
};

template <typename Signature>
class MoveOnlyFunction;

template <typename R, typename... Args>
class MoveOnlyFunction<R(Args...)>
        : public function_detail::BasicFunction<R(Args...), false> {
private :
    using Base = function_detail::BasicFunction<R(Args...), false>;

public:
    MoveOnlyFunction() noexcept = default;
    MoveOnlyFunction(std::nullptr_t) noexcept : Base(nullptr) {}

    MoveOnlyFunction(const MoveOnlyFunction&) = delete;
    MoveOnlyFunction& operator=(const MoveOnlyFunction&) = delete;

    MoveOnlyFunction(MoveOnlyFunction&& other) noexcept 
        : Base(std::move(static_cast<Base&>(other))) {}

    template <typename F>
        requires (
            !std::is_same_v<std::remove_cvref_t<F>, MoveOnlyFunction> &&
            std::is_invocable_r_v<R, std::decay_t<F>&, Args...> &&
            std::is_constructible_v<std::decay_t<F>, F>)
    MoveOnlyFunction(F&& f) {
        this->template assign_callable<F>(std::forward<F>(f));
    }

    MoveOnlyFunction& operator=(MoveOnlyFunction&& other) noexcept {
        this->move_assign(std::move(static_cast<Base&>(other)));
        return *this;
    }

    MoveOnlyFunction& operator=(std::nullptr_t) noexcept {
        this->reset();
        return *this;
    }

    template <typename F>
        requires (
            !std::is_same_v<std::remove_cvref_t<F>, MoveOnlyFunction> &&
            std::is_invocable_r_v<R, std::decay_t<F>&, Args...> &&
            std::is_constructible_v<std::decay_t<F>, F>)
    MoveOnlyFunction& operator=(F&& f) {
        this->template assign_callable<F>(std::forward<F>(f));
        return *this;
    }

    using Base::operator();
    using Base::operator bool;
    using Base::operator==;
    using Base::operator!=;
    using Base::reset;
    using Base::swap;
};

template <typename F>
Function(F) -> Function<function_detail::deducted_signature_t<F>>;
