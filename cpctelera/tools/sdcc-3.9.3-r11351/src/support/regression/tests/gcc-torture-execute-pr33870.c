/*
   pr33870.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <string.h>

typedef struct PgHdr PgHdr;
typedef unsigned char u8;
struct PgHdr {
  unsigned int pgno;
  PgHdr *pNextHash, *pPrevHash;
  PgHdr *pNextFree, *pPrevFree;
  PgHdr *pNextAll;
  u8 inJournal;
  short int nRef;
  PgHdr *pDirty, *pPrevDirty;
  unsigned int notUsed;
};

#if !defined(__SDCC_pdk14) && !defined (__SDCC_pdk15) // Bug #2874
#ifndef __SDCC_mcs51
static inline PgHdr *merge_pagelist(PgHdr *pA, PgHdr *pB)
{
  PgHdr result;
  PgHdr *pTail;
  pTail = &result;
  while( pA && pB ){
    if( pA->pgno<pB->pgno ){
      pTail->pDirty = pA;
      pTail = pA;
      pA = pA->pDirty;
    }else{
      pTail->pDirty = pB;
      pTail = pB;
      pB = pB->pDirty;
    }
  }
  if( pA ){
    pTail->pDirty = pA;
  }else if( pB ){
    pTail->pDirty = pB;
  }else{
    pTail->pDirty = 0;
  }
  return result.pDirty;
}

PgHdr *sort_pagelist(PgHdr *pIn)
{
  PgHdr *a[25], *p;
  int i;
  memset (a, 0, sizeof (a));
  while( pIn ){
    p = pIn;
    pIn = p->pDirty;
    p->pDirty = 0;
    for(i=0; i<25 -1; i++){
      if( a[i]==0 ){
        a[i] = p;
        break;
      }else{
        p = merge_pagelist(a[i], p);
        a[i] = 0;
      }
    }
    if( i==25 -1 ){
      a[i] = merge_pagelist(a[i], p);
    }
  }
  p = a[0];
  for(i=1; i<25; i++){
    p = merge_pagelist (p, a[i]);
  }
  return p;
}
#endif
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_pdk14) && !defined (__SDCC_pdk15) // Bug #2874
#ifndef __SDCC_mcs51
  PgHdr a[5];
  PgHdr *p;
  a[0].pgno = 5;
  a[0].pDirty = &a[1];
  a[1].pgno = 4;
  a[1].pDirty = &a[2];
  a[2].pgno = 1;
  a[2].pDirty = &a[3];
  a[3].pgno = 3;
  a[3].pDirty = 0;
  p = sort_pagelist (&a[0]);
  if (p->pDirty == p)
    ASSERT (0);
  return;
#endif
#endif
}

