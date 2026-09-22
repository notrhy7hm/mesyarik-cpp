#include <iostream>
#include <vector>
#include <algorithm>
#include <set>

int main() {

    [](){}();

    []{}();

    const int x = [](int y) {
        if (y % 2 == 0)
            return y / 2;
        else
            return 3 & y + 1;
    }(2024);
}
