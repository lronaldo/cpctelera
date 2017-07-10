/* bug1520966 "Floating-point bug with small numbers"
 */

#include <testfwk.h>
#include <math.h>

void
testFloatOp(void)
{
    volatile float a,b;
    float result;

    a = 2.04e-38;
    b = 0.01;
    result = a * b;
    ASSERT( result < a );
    ASSERT( result >= 0.0 );

    b = 100;
    result = a / b;
    ASSERT( result < a );
    ASSERT( result >= 0.0 );
}
