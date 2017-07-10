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
-------------------------------------------------------------------------*/

#ifndef Z80GEN_H
#define Z80GEN_H

typedef enum
{
  AOP_INVALID,
  /* Is a literal */
  AOP_LIT = 1,
  /* Is in a register */
  AOP_REG,
  /* Is in direct space */
  AOP_DIR,
  /* SFR space ($FF00 and above) */
  AOP_SFR,
  /* Is on the stack */
  AOP_STK,
  /* Is an immediate value */
  AOP_IMMD,
  /* Is a string (?) */
  AOP_STR,
  /* Is in the carry register */
  AOP_CRY,
  /* Is pointed to by IY */
  AOP_IY,
  /* Is pointed to by HL */
  AOP_HL,
  /* Is in A */
  AOP_ACC,
  /* Is in H and L */
  AOP_HLREG,
  /* Simple literal. */
  AOP_SIMPLELIT,
  /* Is in the extended stack pointer (IY on the Z80) */
  AOP_EXSTK,
  /* Is referenced by a pointer in a register pair. */
  AOP_PAIRPTR,
  /* Read undefined, discard writes */
  AOP_DUMMY
}
AOP_TYPE;

/* type asmop : a homogenised type for 
   all the different spaces an operand can be
   in */
typedef struct asmop
{
  AOP_TYPE type;
  short coff;                   /* current offset */
  short size;                   /* total size */
  unsigned code:1;              /* is in Code space */
  unsigned paged:1;             /* in paged memory  */
  unsigned freed:1;             /* already freed    */
  unsigned bcInUse:1;           /* for banked I/O, which uses bc for the I/O address */
  union
  {
    value *aop_lit;             /* if literal */
    reg_info *aop_reg[4];       /* array of registers */
    char *aop_dir;              /* if direct  */
    char *aop_immd;             /* if immediate others are implied */
    int aop_stk;                /* stack offset when AOP_STK */
    const char *aop_str[4];     /* just a string array containing the location */
    unsigned long aop_simplelit;        /* Just the value. */
    int aop_pairId;             /* The pair ID */
  }
  aopu;
}
asmop;

void genZ80Code (iCode *);
void z80_emitDebuggerSymbol (const char *);

extern bool z80_assignment_optimal;
extern bool should_omit_frame_ptr;

#endif

