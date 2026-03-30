// EXPECT-ERROR
// CHECK-ERR-NOT: Fatal error: Failure of internal consistency check
// CHECK-ERR-NOT: template symbol buffer lost
// CHECK-ERR-NOT: Missing class member function name
// CHECK-ERR-NOT: while instantiating 'class B'

template<typename T, typename U>
class O {
public:
    class A {
    public:
        class B {
        public:
            B() {}

            B operator++(int) { B old(*this); ++(*this); return old; }
        };
    };
};
