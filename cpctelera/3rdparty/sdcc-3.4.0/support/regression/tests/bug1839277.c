/*
    bug 1839277 & 1839299
*/

#include <testfwk.h>

const __code struct Value {
  const __code char* Name[2];
} Values[2]= {{{"abc", "def"}}, {{"ghi", "jkl"}}};

char i = 1;

void
testBug1839277 (void)
{
  const char __code* const * volatile p;
  unsigned long v = 0;
//first subexpression 'Values[0].Name' is evaluted as follows:
//mov     r2,#_Values
//mov     r3,#(_Values >> 8)
//mov     r4,#(_Values >> 16) ;this is wrong - should be 'mov r4,#128' shouldn't it?
//second subexpression 'Values[1].Name' is evaluted as follows:
//mov     a,#0x04
//add     a,#_Values
//mov     r2,a
//clr     a
//addc    a,#(_Values >> 8)
//mov     r3,a
//mov     r4,#128 ;this is all right
  p = i ? Values[0].Name : Values[1].Name;
#if defined(SDCC_mcs51)
  v = (unsigned long)p;
  ASSERT ((unsigned char)(v >> 16) == 0x80);
#endif

//everything is all right with explicit typecast - but why do I need it?
  p = i ? (const char __code* const *)Values[0].Name : (const char __code* const *)Values[1].Name;
#if defined(SDCC_mcs51)
  v = (unsigned long)p;
  ASSERT ((unsigned char)(v >> 16) == 0x80);
#endif

//this is the best/optimal version - again with explicit typecast
//Question: Why is it necessary to have explicit typecast to make things right?
  p = i ? (const char __code* const __code*)Values[0].Name : (const char __code* const __code*)Values[1].Name;
#if defined(SDCC_mcs51)
  v = (unsigned long)p;
  ASSERT ((unsigned char)(v >> 16) == 0x80);
#endif
}

void
testBug1839299 (void)
{
  const char __code* const * volatile p;
  unsigned long v = 0;
//'Values[0].Name' subexpression is evaluated as follows first:
//mov     r2,#_Values
//mov     r3,#(_Values >> 8)
//mov     r4,#(_Values >> 16) ;this is wrong - see bug 1839277
  p = i ? Values[0].Name : Values[1].Name;
//this assignment has some sideeffect on the following one
//in fact it is the evaluation of 'Values[0].Name' itself has the effect, not the assignment
  p = Values[0].Name;
//'Values[0].Name' subexpression is evaluated as follows second:
//mov     r2,#_Values
//mov     r3,#(_Values >> 8)
//mov     r4,#0x00 ;this is different from first occurrence but also wrong
  p = i ? Values[0].Name : Values[1].Name;
#if defined(SDCC_mcs51)
  v = (unsigned long)p;
  ASSERT ((unsigned char)(v >> 16) == 0x80);
#endif
}
