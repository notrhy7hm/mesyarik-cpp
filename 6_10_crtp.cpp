

template <class T>
struct Base {
    void interface() {
        static_cast<T*>(this)->implementation();
    }

    static void static_func() {
        T::static_sub_func();
    }
};

struct Derived : Base<Derived> {
    void implementation();
    static void statuc_sub_func();
};

int main() {}
