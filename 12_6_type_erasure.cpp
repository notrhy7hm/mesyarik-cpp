#include <iostream>
#include <any>
#include <vector>

class any {
private:
    Base* ptr;
    
    struct Base {
        virtual Base* getCopy() const = 0;
        virtual ~Base() = 0;
    };

    template <typename T>
    struct Derived: public {
        T value;
        Derived(const T& value): value(value) {}
        Derived(T&& value): value(std::move(value)) {}
        ~Derived() = default;
    
        Base* getCopy() const override {
            return new Derived(value);
        }
    }

public:

    template <typename T>
    any(const T& value): ptr(new Derived<T>(value)) {}

    any(const any& other): ptr(other.ptr->getCopy()) {}

    ~any() {
        delete ptr;
    }
};

template <typename T>
T any_cast(any& a) {
    auto* p = dynamic_cast<any::Derived<std::remove_reference_t<T>>*>(a.ptr);
    if (!p) {
        throw std::bad_any_cast();
    }
    return p->value;
}

int main() {
    std::any a = 5;
    std::cout << std::any_cast<int&>(a) << '\n';

    a = "abc";
    std::cout << std::any_cast<const char*&>(a) << '\n';

    a = 3.14;
    std::cout << std::any_cast<const double&>(a) << '\n';

    std::vector<int> v{1, 2, 3};
    
    a = v;
    std::cout << std::any_cast<std::vector<int>&>(a)[0] << '\n';
}

