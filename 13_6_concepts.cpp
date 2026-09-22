#include <iostream>
#include <vector>
#include <concepts>
#include <iterator>
#include <utility>

template <typename T>
concept InputIterator = requires(T it) {
    { ++it } -> std::same_as<T&>;
    typename std::iterator_traits<T>::value_type;
    { *it } -> std::convertible_to<
        typename std::iterator_traits<T>::value_type
    >;
};

auto my_find_if(InputIterator auto beg, decltype(beg) end,
            std::predicate<decltype(*beg)> auto p) {
    for (auto it = beg; it != end; ++it) {
        if (p(*it)) {
            return it;
        }
    }

    return end;
}

template <typename T, typename U>
requires (decltype(std::declval<T>() + std::declval<U>())(), true)
void add() {}

int main() {
    std::vector<int> v(10);

    my_find_if(v.begin(), v.end(), [](int x) { return x > 0; });
}
