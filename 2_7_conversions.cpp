#include <iostream>
#include <string>

int main() {
    int x = 0;
    static_cast<double>(x);

    // long long y = 1729;
    // double & d = reinterpret_cast<double&>(y); 
    // d = 3.14;
    // std::cout << y;
    
    // int * p = &x;
    // std::string* str = reinterpret_cast<std::string*>(p);    
    // std::cout << *str;

    /* const int c = 5;
    int& cc = const_cast<int&>(c);
    cc = 7;
    std::cout << c << ' ' << cc; */

    double d = (double) x;
}
