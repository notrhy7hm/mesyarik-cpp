struct S {
 static const int n = 5;
};

int x = S::n + 1;

int foo(const int* x) { return *x; }

const int S::n;

int y = foo(&S::n) + 1;

int main() {}
