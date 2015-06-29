/*
   pr39100.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 85
#endif

/* Bad PTA results (incorrect store handling) was causing us to delete
   *na = 0 store.  */

typedef struct E
{
  int p;
  struct E *n;
} *EP;   

typedef struct C
{
  EP x;
  short cn, cp; 
} *CP;

CP
foo (CP h, EP x)
{
  EP pl = 0, *pa = &pl;
  EP nl = 0, *na = &nl;
  EP n;

  while (x)
    {
      n = x->n;   
      if ((x->p & 1) == 1) 
        {
          h->cp++;
          *pa = x;
          pa = &((*pa)->n);
        }
      else
        {
          h->cn++;
          *na = x;
          na = &((*na)->n);
        }    
      x = n;
    }
  *pa = nl;
  *na = 0;
  h->x = pl;
  return h;
}

void
testTortureExecute (void)
{  
  struct C c = { 0, 0, 0 };
  struct E e[2] = { { 0, &e[1] }, { 1, 0 } };
  EP p;

  foo (&c, &e[0]);
  if (c.cn != 1 || c.cp != 1)
    ASSERT (0);
  if (c.x != &e[1])
    ASSERT (0);
  if (e[1].n != &e[0])
    ASSERT (0);
  if (e[0].n)
    ASSERT (0);
  return;  
}


