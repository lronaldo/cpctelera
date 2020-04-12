/*  Make sure global common subexpression optimization does not
    inappropriately cache values across function calls.

    sign1: signed, unsigned
    sign2: signed, unsigned
    type: int

 */
#include <testfwk.h>

typedef struct
{
  {sign1} {type} field1;
  {sign1} {type} *field2;
  {sign1} {type} field3;
} struct1;
  

void
spoil({sign1} {type} val)
{
  UNUSED(val);
}

void
inc1({sign1} {type} *valptr)
{
  (*valptr)++;
}

{sign2}
gcse1({sign2} {type} target)
{
  spoil(target);
  inc1(&target);
  return target;
}


void
inc2(struct1 *s)
{
  (*s->field2)++;
}

{sign2}
gcse2({sign2} {type} target)
{
  struct1 s;
  
  s.field2 = &s.field3;
  *s.field2 = target;
  spoil(s.field3);
  inc2(&s);
  return s.field3;
}


void
testgcse(void)
{
  ASSERT(gcse1(1)==2);
  ASSERT(gcse2(1)==2);
}
