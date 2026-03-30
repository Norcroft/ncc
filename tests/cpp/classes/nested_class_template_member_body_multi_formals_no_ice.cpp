// EXPECT-ERROR
// CHECK-ERR-NOT: Fatal error: Failure of internal consistency check
// CHECK-ERR-NOT: template symbol buffer lost
// CHECK-ERR-NOT: Missing class member function name
// CHECK-ERR-NOT: while instantiating 'class A'

template<typename T, typename U>
class O {
public:
    class A {
    public:
        A() {}

        A operator++(int) { A old(*this); ++(*this); return old; }
    };
};
