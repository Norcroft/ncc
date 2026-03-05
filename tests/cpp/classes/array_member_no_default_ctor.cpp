// Regression test: array member whose element type lacks a default ctor.
//
// Previously caused a compiler segfault in structor_expr() when
// default_structor() returned NULL and its result was passed to
// mkunary(s_addrof).

class X { public: X(int); };
class A { X m_b[5]; };

// The following ought to be an error, at least for this specific case.
// CHECK-ERR: Warning: 'm_b': 'class X' has no default constructor
void fn() { A a; } // should be an error: A has no default constructor
