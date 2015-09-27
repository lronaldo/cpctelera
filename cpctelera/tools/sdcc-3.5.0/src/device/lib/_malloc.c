/*-------------------------------------------------------------------------
   malloc.c - allocate memory.

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
extern MEMHEADER _sdcc_heap_start;
/* Address of this variable is the last byte of the heap. */
extern char _sdcc_heap_end;

void
_sdcc_heap_init(void)
{
  MEMHEADER *pbase = &_sdcc_heap_start;
  unsigned int size = &_sdcc_heap_end - (char *)pbase;

  pbase->next = (MEMHEADER *)((char *)pbase + size - HEADER_SIZE);
  pbase->next->next = NULL; //And mark it as last
  pbase->prev       = NULL; //and mark first as first
  pbase->len        = 0;    //Empty and ready.
}

void *
malloc (unsigned int size)
{
  register MEMHEADER * current_header;
  register MEMHEADER * new_header;
  register void * ret;

  if (size>(0xFFFF-HEADER_SIZE))
    {
      return NULL; //To prevent overflow in next line
    }

  size += HEADER_SIZE; //We need a memory for header too
  current_header = &_sdcc_heap_start;

  CRITICAL
    {
      while (1)
        {
          //    current
          //    |   len       next
          //    v   v         v
          //....*****.........******....
          //         ^^^^^^^^^
          //           spare

          if ((((unsigned int)current_header->next) -
               ((unsigned int)current_header) -
               current_header->len) >= size)
            { //if spare is more than needed
              ret = &current_header->mem;
              break;
            }
          current_header = current_header->next;    //else try next
          if (!current_header->next)
            { //if end_of_list reached
              ret = NULL;
              break;
            }
        }

      if (ret)
        {
          if (!current_header->len)
            { //This code works only for first_header in the list and only
              current_header->len = size; //for first allocation
            }
          else
            {
              //else create new header at the begin of spare
              new_header = (MEMHEADER * )((char *)current_header + current_header->len);
              new_header->next = current_header->next; //and plug it into the chain
              new_header->prev = current_header;
              current_header->next  = new_header;
              if (new_header->next)
                {
                  new_header->next->prev = new_header;
                }
              new_header->len  = size; //mark as used
              ret = &new_header->mem;
            }
        }
    }
  return ret;
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

            #define __sdcc_first_memheader ((MEMHEADER __xdata * ) __sdcc_heap)

            extern __xdata char __sdcc_heap[];
            extern __xdata void * const __sdcc_last_memheader;

            void __xdata * malloc (unsigned int size)
            {
              MEMHEADER __xdata * current_header;
              MEMHEADER __xdata * new_header;
              void __xdata * ret;

              size += HEADER_SIZE; //We need a memory for header too
              if (size <= HEADER_SIZE)
                return (void __xdata *) NULL; //Overflow or requested size was 0

              if (!__sdcc_first_memheader->next)
              {
                //Reserve a mem for last header
                __sdcc_first_memheader->next = (MEMHEADER __xdata * )__sdcc_last_memheader;
              }

              current_header = __sdcc_first_memheader;
              CRITICAL
              {
                while (1)
                {

                  //    current
                  //    |   len       next
                  //    v   v         v
                  //....*****.........******....
                  //         ^^^^^^^^^
                  //           spare

                  if ((((unsigned int)current_header->next) -
                       ((unsigned int)current_header) -
                       current_header->len) >= size)
                  { //if spare is more than needed
                    ret = current_header->mem;
                    break;
                  }
                  current_header = current_header->next;    //else try next
                  if (!current_header->next)
                  { //if end_of_list reached
                    ret = (void __xdata *) NULL;
                    break;
                  }
                }
                if (ret)
                {
                  if (!current_header->len)
                  { //This code works only for first_header in the list and only
                    current_header->len = size; //for first allocation
                  }
                  else
                  { //else create new header at the begin of spare
                    new_header = (MEMHEADER __xdata * )((char __xdata *)current_header + current_header->len);
                    new_header->next = current_header->next; //and plug it into the chain
                    current_header->next  = new_header;
                    new_header->len  = size; //mark as used
                    ret = new_header->mem;
                  }
                }
              }
              return ret;
            }

            //END OF MODULE
#endif

