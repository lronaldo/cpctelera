/*-----------------------------------------------------------------
    SDCCbitv.c - contains support routines for bitvectors

    Written By - Sandeep Dutta . sandeep.dutta@usa.net (1998)

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2, or (at your option) any
    later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

    In other words, you are welcome to use, share and improve this program.
    You are forbidden to forbid anyone else to use, share and improve
    what you give them.   Help stamp out software-hoarding!
-------------------------------------------------------------------------*/

#include "common.h"

#include "newalloc.h"

int bitVectDefault = 1024;

#define BYTE_SIZEOF_ELEMENT (sizeof(unsigned int))
#define BIT_SIZEOF_ELEMENT (BYTE_SIZEOF_ELEMENT*8)

/* genernal note about a bitvectors:
   bit positions must start from 0 */
/*-----------------------------------------------------------------*/
/* newBitVect - returns a new bitvector of size                    */
/*-----------------------------------------------------------------*/
bitVect *
newBitVect (int size)
{
  bitVect *bvp;

  bvp = Safe_calloc (1, sizeof (bitVect));

  bvp->size = size;
  bvp->allocSize = (size+BIT_SIZEOF_ELEMENT-1) / BIT_SIZEOF_ELEMENT;
  bvp->vect = Safe_calloc (BYTE_SIZEOF_ELEMENT, bvp->allocSize);
  return bvp;
}


/*-----------------------------------------------------------------*/
/* freeBitVect - frees the memory used by the bitVector            */
/*-----------------------------------------------------------------*/
void
freeBitVect (bitVect * bvp)
{
  if (!bvp)
    return;

  Safe_free (bvp->vect);
  Safe_free (bvp);
}

/*-----------------------------------------------------------------*/
/* bitVectResize - changes the size of a bit vector                */
/*-----------------------------------------------------------------*/
bitVect *
bitVectResize (bitVect * bvp, int size)
{
  int allocSize;

  if (!bvp)
    return newBitVect (size);

  allocSize = (size+BIT_SIZEOF_ELEMENT-1) / BIT_SIZEOF_ELEMENT;

  /* if we already have enough space */
  if (bvp->allocSize >= allocSize)
    {
      if (size > bvp->size)
	      bvp->size = size;
      return bvp;
    }

  bvp->vect = Clear_realloc (bvp->vect, bvp->allocSize*BYTE_SIZEOF_ELEMENT, allocSize*BYTE_SIZEOF_ELEMENT);
  bvp->size = size;
  bvp->allocSize = allocSize;

  return bvp;
}

/*-----------------------------------------------------------------*/
/* bitVectSetBit - sets a given bit in the bit vector              */
/*-----------------------------------------------------------------*/
bitVect *
bitVectSetBit (bitVect * bvp, int pos)
{
  unsigned int index;
  unsigned int bitofs;

  assert (pos>=0);
  /* if set is null then allocate it */
  if (!bvp)
    bvp = newBitVect (bitVectDefault);	/* allocate for twice the size */

  if (bvp->size <= pos)
    bvp = bitVectResize (bvp, pos + 2);		/* conservatively resize */

  index = pos / BIT_SIZEOF_ELEMENT;
  bitofs = pos % BIT_SIZEOF_ELEMENT;
  bvp->vect[index] |= 1u << bitofs;
  return bvp;
}

/*-----------------------------------------------------------------*/
/* bitVectUnSetBit - unsets the value of a bit in a bitvector      */
/*-----------------------------------------------------------------*/
void 
bitVectUnSetBit (const bitVect *bvp, int pos)
{
  unsigned int index;
  unsigned int bitofs;

  assert (pos>=0);
  if (!bvp)
    return;

  if (bvp->size <= pos)
    return;

  index = pos / BIT_SIZEOF_ELEMENT;
  bitofs = pos % BIT_SIZEOF_ELEMENT;
  bvp->vect[index] &= ~(1u << bitofs);
}

/*-----------------------------------------------------------------*/
/* bitVectBitValue - returns value value at bit position           */
/*-----------------------------------------------------------------*/
int 
bitVectBitValue (const bitVect *bvp, int pos)
{
  unsigned int index;
  unsigned int bitofs;

  assert (pos>=0);
  if (!bvp)
    return 0;

  index = pos / BIT_SIZEOF_ELEMENT;
  bitofs = pos % BIT_SIZEOF_ELEMENT;

  if (bvp->size <= pos)
    return 0;


  return (bvp->vect[index] >> bitofs) & 1;
}

/*-----------------------------------------------------------------*/
/* bitVectUnion - unions two bitvectors                            */
/*-----------------------------------------------------------------*/
bitVect *
bitVectUnion (bitVect * bvp1, bitVect * bvp2)
{
  bitVect *newBvp;
  unsigned int *pn, *p1, *p2;
  int elements;

  /* if both null */
  if (!bvp1 && !bvp2)
    return NULL;

  /* if one of them null then return the other */
  if (!bvp1 && bvp2)
    return bitVectCopy (bvp2);

  if (bvp1 && !bvp2)
    return bitVectCopy (bvp1);

  /* if they are not the same size */
  /* make them the same size */
  if (bvp1->size < bvp2->size)
    bvp1 = bitVectResize (bvp1, bvp2->size);
  else if (bvp2->size < bvp1->size)
    bvp2 = bitVectResize (bvp2, bvp1->size);

  newBvp = newBitVect (bvp1->size);
  elements = bvp1->allocSize;

  pn = newBvp->vect; 
  p1 = bvp1->vect;
  p2 = bvp2->vect;

  while (elements--)
    {
      *pn++ = *p1++ | *p2++;
    }

  return newBvp;
}

/*-----------------------------------------------------------------*/
/* bitVectInplaceUnion - unions two bitvectors                     */
/*-----------------------------------------------------------------*/
bitVect *
bitVectInplaceUnion (bitVect * bvp1, bitVect * bvp2)
{
  unsigned int *p1, *p2;
  int elements;

  /* if both null */
  if (!bvp1 && !bvp2)
    return NULL;

  /* if one of them null then return the other */
  if (!bvp1 && bvp2)
    return bitVectCopy (bvp2);

  if (bvp1 && !bvp2)
    return bvp1;

  /* if they are not the same size */
  /* make them the same size */
  if (bvp1->size < bvp2->size)
    bvp1 = bitVectResize (bvp1, bvp2->size);
  else if (bvp2->size < bvp1->size)
    bvp2 = bitVectResize (bvp2, bvp1->size);

  elements = bvp1->allocSize;

  p1 = bvp1->vect;
  p2 = bvp2->vect;

  while (elements--)
    {
      *p1 |= *p2;
      p1++; p2++;
    }

  return bvp1;
}


/*-----------------------------------------------------------------*/
/* bitVectIntersect - intersect  two bitvectors                    */
/*-----------------------------------------------------------------*/
bitVect *
bitVectIntersect (bitVect * bvp1, bitVect * bvp2)
{
  bitVect *newBvp;
  unsigned int *pn, *p1, *p2;
  int elements;

  if (!bvp2 || !bvp1)
    return NULL;

  /* if they are not the same size */
  /* make them the same size */
  if (bvp1->size < bvp2->size)
    bvp1 = bitVectResize (bvp1, bvp2->size);
  else if (bvp2->size < bvp1->size)
    bvp2 = bitVectResize (bvp2, bvp1->size);

  newBvp = newBitVect (bvp1->size);
  elements = bvp1->allocSize;

  pn = newBvp->vect; 
  p1 = bvp1->vect;
  p2 = bvp2->vect;

  while (elements--)
    {
      *pn++ = *p1++ & *p2++;
    }

  return newBvp;
}

/*-----------------------------------------------------------------*/
/* bitVectInplaceIntersect - intersect  two bitvectors             */
/*-----------------------------------------------------------------*/
bitVect *
bitVectInplaceIntersect (bitVect * bvp1, bitVect * bvp2)
{
  unsigned int *p1, *p2;
  int elements;

  if (!bvp2 || !bvp1)
    return NULL;

  /* if they are not the same size */
  /* make them the same size */
  if (bvp1->size < bvp2->size)
    bvp1 = bitVectResize (bvp1, bvp2->size);
  else if (bvp2->size < bvp1->size)
    bvp2 = bitVectResize (bvp2, bvp1->size);

  elements = bvp1->allocSize;

  p1 = bvp1->vect;
  p2 = bvp2->vect;

  while (elements--)
    {
      *p1 &= *p2;
      p1++; p2++;
    }

  return bvp1;
}

/*-----------------------------------------------------------------*/
/* bitVectBitsInCommon - special case of intersection determines   */
/*                       if the vectors have any common bits set   */
/*-----------------------------------------------------------------*/
int 
bitVectBitsInCommon (const bitVect * bvp1, const bitVect * bvp2)
{
  int elements;
  unsigned int *p1, *p2;

  if (!bvp1 || !bvp2)
    return 0;

  elements = min (bvp1->allocSize, bvp2->allocSize);

  p1 = bvp1->vect;
  p2 = bvp2->vect;

  while (elements--)
    {
      if (*p1 & *p2)
        return 1;
      p1++; p2++;
    }

  return 0;
}

/*-----------------------------------------------------------------*/
/* bitVectCplAnd - complement the second & and it with the first   */
/*-----------------------------------------------------------------*/
bitVect *
bitVectCplAnd (bitVect * bvp1, bitVect * bvp2)
{
  unsigned int *p1, *p2;
  int elements;
  
  if (!bvp2)
    return bvp1;

  if (!bvp1)
    return bvp1;

  /* if they are not the same size */
  /* make them the same size */
  if (bvp1->size < bvp2->size)
    bvp1 = bitVectResize (bvp1, bvp2->size);
  else if (bvp2->size < bvp1->size)
    bvp2 = bitVectResize (bvp2, bvp1->size);

  elements = bvp1->allocSize;
  p1 = bvp1->vect;
  p2 = bvp2->vect;

  while (elements--)
    {
      *p1 = *p1 & ~ *p2;
      p2++; p1++;
    }

  return bvp1;
}

/*-----------------------------------------------------------------*/
/* bitVectIsZero - bit vector has all bits turned off              */
/*-----------------------------------------------------------------*/
int 
bitVectIsZero (const bitVect * bvp)
{
  int i;

  if (!bvp)
    return 1;

  for (i = 0; i < bvp->allocSize; i++)
    if (bvp->vect[i] != 0)
      return 0;

  return 1;
}

/*-----------------------------------------------------------------*/
/* bitVectsEqual - returns 1 if the two bit vectors are equal      */
/*-----------------------------------------------------------------*/
int 
bitVectEqual (bitVect * bvp1, bitVect * bvp2)
{
  int i;
  int elements;
  
  if (!bvp1 || !bvp2)
    return 0;

  if (bvp1 == bvp2)
    return 1;

  /* elements common to both allocations must match */
  elements = min (bvp1->allocSize, bvp2->allocSize);
  for (i = 0; i < elements; i++)
    if (bvp1->vect[i] != bvp2->vect[i])
      return 0;

  /* any extra elements allocated must be 0 */
  if (bvp1->allocSize > elements)
    {
      for (i = elements; i < bvp1->allocSize; i++)
        if (bvp1->vect[i])
          return 0;
    }
  if (bvp2->allocSize > elements)
    {
      for (i = elements; i < bvp2->allocSize; i++)
        if (bvp2->vect[i])
          return 0;
    }

  return 1;
}

/*-----------------------------------------------------------------*/
/* bitVectCopy - creates a bitvector from another bit Vector       */
/*-----------------------------------------------------------------*/
bitVect *
bitVectCopy (const bitVect * bvp)
{
  bitVect *newBvp;
  int i;

  if (!bvp)
    return NULL;

  newBvp = newBitVect (bvp->size);
  for (i = 0; i < bvp->allocSize; i++)
    newBvp->vect[i] = bvp->vect[i];

  return newBvp;
}

/*-----------------------------------------------------------------*/
/* bitVectnBitsOn - returns the number of bits that are on         */
/*-----------------------------------------------------------------*/
int 
bitVectnBitsOn (const bitVect * bvp)
{
  int count = 0;
  unsigned int *p1;
  int elements;

  if (!bvp)
    return 0;

  p1 = bvp->vect;
  elements = bvp->allocSize;
  
  while (elements--)
    {
      unsigned int word = *p1++;
      while (word)
        {
          count++;
          word &= word-1;
        }
    }

  return count;
}

/*-----------------------------------------------------------------*/
/* bitVectFirstBit - returns the key for the first bit that is on  */
/*-----------------------------------------------------------------*/
int 
bitVectFirstBit (const bitVect * bvp)
{
  int i;
  int index;

  if (!bvp)
    return -1;

  for (i = 0, index = 0; i < bvp->size; i+=BIT_SIZEOF_ELEMENT, index++)
    if (bvp->vect[index])
      break;

  for (; i < bvp->size; i++)
    if (bitVectBitValue (bvp, i))
      return i;

  return -1;
}

/*-----------------------------------------------------------------*/
/* bitVectClear - clear all bits                                   */
/*-----------------------------------------------------------------*/
void
bitVectClear (bitVect *bvp)
{
  int i;

  if (!bvp)
    return;

  for (i = 0; i < bvp->allocSize; i++)
    bvp->vect[i] = 0;
}

/*-----------------------------------------------------------------*/
/* bitVectDebugOn - prints bits that are on                        */
/*-----------------------------------------------------------------*/
void 
bitVectDebugOn (const bitVect * bvp, FILE * of)
{
  int i;

  if (of == NULL)
    of = stdout;
  if (!bvp)
    return;

  fprintf (of, "bitvector Size = %d allocSize = %d\n", bvp->size, bvp->allocSize);
  fprintf (of, "Bits on { ");
  for (i = 0; i < bvp->size; i++)
    {
      if (bitVectBitValue (bvp, i))
        fprintf (of, "(%d) ", i);
    }
  fprintf (of, "}\n");
}

/*-----------------------------------------------------------------*/
/* bitVectPrint - prints all the bits                              */
/*-----------------------------------------------------------------*/
void
bitVectPrint (FILE * of, const bitVect * bvp)
{
  if (bvp)
    {
      for (int i = 0; i < bvp->size; ++i)
        fprintf (of, "%d", bitVectBitValue (bvp, i));
    }
  else
    fprintf (of, "(nil)");
}
