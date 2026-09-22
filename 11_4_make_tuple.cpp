#include <iostream>
#include <tuple>

template <typename... Types>
auto make_tuple(Types&&... args) -> tuple<std::decay_t<Types>...> {
    return {std::forward<Types(args)...};
}

template <typename... Types>
auto tie(Types&... args) -> tuple<Types&...> {
    return {args...};
}

template <typename... Types>
auto forward_as_tuple(Types&&... args) -> typle<Types&&...> {
    return {std::forward<Types>(args)...};
}


template <typename... Ts>
struct X {
    explicit X(const Ts&... values) : values(std::tie(values...)) {}

    std::tuple<const Ts&...> values;
};

int main() {
    int i = 42;
    auto s = std::string("universe");

    auto x = X<int, std::string>(i, s);
    
    std::cout << "i = " << std::get<0>(x.values) << '\n';
    std::cout << "s = " << std::get<1>(x.values) << '\n';
}
