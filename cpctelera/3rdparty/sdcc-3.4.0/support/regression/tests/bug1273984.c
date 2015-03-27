/*  An assignment inside a functioncall changed the type of the parameter.
    See bug description 1273984 for details.

    Bug detected and fixed by Guenther Jehle

    sign: unsigned,
 */

#include <testfwk.h>

void foo({sign} int val) {
  val; //make the compiler happy
}

void fooInt({sign} int val) {
  ASSERT(val==3);
}

void fooChar({sign} char val) {
  ASSERT(val==6);
}

void
testAssignInFunctioncall(void)
{
  volatile {sign} char charVal=3;
  volatile {sign} int intVal=0x4040;

  fooInt(intVal=charVal); // should cast charVal to int for function call.
                          // without patch #1645121, a char is put on the stack
                          // (or hold in registers)
  foo(0xAAAA);
  fooInt(intVal=charVal);

  intVal=6;

  fooChar(charVal=intVal); // without patch, a int is put on the stack
  foo(0xAAAA);
  fooChar(charVal=intVal);

}


