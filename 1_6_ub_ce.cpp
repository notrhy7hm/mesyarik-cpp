#include <iostream>
#include <vector>
// 1.6 CE, RE, UB


int main() {
    std::vector<int> v(10);
    v[10] = 1;
}
