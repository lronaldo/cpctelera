/*
   preproc.c

   preprocessor and punctuators tests
*/

#include <testfwk.h>

/*
 * the following tests are applicable only for SDCC,
 * since they are using SDCC specific pragmas
 */
#ifdef __SDCC
/*
 * test for bug 135170
 */
const char *
const_str(void)
{
return "A"



/* just for testing, actually nop */
#pragma save





"B";
#pragma restore
}

/*
 * test for bug 982435
 */
#pragma pedantic_parse_number +

#define LO_B(x) ((x) & 0xff)
#define BAD(x) ((x) & 0xff)

unsigned char
hexeminus(void)
{
unsigned char a=0x\
fe\
-\
LO_B(3);

unsigned char b=0xfe-BAD(3);

return (a + b);
}
#endif

/*
 * test punctuators, as defined in ISO/IEC 9899:1999, chapter 6.4.6#3
 */
void
testPunctuators (void)
<%
%:define N        10
%:define VAL      123
%:define C(NAME)  NAME %:%: ___
  int C(a)<:N:>;

  C(a)<:0:> = VAL;
  ASSERT (a___<:0:> == VAL);
%>
