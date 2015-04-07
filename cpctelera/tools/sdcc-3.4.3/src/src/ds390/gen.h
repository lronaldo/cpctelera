/*-------------------------------------------------------------------------
  gen.h - header file for code generation for DS80C390

  Copyright (C) 1998, Sandeep Dutta . sandeep.dutta@usa.net

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
-------------------------------------------------------------------------*/

#ifndef SDCCGEN390_H
#define SDCCGEN390_H

enum
{
  AOP_LIT = 1,
  AOP_REG, AOP_DIR,
  AOP_DPTR, AOP_DPTR2, AOP_R0, AOP_R1,
  AOP_STK, AOP_IMMD, AOP_STR,
  AOP_CRY, AOP_ACC, AOP_DPTRn, AOP_DUMMY
};

/* type asmop : a homogenised type for
   all the different spaces an operand can be
   in */
typedef struct asmop
{

  short type;                   /* can have values
                                   AOP_LIT    -  operand is a literal value
                                   AOP_REG    -  is in registers
                                   AOP_DIR    -  direct just a name
                                   AOP_DPTR   -  dptr contains address of operand
                                   AOP_DPTR2  -  dptr2 contains address of operand (DS80C390 only).
                                   AOP_R0/R1  -  r0/r1 contains address of operand
                                   AOP_STK    -  should be pushed on stack this
                                                 can happen only for the result
                                   AOP_IMMD   -  immediate value for eg. remateriazable
                                   AOP_CRY    -  carry contains the value of this
                                   AOP_STR    -  array of strings
                                   AOP_ACC    -  result is in the acc:b pair
                                   AOP_DPTRn  -  is in dptr(n)
                                   AOP_DUMMY  -  read as 0, discard writes
                                 */
  short coff;                   /* current offset */
  short size;                   /* total size */
  unsigned code:1;              /* is in Code space */
  unsigned paged:1;             /* in paged memory  */
  unsigned short allocated;     /* number of times allocated */
  union
  {
    short dptr;                 /* if AOP_DPTRn */
    value *aop_lit;             /* if literal */
    reg_info *aop_reg[4];       /* array of registers */
    char *aop_dir;              /* if direct  */
    reg_info *aop_ptr;          /* either -> to r0 or r1 */
    struct
    {
      int from_cast_remat;      /* cast remat created this : immd2 field used for highest order */
      char *aop_immd1;          /* if immediate others are implied */
      char *aop_immd2;          /* cast remat will generate this   */
    } aop_immd;
    int aop_stk;                /* stack offset when AOP_STK */
    char *aop_str[5];           /* just a string array containing the location */
  }
  aopu;
}
asmop;

void gen390Code (iCode *);
void ds390_emitDebuggerSymbol (const char *);

#endif
