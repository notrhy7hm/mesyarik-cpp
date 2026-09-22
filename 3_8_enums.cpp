#include <iostream>

enum class E : int8_t {
    White = 2,
    Gray = 2,
    Black
};

int main() {
    E e = E::White;
    std::cout << static_cast<int>(e);
}
