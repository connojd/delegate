#include "delegate.h"
#include <iostream>
#include <typeinfo>
#include <unistd.h>
#include <cstdio>
#include <mcheck.h>

int foo()
{
    return 1;
}

int biz(int n)
{
    return n * n;
}

struct bar {
    bar() : val{rand() % 8} {}
    int baz() { return rand() % 2; }
    int fiz() const { return val + (rand() % 2); }
    int val;
};

int main()
{
    mtrace();

    srand(time(NULL));

    bar b;

    delegate bizd(&biz);
    delegate food(&foo);
    delegate bazd(&bar::baz, &b);
    delegate fizd(&bar::fiz, &b);

    static_assert(std::is_same_v<decltype(bizd), delegate<int, int>>);
    static_assert(std::is_same_v<decltype(food), delegate<int>>);
    static_assert(std::is_same_v<decltype(bazd), delegate<int>>);
    static_assert(std::is_same_v<decltype(fizd), delegate<int>>);

    printf("bizd(2) == %d, sizeof == %d\n", bizd(2), sizeof(bizd));
    printf("food() == %d, sizeof == %d\n", food(), sizeof(food));
    printf("bazd() == %d, sizeof == %d\n", bazd(), sizeof(bazd));
    printf("fizd() == %d, sizeof == %d\n", fizd(), sizeof(fizd));
}
