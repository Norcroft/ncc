static_assert(1, "file scope");
static_assert(sizeof(int) >= 2);

struct S {
    static_assert(sizeof(int) >= 2, "member");
    int value;
};

void f()
{
    static_assert(sizeof(S) >= sizeof(int), "block");
}
