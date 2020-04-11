/*  Test assignment of one struct/union to another, including transitivity.

    type: char, int, long

 */
#include <testfwk.h>

/* declare some structs and unions */
struct t { {type} a; };
struct s { {type} a; {type} b; };

struct t t1 = { 1 };
struct t t2 = { 2 };

struct s s1 = { 1, 2 };
struct s s2 = { 3, 4 };
#ifndef __SDCC_pdk14 // Lack of memory
struct s s3 = { 5, 6 };

union u { {type} a; float b; char c; };

union u u1, u2, u3;
#endif

/* struct: simple assignment */
void
testSimpleStructAssignment(void)
{
  s1 = s2;
  ASSERT(s1.a == 3);
  ASSERT(s1.b == 4);

  t1 = t2;
  ASSERT(t1.a == 2);
}

/* struct: transitive assignment */
void
testTransitiveStructAssignment(void)
{
#ifndef __SDCC_pdk14 // Lack of memory
  s1 = s2 = s3;
  ASSERT(s1.a == 5);
  ASSERT(s1.b == 6);
  ASSERT(s2.a == 5);
  ASSERT(s2.b == 6);
#endif
}

/* union: simple assignment */
void
testSimpleUnionAssignment(void)
{
#ifndef __SDCC_pdk14 // Lack of memory
  u1.b = 0.0f;
  u2.b = -1.5f;
  u1 = u2;
  ASSERT(u1.b == -1.5f);
#endif
}

/* union: transitive assignment */
void
testTransitiveUnionAssignment(void)
{
#ifndef __SDCC_pdk14 // Lack of memory
  u1.b = 0.0f;
  u2.b = 0.1f;
  u3.b = -1.5f;
  u1 = u2 = u3;
  ASSERT(u1.b == -1.5f);
  ASSERT(u2.b == -1.5f);
#endif
}

/* test for unintended double evaluation */
void
testUnintendedDoubleEval(void)
{
#ifndef __SDCC_pdk14 // Lack of memory
  struct s ss[3];
  int n = 2;
  ss[2] = ss[--n] = ss[0];
  ASSERT(n == 1);
#endif
}
