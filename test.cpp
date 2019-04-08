#include "bfdelegate.h"
#include <iostream>
#include <typeinfo>

int foo(void)
{
    return 1;
}

int biz(int n)
{
    return n * n;
}

struct bar {
    int baz(void) { return 42; }
};

int main()
{
    delegate food(foo);
    delegate bizd(biz);

    //static_assert(std::is_same_v<decltype(food), delegate<int>>);
    //static_assert(std::is_same_v<decltype(bizd), delegate<int, int>>);

    printf("food() == %d\n", food());
    printf("bizd(10) == %d\n", bizd(10));
}
