#include "gpsim_assert.h"

unsigned failures = 0;

void
done(void)
{
    ASSERT(MANGLE(failures) == 0);
    PASSED();
}

typedef void (void_void_f)(void);

void
foo(void) {
    failures--;
}

void
bar(void) {
    failures -= 2;
}

static void_void_f *
funcs[] = {
    &foo,
    &bar,
    (void *)0
};

void
main(void)
{
    void_void_f **ptr;
    failures = 3;

    ptr = &funcs[0];
    while (*ptr) {
        (**ptr)();
        ptr++;
    } // while

    done();
}

