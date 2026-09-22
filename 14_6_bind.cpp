#include <iostream>
#include <functional>

int sum (int x, int y) {
    return x + y;
}

int diff (int x, int y) {
    return x - y;
}



int main() {
    auto f = std::bind(sum, 5, std::placeholders::_1);

    std::cout << f(3) << '\n';

    int x = 5;
    std::function<int(int)> g = std::bind(diff, std::placeholders::_1, std::ref(x));

    std::cout << g(3) << '\n';

    x = 1;
    std::cout << g(3) << '\n';


}
