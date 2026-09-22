#include <iostream>
#include <vector>

template <typename T>
struct Debug {
    Debug() = delete;
};

template <typename Container>
decltype(auto) getElement(Container& cont, size_t index) {
    decltype(auto) element = cont[index];
    return (element);
}

int main() {
    std::vector<bool> v(5);
    getElement(v, 0) = 1;
}
