_Static_assert(1, "file scope");
_Static_assert(sizeof(int) >= 2);

struct S {
    _Static_assert(sizeof(int) >= 2, "member");
    int value;
};

void f(void)
{
    _Static_assert(sizeof(struct S) >= sizeof(int), "block");
}
