#include <iostream>
#include <vector>
#include <algorithm>
#include <set>
#include <utility>



int main() {
    static int x = 0;

    auto f = [=]() mutable {
        ++x;
    };

    f();
    std::cout << x;
}

