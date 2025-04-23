/* bug-716242.c

   syntax tests about function pointers at compile time

   This also tests an implementation extension that allows the casting of void * to function pointers,
   which is not available for the stm8 large memory model.
 */
#include <testfwk.h>

#pragma disable_warning 244

#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // pdk needs functions called via pointer to be reentrant even for a single argument
const void *p;
int ret;

int
mul2 (int i)
{
  return 2 * i;
}

void
g (int (*h) (int))
{
  ret = h (2);
}

void
f1 ()
{
#if !((defined __SDCC_stm8) && defined (__SDCC_MODEL_LARGE))
#if defined(__SDCC_ds390)
  p = (void __code *) mul2;
#else
  p = (void *) mul2;
#endif
  g ((int (*) (int)) p);
#endif
}

/****************************/

void g (int (*h) (int));

void
f2 ()
{
  int (*fp) (int) = p;

  g (fp);
}

/****************************/

void g (int (*h) (int));

void
f3 ()
{
  int (*fp) (int) = (int (*) (int)) p;

  g (fp);
}

/****************************/

void
f4 ()
{
  ((void (__code *) (void)) p) ();
}

/****************************/

void
f5 ()
{
  int (*fp) (int) = mul2;

  fp (1);
}

/****************************/

void
f6 ()
{
  ((void (__code *) (void)) 0) ();
}

/****************************/
#endif

static void
testFuncPtr (void)
{
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15)
#if !((defined __SDCC_stm8) && defined (__SDCC_MODEL_LARGE)) // STM8 large model has sizeof(void *) != size of function pointers.
  f1 ();
  ASSERT (ret == 4);
#endif
#endif
}
