#include <iostream>
#include <climits>

void f() {
    int *p = new int(5);
    std::cout << p << ' ' << *p << '\n';
    delete p;
}   

int main() {
    f();
    int x = INT_MAX;
    ++x;
    
}
