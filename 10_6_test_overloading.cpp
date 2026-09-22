#include <iostream>

template <typename T>
class Optional {
public:
    Optional() = default;

    Optional(const Optional&) {
        std::cout << "A" << std::endl;
    }

    template <typename U>
    Optional(Optional<U>&&) {
        std::cout << "B" << std::endl;
    }

    template <typename U>
    Optional(U&&) {
        std::cout << "C" << std::endl;
    }
};

int main() {
    Optional<int> a;
    Optional<int> b(a);
}
