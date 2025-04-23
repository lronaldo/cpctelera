/*
   punctuators.c

   punctuators test
*/

#include <testfwk.h>

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
