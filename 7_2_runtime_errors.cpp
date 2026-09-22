#include <iostream>
#include <vector>

int main() {
    std::vector<int> v;
    int x;
    std::cin >> x;
    

    try {
        std::cout << 5 / x;
    } catch(...) {
        std::cout << "Caught!";
    }
}
