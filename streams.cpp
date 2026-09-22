#include <iostream>
#include <sstream>
#include <cassert>

int main() {
    std::string str("1 2 3 4 5");
    std::istringstream iss(str);

    int x;
    iss >> x;
    std::cout << x + 5;
}
