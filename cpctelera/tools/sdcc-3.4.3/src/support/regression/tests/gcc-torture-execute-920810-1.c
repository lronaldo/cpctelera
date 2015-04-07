/*
   920810-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct { void* super; int name; int size; } t;

t* f(t*clas, int size)
{
  t* child = (t*)malloc(size);
  memcpy(child, clas, clas->size);
  child->super=clas;
  child->name=0;
  child->size=size;
  return child;
}

void
testTortureExecute (void)
{
  t foo, *bar;
  memset(&foo, 37, sizeof(t));
  foo.size=sizeof(t);
  bar=f(&foo,sizeof(t));
  ASSERTFALSE (bar->super!=&foo||bar->name!=0||bar->size!=sizeof(t));
  return;
}
