#include <iostream>
#include <cmath>
#include <vector>
struct Shape {
    virtual double area() const = 0;
    virtual ~Shape() = default;
};

struct Square: Shape {
    double a;
    Square(double a): a(a) {}
    double area() const override {
        return a * a;
    }
};

struct Circle: Shape {
    double r;
    Circle(double r): r(r) {}
    double area() const override {
        return M_PI * r * r;
    }
};

int main() {
    
    std::vector<Shape*> v;
    v.push_back(new Square(1.0));
    v.push_back(new Circle(1.0));

    for (Shape* s : v) {
        std::cout << s->area() << " ";
    }


}


