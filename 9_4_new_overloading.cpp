#include <iostream>
#include <vector>

void* operator new(size_t n) {
    std::cout << n << " bytes allocated\n";
    return malloc(n);
}

void operator delete(void* ptr) {
    free(ptr);
}

void* operator new[](size_t n) {
    std::cout << n << "[] bytes allocated\n";
    return malloc(n);
}

void operator delete[](void* ptr) {
    free(ptr);
}

void* operator new(size_t n, int a, double b) {
    std::cout << n << " bytes allocated with custom new " << a << " " << b << "\n";
    return malloc(n);
}

void operator delete(void* ptr, int a, double b) {
    std::cout << "custom delete called" << a << ' ' << b << '\n';
    return free(ptr);
}

struct S {
    inline static int count = 0;

    void* operator new(size_t n) {
        std::cout << "operator new for S\n";
        return malloc(n);
    }
    void operator delete(void* ptr) {
        std::cout << "operator delete for S\n";
        return free(ptr);
    }


    S() {
        ++count;
        if (count == 5) {
            throw 1;
        }
        std::cout << "created S\n";
    }
    ~S() {
        std::cout << "destroyed S\n";
    }
};

struct Base {
    Base() { std::cout << "created Base\n"; }
    virtual ~Base() { std::cout << "destroyed Base\n"; }
};

struct Derived: Base {
    int* p;
    Derived() { 
        std::cout << "created Derived\n";
        p = new int;
    }
    ~Derived() { 
        std::cout << "destroyed Derived\n";
        delete p;
    }
};

int main() {

}
