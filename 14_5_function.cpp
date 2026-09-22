#include <iostream>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <cstddef>
#include <new>

int square(int x) {
    return x*x;
}   

struct AddOne {
    int operator()(int x) {
        return x + 1;
    }
};

struct Strange {
    int moduloSeven(int n) {
        return n % 7;
    }
};

template <typename T>
class function;

template <typename Ret, typename... Args>
class function<Ret(Args...)> {
private:
    struct Base {
        virtual Ret call(Args...) = 0;
        virtual ~Base() = default;
    };

    template <typename F>
    struct Derived: Base {
        F f;
        ~Derived() override = default;
        Derived(const F& f): f(f) {}
        Derived(F&& f): f(std::move(f)) {}

        Ret call(Args... args) override {
            if constexpr(!std::is_member_function_pointer_v<F>) {
                // TODO
            } else if constexpr (std::is_member_object_pointer_v<F>) {
                // TODO
            } else {
                return f(std::forward<Args>(args)...);
            }
        }
    };
private:
    static const size_t BUFFER_SIZE = 16;
    void* fptr;
    alignas(std::max_align_t) char buffer[BUFFER_SIZE];
    
    using invoke_ptr_t = Ret(*)(void*, Args...);
    using destroy_ptr_t = void(*)(void*);


    invoke_ptr_t invoke_ptr;
    destroy_ptr_t destroy_ptr;

public:
    template <typename F>
    static Ret invoker(void* ptr, Args... args) {
        F* fptr = static_cast<F*>(ptr);
        return (*fptr)(std::forward<Args>(args)...);
    }
    
    template <typename F>
    static void destroyer(void* ptr) {
        F* fptr = static_cast<F*>(ptr);

        if constexpr (sizeof(F) > BUFFER_SIZE) {
            delete fptr;
        } else {
            fptr->~F();
        }
    }

    template <typename F>
    function(const F& func)
        : invoke_ptr(&invoker<F>) 
        , destroy_ptr(&destroyer<F>)
    {
        if constexpr (sizeof(F) > BUFFER_SIZE) {
            fptr = new F(func);
        } else {
            new (buffer) F(func);
            fptr = buffer;
        }
    }
    
    ~function() {
        destroy_ptr(fptr);
    }

    Ret operator()(Args... args) const {
        return invoke_ptr(fptr, std::forward<Args>(args)...);    
        // return fptr->call(std::forward<Args>(args)...);
    }
};

struct NotCopyable {
    std::unique_ptr<int> p { new int(5) };
    void operator()(int x) {
        return x + * p;
    }
};
 

int main() {

    std::function<int(int)> f = NotCopyable();
    f(5);

}
