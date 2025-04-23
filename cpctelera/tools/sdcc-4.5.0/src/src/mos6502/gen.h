/*-------------------------------------------------------------------------
  gen.h - header file for code generation for mos6502

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
-------------------------------------------------------------------------*/

#ifndef SDCCGENM6502_H
#define SDCCGENM6502_H

typedef enum
  {
  AOP_INVALID,
  AOP_LIT = 1,   /* operand is a literal value */
  AOP_REG,       /* is in registers */
  AOP_DIR,       /* operand using direct addressing mode */
  AOP_STK,       /* should be pushed on stack this
                    can happen only for the result */
  AOP_IMMD,      /* immediate value for eg. remateriazable */
  AOP_STR,       /* array of strings */
  AOP_CRY,       /* carry contains the value of this */
  AOP_EXT,       /* operand using extended addressing mode */
  AOP_SOF,       /* operand at an offset on the stack */
  AOP_DUMMY,     /* Read undefined, discard writes */
  AOP_IDX        /* operand using indexed addressing mode */
}
AOP_TYPE;

/* asmop: A homogenised type for all the different
   spaces an operand can be in */
typedef struct asmop
  {
    AOP_TYPE type;		
    short coff;			/* current offset */
    short size;			/* total size */    	
    short regmask;              /* register mask if AOP_REG */
    operand *op;		/* originating operand */
    unsigned freed:1;		/* already freed    */
    unsigned stacked:1;		/* partial results stored on stack */
    struct asmop *stk_aop[4];	/* asmops for the results on the stack */
    union
      {
	value *aop_lit;		/* if literal */
	reg_info *aop_reg[4];	/* array of registers */
	char *aop_dir;		/* if direct  */
        char *aop_immd;         /* if immediate */
	int aop_stk;		/* stack offset when AOP_STK */
    } aopu;
  }
asmop;

void genm6502Code (iCode *);
void m6502_emitDebuggerSymbol (const char *);

extern unsigned fReturnSizeM6502;

extern bool m6502_assignment_optimal;

#endif

