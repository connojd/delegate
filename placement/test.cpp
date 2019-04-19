#include "delegate.h"
#include <iostream>
#include <typeinfo>
#include <unistd.h>
#include <cstdio>

int foo()
{
    return 1;
}

int biz(int n)
{
    return rand() * rand() * n;
}

struct bar {
    bar() : val{rand() % 8} {}
    int baz() { return rand() % 2; }
    int fiz() const { return val + (rand() % 2); }
    int val;
};

struct bell : public bar {
    bell() : bar() {}
    int f0() { return rand() % 2; }
    int f1() const { return val + (rand() % 2); }
    double val;
};

int main()
{
    bar b;
    bell h;

    const bar c;

    delegate bizd(&biz);
    delegate food(&foo);
    delegate bazd(&bar::baz, &b);
    delegate beld(&bell::f0, &h);

    auto bizc = bizd;
    delegate fizd(&bar::fiz, &b);

    delegate cizd(&bar::fiz, &c);
    auto cizm = std::move(cizd);

    static_assert(std::is_same_v<decltype(bizd), delegate<int, int>>);
    static_assert(std::is_same_v<decltype(bizd), decltype(bizc)>);
    static_assert(std::is_same_v<decltype(food), delegate<int>>);
    static_assert(std::is_same_v<decltype(bazd), delegate<int>>);
    static_assert(std::is_same_v<decltype(fizd), delegate<int>>);
    static_assert(std::is_same_v<decltype(cizm), delegate<int>>);

    printf("bizd(2) == %d, sizeof == %lu\n", bizd(2), sizeof(bizd));
    printf("bizc(2) == %d, sizeof == %lu\n", bizc(2), sizeof(bizc));
    printf("food() == %d, sizeof == %lu\n", food(), sizeof(food));
    printf("bazd() == %d, sizeof == %lu\n", bazd(), sizeof(bazd));
    printf("beld() == %d, sizeof == %lu\n", beld(), sizeof(beld));
    printf("fizd() == %d, sizeof == %lu\n", fizd(), sizeof(fizd));
    printf("cizm() == %d, sizeof == %lu\n", cizm(), sizeof(cizm));
}
