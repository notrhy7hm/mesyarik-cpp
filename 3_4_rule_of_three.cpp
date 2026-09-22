#include <iostream>
#include <cstring>
#include <algorithm>
#include <initializer_list>

class String {
    char* arr = nullptr;
    size_t sz = 0;
    size_t cp = 0;
private:
    String(size_t n): arr(new char[n+1]), sz(n), cp(n+1) {
        arr[sz] = '\0';
    }
public:
    String() = default;

    String(size_t n, char c): String(n) {
        memset(arr, c, n);
    }

    String(std::initializer_list<char> list): String(list.size()) {
        std::copy(list.begin(), list.end(), arr);
    }

    String(const String& other): String(other.sz) {
        memcpy(arr, other.arr, sz);
    }
    // copy and swap idiom
    String& operator=(String other) {
        swap(other);
        return *this;
    }

    void swap(String& other) {
        std::swap(arr, other.arr);
        std::swap(sz, other.sz);
        std::swap(cp, other.cp);
    }

    ~String() {
        delete[] arr;
    }
};

int main() {
    String s = {'a', 'b', 'c', 'd'};
    String s2(2, 'a');
    // String s3;

    // String s4 = s;
}
