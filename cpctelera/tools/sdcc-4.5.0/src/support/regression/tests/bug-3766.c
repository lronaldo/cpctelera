/** bug-3766.c: a bug in an optimization resulted in a wrong type on a struct member of 
    another struct when passed as function parameter, resulting in a unbalanced stack
    assertion being triggered in code generation when the two structs had different size,
*/

#include <testfwk.h>

typedef struct  {
  unsigned  len;
} Buf;

typedef struct  {
  Buf name;
  unsigned* dp; // Ensure that Search is bigger than Buf.
} Search;

int compareEntryName(Buf qName);

void lookupFunc(Search* s) {
  compareEntryName(s->name); // Bug triggered on this line.
}

void
testBug (void)
{
  Search s;
  s.name.len = 0x55aa;
}

int compareEntryName (Buf qName)
{
  ASSERT (qName.len == 0x55aa);
}

