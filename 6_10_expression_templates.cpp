#include <iostream>
#include <array>
#include <cassert>
#include <type_traits>

template <typename E>
class VecExpression {
public:
    static constexpr bool is_leaf = false;

    double operator[](size_t i) const {
        return static_cast<E const&>(*this)[i];
    }
    size_t size() const { return static_cast<E const&>(*this).size(); } 
};

class Vec : public VecExpression<Vec> {
    std::array<double, 3> elems;

public:
    static constexpr bool is_leaf = true;

    double operator[](size_t i) const { return elems[i]; }
    double& operator[](size_t i)      { return elems[i]; }
    size_t size()               const { return elems.size(); }

    Vec(std::initializer_list<double> init) {
        std::copy(init.begin(), init.end(), elems.begin());
    }

    template <typename E>
    Vec(VecExpression<E> const& expr) {
        for (size_t i = 0; i != expr.size(); ++i) {
            elems[i] = expr[i];
        }
    }
};

template <typename E1, typename E2>
class VecSum : public VecExpression<VecSum<E1, E2>> {
    std::conditional_t<E1::is_leaf, const E1&, const E1> _u;
    std::conditional_t<E2::is_leaf, const E2&, const E2> _v;

public:
    static constexpr bool is_leaf = false;

    VecSum(E1 const& u, E2 const& v) : _u(u), _v(v) {
        assert(u.size() == v.size());
    }
    double operator[](size_t i) const { return _u[i] + _v[i]; }
    size_t size()               const { return _v.size(); }
};

template <typename E1, typename E2>
VecSum<E1, E2>
operator+(VecExpression<E1> const& u, VecExpression<E2> const& v) {
    return VecSum<E1, E2>(*static_cast<const E1*>(&u), *static_cast<const E2*>(&v));
}

int main() {
    Vec v0 = {23.4, 12.5, 144.56};
    Vec v1 = {67.12, 34.8, 90.34};
    Vec v2 = {34.90, 111.9, 45.12};

    Vec sum_of_vec_type = v0 + v1 + v2;

    for (size_t i = 0; i < sum_of_vec_type.size(); ++i)
        std::cout << sum_of_vec_type[i] << std::endl;

    auto sum = v0 + v1 + v2;
    for (size_t i  = 0; i < sum.size(); ++i)
        std::cout << sum[i] << std::endl;
}


