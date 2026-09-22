#include <iostream>

int main() {
    int* a = new int[10];
    char* ac = reinterpret_cast<char*>(a);
    ++ac;
    int* b = reinterpret_cast<int*>(ac);

    *b = 1;
    int x = *b;
    std::cout << x;

    delete[] a;
}
