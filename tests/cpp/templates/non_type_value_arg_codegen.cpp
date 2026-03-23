// RUN: %cxx -c %s
// CHECK-ERR-NOT: Fatal error:
// CHECK-ERR-NOT: Serious error:

// Exercises constant-folded non-type template arguments whose intorig_()
// still points at a cast-shaped expression.

template<class T, T N>
struct Box {
    enum { value = N };
};

Box<unsigned, 1> a;
Box<unsigned, (int)1> b;
Box<unsigned, 1 + 0> c;
