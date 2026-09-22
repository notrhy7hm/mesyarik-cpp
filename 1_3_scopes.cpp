#include <iostream>

// Declarations, definitions, scopes

int a;

namespace N {
    int x;
}

int main() {
    using N::x;
    std::cout << x << std::endl;
}
