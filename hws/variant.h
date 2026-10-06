#pragma once

#include <cstddef>
#include <exception>
#include <functional>
#include <initializer_list>
#include <memory>
#include <new>
#include <type_traits>
#include <tuple>
#include <utility>
#include <variant>

namespace variant_detail {

template <std::size_t I, typename T, typename... Ts>
struct nth_type_impl : nth_type_impl<I - 1, Ts...> {};

template <typename T, typename... Ts>
struct nth_type_impl<0, T, Ts...> {
    using type = T;
};

template <std::size_t I, typename... Ts>
using nth_type_t = typename nth_type_impl<I, Ts...>::type;

template <typename T, typename... Ts>
inline constexpr std::size_t type_count_v = (0u + ... + (std::is_same_v<T, Ts> ? 1u : 0u));

template <typename T, typename First, typename... Rest>
consteval std::size_t type_index_fn() {
    if constexpr (std::is_same_v<T, First>) {
        return 0;
    } else {
        static_assert(sizeof...(Rest) > 0, "type is not an alternative of Variant");
        return 1 + type_index_fn<T, Rest...>();
    }
}

template <typename T, typename... Ts>
struct type_index_impl {
    static constexpr std::size_t value = type_index_fn<T, Ts...>();
};

template <typename T>
void list_init_probe(T);

template <typename To, typename From, typename = void>
struct is_list_initializable : std::false_type {};

template <typename To, typename From>
struct is_list_initializable<
    To,
    From,
    std::void_t<decltype(list_init_probe<To>({std::declval<From>()}))>>
    : std::true_type {};

struct no_match {};

template <std::size_t I, typename T, typename U,
          bool = is_list_initializable<T, U>::value>
struct candidate {
    static std::integral_constant<std::size_t, I> select(no_match);
};

template <std::size_t I, typename T, typename U>
struct candidate<I, T, U, true> {
    static std::integral_constant<std::size_t, I> select(T);
};

template <typename U, typename Seq, typename... Ts>
struct overload_set;

template <typename U, std::size_t... Is, typename... Ts>
struct overload_set<U, std::index_sequence<Is...>, Ts...>
    : candidate<Is, Ts, U>... {
    using candidate<Is, Ts, U>::select...;
};

template <typename U, typename Dummy, typename... Ts>
struct selected_index_impl {};

template <typename U, typename... Ts>
struct selected_index_impl<
    U,
    std::void_t<decltype(overload_set<U, std::index_sequence_for<Ts...>, Ts...>::select(
        std::declval<U>()))>,
    Ts...> {
    using type = decltype(overload_set<U, std::index_sequence_for<Ts...>, Ts...>::select(
        std::declval<U>()));
    static constexpr std::size_t value = type::value;
};

template <typename U, typename... Ts>
inline constexpr bool has_selected_index_v = requires {
    typename selected_index_impl<U, void, Ts...>::type;
};

template <typename U, typename... Ts>
inline constexpr std::size_t selected_index_v = selected_index_impl<U, void, Ts...>::value;

template <typename... Ts>
constexpr std::size_t max_sizeof() {
    std::size_t result = 0;
    ((result = result < sizeof(Ts) ? sizeof(Ts) : result), ...);
    return result;
}

template <typename... Ts>
constexpr std::size_t max_alignof() {
    std::size_t result = 0;
    ((result = result < alignof(Ts) ? alignof(Ts) : result), ...);
    return result;
}

} 

class BadVariantAccess : public std::exception {
public:
    const char* what() const noexcept override {
        return "bad Variant access";
    }
};

template <typename... Ts>
class Variant;

template <std::size_t I, typename... Ts>
variant_detail::nth_type_t<I, Ts...>& get(Variant<Ts...>& v);

template <std::size_t I, typename... Ts>
const variant_detail::nth_type_t<I, Ts...>& get(const Variant<Ts...>& v);

template <std::size_t I, typename... Ts>
variant_detail::nth_type_t<I, Ts...>&& get(Variant<Ts...>&& v);

template <std::size_t I, typename... Ts>
const variant_detail::nth_type_t<I, Ts...>&& get(const Variant<Ts...>&& v);

template <typename T, typename... Ts>
T& get(Variant<Ts...>& v);

template <typename T, typename... Ts>
const T& get(const Variant<Ts...>& v);

template <typename T, typename... Ts>
T&& get(Variant<Ts...>&& v);

template <typename T, typename... Ts>
const T&& get(const Variant<Ts...>&& v);

template <typename... Ts>
class Variant {
    static_assert(sizeof...(Ts) > 0, "Variant must have at least one alternative");

public:
    static constexpr std::size_t npos = static_cast<std::size_t>(-1);

private:
    static constexpr std::size_t kStorageSize = variant_detail::max_sizeof<Ts...>();
    static constexpr std::size_t kStorageAlign = variant_detail::max_alignof<Ts...>();
    using Storage = std::aligned_storage_t<kStorageSize, kStorageAlign>;

    Storage storage_;
    std::size_t index_ = npos;

    template <std::size_t I>
    using Alt = variant_detail::nth_type_t<I, Ts...>;

    template <std::size_t I>
    Alt<I>* ptr() noexcept {
        return std::launder(reinterpret_cast<Alt<I>*>(std::addressof(storage_)));
    }

    template <std::size_t I>
    const Alt<I>* ptr() const noexcept {
        return std::launder(reinterpret_cast<const Alt<I>*>(std::addressof(storage_)));
    }

    template <std::size_t I = 0>
    void destroy_active() noexcept {
        if constexpr (I < sizeof...(Ts)) {
            if (index_ == I) {
                std::destroy_at(ptr<I>());
                index_ = npos;
                return;
            }
            destroy_active<I + 1>();
        }
    }

    template <std::size_t I = 0>
    void copy_from(const Variant& other) {
        if constexpr (I < sizeof...(Ts)) {
            if (other.index_ == I) {
                ::new (static_cast<void*>(std::addressof(storage_))) Alt<I>(*other.template ptr<I>());
                index_ = I;
                return;
            }
            copy_from<I + 1>(other);
        }
    }

    template <std::size_t I = 0>
    void move_from(Variant&& other) {
        if constexpr (I < sizeof...(Ts)) {
            if (other.index_ == I) {
                ::new (static_cast<void*>(std::addressof(storage_))) Alt<I>(
                    std::move(*other.template ptr<I>()));
                index_ = I;
                return;
            }
            move_from<I + 1>(std::move(other));
        }
    }

    template <std::size_t I, typename... Args>
    Alt<I>& construct_alt(Args&&... args) {
        ::new (static_cast<void*>(std::addressof(storage_))) Alt<I>(
            std::forward<Args>(args)...);
        index_ = I;
        return *ptr<I>();
    }

public:
    Variant() requires std::is_default_constructible_v<Alt<0>> {
        construct_alt<0>();
    }

    ~Variant() {
        destroy_active();
    }

    Variant(const Variant& other)
        requires ((std::is_copy_constructible_v<Ts>) && ...)
    {
        if (!other.valueless_by_exception()) {
            copy_from(other);
        }
    }

    Variant(Variant&& other) noexcept(((std::is_nothrow_move_constructible_v<Ts>) && ...))
        requires ((std::is_move_constructible_v<Ts>) && ...)
    {
        if (!other.valueless_by_exception()) {
            move_from(std::move(other));
        }
    }

    template <typename U>
        requires (
            !std::is_same_v<std::remove_cvref_t<U>, Variant> &&
            variant_detail::has_selected_index_v<U&&, Ts...> &&
            std::is_constructible_v<
                Alt<variant_detail::selected_index_v<U&&, Ts...>>, U&&>)
    Variant(U&& value) {
        constexpr std::size_t I = variant_detail::selected_index_v<U&&, Ts...>;
        construct_alt<I>(std::forward<U>(value));
    }

    Variant& operator=(const Variant& other)
        requires (((std::is_copy_constructible_v<Ts>) && ...))
    {
        if (this == &other) {
            return *this;
        }
        destroy_active();
        if (!other.valueless_by_exception()) {
            copy_from(other);
        }
        return *this;
    }

    Variant& operator=(Variant&& other)
        noexcept(((std::is_nothrow_move_constructible_v<Ts>) && ...))
        requires (((std::is_move_constructible_v<Ts>) && ...))
    {
        if (this == &other) {
            return *this;
        }
        destroy_active();
        if (!other.valueless_by_exception()) {
            move_from(std::move(other));
        }
        return *this;
    }

    template <typename U>
        requires (
            !std::is_same_v<std::remove_cvref_t<U>, Variant> &&
            variant_detail::has_selected_index_v<U&&, Ts...> &&
            std::is_constructible_v<
                Alt<variant_detail::selected_index_v<U&&, Ts...>>, U&&>)
    Variant& operator=(U&& value) {
        constexpr std::size_t I = variant_detail::selected_index_v<U&&, Ts...>;
        destroy_active();
        construct_alt<I>(std::forward<U>(value));
        return *this;
    }

    template <typename T, typename... Args>
        requires (variant_detail::type_count_v<T, Ts...> == 1 &&
                  std::is_constructible_v<T, Args&&...>)
    T& emplace(Args&&... args) {
        constexpr std::size_t I = variant_detail::type_index_impl<T, Ts...>::value;
        destroy_active();
        return construct_alt<I>(std::forward<Args>(args)...);
    }

    template <typename T, typename U, typename... Args>
        requires (variant_detail::type_count_v<T, Ts...> == 1 &&
                  std::is_constructible_v<T, std::initializer_list<U>&, Args&&...>)
    T& emplace(std::initializer_list<U> il, Args&&... args) {
        constexpr std::size_t I = variant_detail::type_index_impl<T, Ts...>::value;
        destroy_active();
        return construct_alt<I>(il, std::forward<Args>(args)...);
    }

    template <std::size_t I, typename... Args>
        requires (I < sizeof...(Ts) && std::is_constructible_v<Alt<I>, Args&&...>)
    Alt<I>& emplace(Args&&... args) {
        destroy_active();
        return construct_alt<I>(std::forward<Args>(args)...);
    }

    template <std::size_t I, typename U, typename... Args>
        requires (I < sizeof...(Ts) &&
                  std::is_constructible_v<Alt<I>, std::initializer_list<U>&, Args&&...>)
    Alt<I>& emplace(std::initializer_list<U> il, Args&&... args) {
        destroy_active();
        return construct_alt<I>(il, std::forward<Args>(args)...);
    }

    [[nodiscard]] std::size_t index() const noexcept {
        return index_;
    }

    [[nodiscard]] bool valueless_by_exception() const noexcept {
        return index_ == npos;
    }

private:
    template <std::size_t I, typename... Us>
    friend variant_detail::nth_type_t<I, Us...>& get(Variant<Us...>& v);
    template <std::size_t I, typename... Us>
    friend const variant_detail::nth_type_t<I, Us...>& get(const Variant<Us...>& v);
    template <std::size_t I, typename... Us>
    friend variant_detail::nth_type_t<I, Us...>&& get(Variant<Us...>&& v);
    template <std::size_t I, typename... Us>
    friend const variant_detail::nth_type_t<I, Us...>&& get(const Variant<Us...>&& v);
};

template <std::size_t I, typename... Ts>
variant_detail::nth_type_t<I, Ts...>& get(Variant<Ts...>& v) {
    static_assert(I < sizeof...(Ts));
    if (v.index_ != I) {
        throw BadVariantAccess();
    }
    return *v.template ptr<I>();
}

template <std::size_t I, typename... Ts>
const variant_detail::nth_type_t<I, Ts...>& get(const Variant<Ts...>& v) {
    static_assert(I < sizeof...(Ts));
    if (v.index_ != I) {
        throw BadVariantAccess();
    }
    return *v.template ptr<I>();
}

template <std::size_t I, typename... Ts>
variant_detail::nth_type_t<I, Ts...>&& get(Variant<Ts...>&& v) {
    return std::move(get<I>(v));
}

template <std::size_t I, typename... Ts>
const variant_detail::nth_type_t<I, Ts...>&& get(const Variant<Ts...>&& v) {
    return std::move(get<I>(v));
}

template <typename T, typename... Ts>
T& get(Variant<Ts...>& v) {
    static_assert(variant_detail::type_count_v<T, Ts...> == 1,
                  "T must occur exactly once in Variant");
    constexpr std::size_t I = variant_detail::type_index_impl<T, Ts...>::value;
    return get<I>(v);
}

template <typename T, typename... Ts>
const T& get(const Variant<Ts...>& v) {
    static_assert(variant_detail::type_count_v<T, Ts...> == 1,
                  "T must occur exactly once in Variant");
    constexpr std::size_t I = variant_detail::type_index_impl<T, Ts...>::value;
    return get<I>(v);
}

template <typename T, typename... Ts>
T&& get(Variant<Ts...>&& v) {
    static_assert(variant_detail::type_count_v<T, Ts...> == 1,
                  "T must occur exactly once in Variant");
    constexpr std::size_t I = variant_detail::type_index_impl<T, Ts...>::value;
    return get<I>(std::move(v));
}

template <typename T, typename... Ts>
const T&& get(const Variant<Ts...>&& v) {
    static_assert(variant_detail::type_count_v<T, Ts...> == 1,
                  "T must occur exactly once in Variant");
    constexpr std::size_t I = variant_detail::type_index_impl<T, Ts...>::value;
    return get<I>(std::move(v));
}

template <typename T, typename... Ts>
bool holds_alternative(const Variant<Ts...>& v) noexcept {
    static_assert(variant_detail::type_count_v<T, Ts...> == 1,
                  "T must occur exactly once in Variant");
    constexpr std::size_t I = variant_detail::type_index_impl<T, Ts...>::value;
    return v.index() == I;
}

template <std::size_t I, typename... Ts>
auto* get_if(Variant<Ts...>* v) noexcept {
    using T = variant_detail::nth_type_t<I, Ts...>;
    if (v == nullptr || v->index() != I) {
        return static_cast<T*>(nullptr);
    }
    return std::addressof(get<I>(*v));
}

template <std::size_t I, typename... Ts>
const auto* get_if(const Variant<Ts...>* v) noexcept {
    using T = variant_detail::nth_type_t<I, Ts...>;
    if (v == nullptr || v->index() != I) {
        return static_cast<const T*>(nullptr);
    }
    return std::addressof(get<I>(*v));
}

template <typename T, typename... Ts>
T* get_if(Variant<Ts...>* v) noexcept {
    static_assert(variant_detail::type_count_v<T, Ts...> == 1,
                  "T must occur exactly once in Variant");
    constexpr std::size_t I = variant_detail::type_index_impl<T, Ts...>::value;
    return get_if<I>(v);
}

template <typename T, typename... Ts>
const T* get_if(const Variant<Ts...>* v) noexcept {
    static_assert(variant_detail::type_count_v<T, Ts...> == 1,
                  "T must occur exactly once in Variant");
    constexpr std::size_t I = variant_detail::type_index_impl<T, Ts...>::value;
    return get_if<I>(v);
}

namespace variant_detail {

template <typename T>
struct variant_size;

template <typename... Us>
struct variant_size<Variant<Us...>> : std::integral_constant<std::size_t, sizeof...(Us)> {};

template <typename VariantLike>
using first_alt_ref_t = decltype(get<0>(std::declval<VariantLike>()));

template <typename Visitor, typename... Variants>
using visit_result_t = std::invoke_result_t<Visitor, first_alt_ref_t<Variants>...>;

template <typename R, std::size_t... AltIs, std::size_t... VarIs,
          typename Visitor, typename Tuple>
R invoke_selected(std::index_sequence<AltIs...>, std::index_sequence<VarIs...>,
                  Visitor&& visitor, Tuple&& vars) {
    if constexpr (std::is_void_v<R>) {
        std::invoke(
            std::forward<Visitor>(visitor),
            get<AltIs>(std::get<VarIs>(std::forward<Tuple>(vars)))...);
    } else {
        return std::invoke(
            std::forward<Visitor>(visitor),
            get<AltIs>(std::get<VarIs>(std::forward<Tuple>(vars)))...);
    }
}

template <typename R, std::size_t Pos, std::size_t Candidate, std::size_t... AltIs,
          typename Visitor, typename Tuple>
R visit_pick(std::index_sequence<AltIs...>, Visitor&& visitor, Tuple&& vars) {
    auto&& current = std::get<Pos>(std::forward<Tuple>(vars));
    using CurrentVariant = std::remove_cv_t<std::remove_reference_t<decltype(current)>>;
    constexpr std::size_t Count = variant_size<CurrentVariant>::value;
    constexpr std::size_t VariantCount = std::tuple_size_v<std::remove_reference_t<Tuple>>;

    if (current.index() == Candidate) {
        if constexpr (Pos + 1 == VariantCount) {
            return invoke_selected<R>(
                std::index_sequence<AltIs..., Candidate>{},
                std::make_index_sequence<VariantCount>{},
                std::forward<Visitor>(visitor),
                std::forward<Tuple>(vars));
        } else {
            return visit_pick<R, Pos + 1, 0>(
                std::index_sequence<AltIs..., Candidate>{},
                std::forward<Visitor>(visitor),
                std::forward<Tuple>(vars));
        }
    }

    if constexpr (Candidate + 1 < Count) {
        return visit_pick<R, Pos, Candidate + 1>(
            std::index_sequence<AltIs...>{},
            std::forward<Visitor>(visitor),
            std::forward<Tuple>(vars));
    } else {
        throw BadVariantAccess();
    }
}

} 

template <typename Visitor, typename... Variants>
decltype(auto) visit(Visitor&& visitor, Variants&&... variants) {
    static_assert(sizeof...(Variants) > 0, "visit needs at least one Variant");

    using R = variant_detail::visit_result_t<Visitor&&, Variants&&...>;
    auto tuple = std::forward_as_tuple(std::forward<Variants>(variants)...);
    return variant_detail::visit_pick<R, 0, 0>(
        std::index_sequence<>{},
        std::forward<Visitor>(visitor),
        std::move(tuple));
}


