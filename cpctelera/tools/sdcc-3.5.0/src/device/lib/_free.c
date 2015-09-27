/*-------------------------------------------------------------------------
   free.c - release allocated memory.

   Copyright (C) 2004, Maarten Brock, sourceforge.brock@dse.nl

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License 
   along with this library; see the file COPYING. If not, write to the
   Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
   MA 02110-1301, USA.

   As a special exception, if you link this library with other files,
   some of which are compiled with SDCC, to produce an executable,
   this library does not by itself cause the resulting executable to
   be covered by the GNU General Public License. This exception does
   not however invalidate any other reasons why the executable file
   might be covered by the GNU General Public License.
-------------------------------------------------------------------------*/

#include <sdcc-lib.h>
#include <malloc.h>

#if defined(__SDCC_STACK_AUTO) || defined(__SDCC_z80) || defined(__SDCC_z180) || defined(__SDCC_gbz80)
  #define CRITICAL __critical
#else
  #define CRITICAL
#endif

#if _SDCC_MALLOC_TYPE_MLH

typedef struct _MEMHEADER MEMHEADER;

struct _MEMHEADER
{
  MEMHEADER *    next;
  MEMHEADER *    prev;
  unsigned int   len;
  unsigned char  mem;
};

#define HEADER_SIZE (sizeof(MEMHEADER)-sizeof(char))

/* These variables are defined through the crt0 functions. */
/* Base of this variable is the first byte of the heap. */
extern MEMHEADER __sdcc_heap_start;
/* Address of this variable is the last byte of the heap. */
extern char __sdcc_heap_end;

MEMHEADER * __sdcc_prev_memheader;
// apart from finding the header
// this function also finds it's predecessor
MEMHEADER *
__sdcc_find_memheader(void * p)
{
  MEMHEADER * pthis;
  if (!p)
    return NULL;
  pthis = (MEMHEADER * )((char *)  p - HEADER_SIZE); //to start of header
  __sdcc_prev_memheader = pthis->prev;

  return (pthis);
}

void
free (void *p)
{
  MEMHEADER *prev_header, *pthis;

  if ( p ) //For allocated pointers only!
    CRITICAL
    {
      pthis = (MEMHEADER * )((char *)  p - HEADER_SIZE); //to start of header
      if ( pthis->prev ) // For the regular header
        {
          prev_header = pthis->prev;
          prev_header->next = pthis->next;
          if (pthis->next)
            {
              pthis->next->prev = prev_header;
            }
        }
      else
        {
          pthis->len = 0; //For the first header
        }
    }
}

#else

            //--------------------------------------------------------------------
            //Written by Dmitry S. Obukhov, 1997
            //dso@usa.net
            //--------------------------------------------------------------------
            //Modified for SDCC by Sandeep Dutta, 1999
            //sandeep.dutta@usa.net
            //--------------------------------------------------------------------
            //malloc and free functions implementation for embedded system
            //Non-ANSI keywords are C51 specific.
            // __xdata - variable in external memory (just RAM)
            //--------------------------------------------------------------------

            #define MEMHEADER   struct MAH// Memory Allocation Header

            MEMHEADER
            {
              MEMHEADER __xdata *  next;
              unsigned int         len;
              unsigned char        mem[];
            };

            #define HEADER_SIZE sizeof(MEMHEADER)

            extern __xdata char __sdcc_heap[];
            #define __sdcc_first_memheader ((MEMHEADER __xdata * ) __sdcc_heap)

            MEMHEADER __xdata * __sdcc_prev_memheader;
            // apart from finding the header
            // this function also finds it's predecessor
            MEMHEADER __xdata * __sdcc_find_memheader(void __xdata * p)
            {
              MEMHEADER __xdata * pthis;
              MEMHEADER __xdata * cur_header;

              if (!p)
                return NULL;
              pthis = (MEMHEADER __xdata *) p;
              pthis -= 1; //to start of header
              cur_header = __sdcc_first_memheader;
              __sdcc_prev_memheader = NULL;
              while (cur_header && pthis != cur_header)
              {
                __sdcc_prev_memheader = cur_header;
                cur_header = cur_header->next;
              }
              return (cur_header);
            }

            void free (void * p)
            {
              MEMHEADER __xdata * pthis;

              CRITICAL
              {
                pthis = __sdcc_find_memheader(p);
                if (pthis) //For allocated pointers only!
                {
                  if (!__sdcc_prev_memheader)
                  {
                    pthis->len = 0;
                  }
                  else
                  {
                    __sdcc_prev_memheader->next = pthis->next;
                  }
                }
              }
            }
            //END OF MODULE
#endif
