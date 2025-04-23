/* bug-3795.c
   Triggered an assertion in optimization in zsdcc 4.4.0.
   (see also https://github.com/z88dk/z88dk/issues/2635)
*/

#include <testfwk.h>

#include <stdint.h>

uint8_t IntNorms[2][4] =  { {0x1F, 0x00, 0x3E, 0x1F},
                            {0x9F, 0x12, 0x37, 0x10},  };
float test_s;

void f(void)
{
    uint8_t i = 0;
    int16_t value16x;

    while(i < 2)
    {
        value16x = (int16_t)IntNorms[i][0];
        test_s = (float)value16x;
        ++i;
    }
}

void testBug(void)
{
    f();
    ASSERT(test_s == 0x9F);
}

