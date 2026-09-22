#include <iostream>
#include <map>
#include <algorithm>


int main() {

    auto sum = [](int x, int y) { return x + y; };
    
    auto fix_second_argument = [](auto&& f, auto&& second_arg) {
        return [f = std::forward<decltype(f)>(f),
               second_arg = std::forward<decltype(second_arg)>(second_arg)]
                (auto&& first_arg) {
                return f(std::forward<decltype(first_arg)>(first_arg), second_arg);
        };
    };

    auto sum_with_five = fix_second_argument(sum, 5);

    std::cout << sum_with_five(3);
    
    auto print_hello = []() { std::cout << "Hello!\n"; };

    auto do_twice = [](auto&& f) {
        return [f = std::forward<decltype(f)>(f)](const auto&&... args) {
            f(args...);
            f(args...);
        };
    };

    auto print_hello_hello = do_twice(print_hello);

    print_hello_hello();

    

    auto fibonacci = []<typename T>(this T& self, int n) {
        return n > 2 ? self(n-1) + self(n-2) : 1;
    };


    std::cout << fibonacci(8);

}
