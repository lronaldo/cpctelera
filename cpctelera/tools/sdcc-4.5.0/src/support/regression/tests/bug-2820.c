/* bug-2820.c
   Redundancy elimination ignored the difference between integer and floating-point arithmetic operations.
 */

#include <testfwk.h>

#include <stdint.h>

typedef union
{
    int32_t i;
    float f;
} intflt;

#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
void sum(int8_t tp, intflt *if1, intflt *if2)
{
    if(tp)
        (*if1).f *= (*if2).f;
    else
        (*if1).i *= (*if2).i;
}

void sum2(int8_t tp, intflt *if1, intflt *if2)
{
    if(tp)
        (*if1).f += (*if2).f;
    else
        (*if1).i += (*if2).i;
}
#endif

void testBug(void)
{
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
	intflt data;

	data.i = -23;
	sum(0, &data, &data);
	ASSERT(data.i == -23 * -23);

	data.f = -23.0f;
	sum(1, &data, &data);
	ASSERT(data.f == -23.0f * -23.0f);

	data.i = -23;
	sum2(0, &data, &data);
	ASSERT(data.i == -23 + -23);

	data.f = -23.0f;
	sum2(1, &data, &data);
	ASSERT(data.f == -23.0f + -23.0f);
#endif
}

