#include "delegate.h"
#include <iostream>
#include <typeinfo>
#include <unistd.h>

int foo()
{
    return 1;
}

int biz(int n)
{
    return n * n;
}

struct bar {
    int baz() { return 42; }
    int fiz() const { return rand(); }
};

int main()
{
    bar b;

    static_delegate food(&foo);
    static_delegate bizd(&biz);
    member_delegate bazd(&bar::baz, &b);
    member_delegate fizd(&bar::fiz, &b);

    printf("food() == %d\n", food());
    printf("bizd(10) == %d\n", bizd(10));
    printf("memd() == %d\n", bazd());
    printf("fizd() == %d\n", fizd());
}
