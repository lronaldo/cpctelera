/*
lto-tbaa-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

/* { dg-additional-options "-fno-early-inlining -fno-ipa-cp" }  */
struct a {
  float *b;
} *a;
struct b {
  int *b;
} b;
struct c {
  float *b;
} *c;
int d;
void use_a (struct a *a)
{
  (void)a;
}
void set_b (int **a)
{
  *a=&d;
}
void use_c (struct c *a)
{
  (void)a;
}
int **retme(int **val)
{
  return val;
}
int e;
struct b b= {&e};
struct b b2;
struct b b3;
int **ptr = &b2.b;
void
testTortureExecute (void)
{
  a= (void *)0;
  b.b=&e;
  ptr =retme ( &b.b);
  set_b (ptr);
  b3=b;
  ASSERT(b3.b == &d);
  c= (void *)0;
  return;
}
