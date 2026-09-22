#include <iostream>
#include <map>
#include <vector>

template <typename T>
void swap(T x, T y) {
    T t = x;
    x = y;
    y = t;
}

template <typename T>
T max(T x, T y) {
    return x > y ? x : y;
}

template <typename T>
class Vector {
    T* arr;
    size_t sz;
    size_t cap;
};

template <typename T>
struct less {
    bool operator()(const T& x, const T& y) const {
        return x < y;
    }
};

template <typename T>
using mymap = std::map<T, T, std::greater<T>>;

mymap<int> m;

int main() {
    int a = 0;
    long long b = 1;
    swap<long long>(a, b);
    
    std::vector<int> v;
    std::vector<double> v2 = v;

}

