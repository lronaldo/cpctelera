/** bug-3740.c : A bug in code generation for storing 0 into global vairables.
*/

#include <testfwk.h>

#pragma disable_warning 85

unsigned char some_param;
unsigned char some_var;

unsigned char some_param;
unsigned char some_var;
unsigned char some_func(unsigned char value);

void f(void) {
    unsigned char some_return_value = some_func(some_param);
    some_var = 0;
    if (some_return_value == 0x10) {
        some_var = 1;
    }
}


void
testBug(void)
{
    f();
    ASSERT (some_var);
}

unsigned char some_func(unsigned char value)
{
    return 0x10;
}

