/*
   bug2947189.c
 */

#include <testfwk.h>

typedef struct {
  unsigned int c : 1;
  unsigned int b : 2;
  unsigned int a : 3;
} TStruct;

TStruct s = {1, 2, 3};
unsigned char u = 3;

void testBug(void)
{
  unsigned char a = 0;

  // too small for a jumptable
  switch (s.c) {
    case 0:      a += 3;      break; 
    case 1:      a += 2;      break;    
  }

  // bug 2947189: jumptable need not check bounds and should not generate
  // warning 94: comparison is always true resp. false due to limited range of data type
  switch (s.b) {
    case 0:      a += 3;      break; 
    case 1:      a += 2;      break;    
    case 2:      a += 1;      break; 
    case 3:      a += 0;      break;
  }

  // bug 3069862: jumptable should not use signed comparison for bounds check
  switch (s.a) {
    case 0:      a += 4;      break;
    case 1:      a += 3;      break;  
    case 2:      a += 1;      break;
    case 3:      a += 5;      break;

    case 4:      a += 4;      break;
    case 5:      a += 3;      break;  
    case 6:      a += 1;      break;
    // no case 7:
  }

  switch (u) {
    case 0:      a += 4;      break;
    case 1:      a += 3;      break;  
    case 2:      a += 1;      break;
    case 3:      a += 5;      break;

    case 4:      a += 4;      break;
    case 5:      a += 3;      break;  
    case 6:      a += 1;      break;
    // no case 7:
  }

  ASSERT (a == (2 + 1 + 5 + 5));
}
