/*-------------------------------------------------------------------------
  SDCCgen51.h - header file for code generation for 8051

             Written By -  Sandeep Dutta . sandeep.dutta@usa.net (1998)

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

#ifndef SDCCGEN51_H
#define SDCCGEN51_H

enum
  {
    AOP_LIT = 1,
    AOP_REG, 
    AOP_DIR, 
    AOP_FAR,
    AOP_CODE,
    AOP_GPTR,
    AOP_STK,
    AOP_IMMD, 
    AOP_BIT
  };

/* type asmop : a homogenised type for 
   all the different spaces an operand can be
   in */
typedef struct asmop {
  
  short type;			
  /* can have values
     AOP_LIT    -  operand is a literal value
     AOP_REG    -  is in registers
     AOP_DIR    -  direct, just a name
     AOP_FAR    -  
     AOP_CODE   - 
     AOP_GPTR   -
     AOP_STK    -  on stack (with offset)
     AOP_IMMD   -  immediate value for eg. remateriazable 
     AOP_CRY    -  carry contains the value of this
  */
  short size; /* size of this aop */
  char name[2][64]; /* can be "r0" "r6h" [rxbw+y] "#..." */
} asmop;

#define AOP(x) x->aop
#define AOP_TYPE(x) x->aop->type
#define AOP_SIZE(x) x->aop->size
#define AOP_NAME(x) x->aop->name

void xa51_emitDebuggerSymbol (const char *);

#endif
