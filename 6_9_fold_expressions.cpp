#include <iostream>
#include <type_traits>

template <typename... Types>
struct all_pointers {
    static const bool value = (std::is_pointer_v<Types> && ...);
};

template <typename Head, typename... Tail>
struct is_homogeneous{
    static const bool value = (std::is_same_v<Head, Tail> && ...);
};

void print() {}

template <typename... Types>
void print (const Types&... types) {
    (std::cout << ... << types);
}

int main() {
    print(1, 2.0, "abc");
}
