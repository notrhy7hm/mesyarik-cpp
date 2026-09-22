#include <iostream>
#include <cstdlib>
#include <exception>

int divide(int a, int b) {
    if (b == 0) {
        throw std::logic_error("Divide by zero");
    }
    return a / b;
}

int main() {
    try {
        new int[400'000'000'000];
    } catch (std::logic_error& err) {
        std::cout << err.what();
    }
}
