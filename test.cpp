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
    bar b;

    static_delegate food(foo);
    static_delegate bizd(biz);
    member_delegate memd(&bar::baz, &b);

    static_assert(std::is_same_v<decltype(food), static_delegate<int>>);
    static_assert(std::is_same_v<decltype(bizd), static_delegate<int, int>>);
    static_assert(std::is_same_v<decltype(memd), member_delegate<int>>);

    printf("food() == %d\n", food());
    printf("bizd(10) == %d\n", bizd(10));
    printf("memd() == %d\n", memd());
}
