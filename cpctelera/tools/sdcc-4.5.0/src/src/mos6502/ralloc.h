/*-------------------------------------------------------------------------

  SDCCralloc.h - header file register allocation

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

#ifndef SDCCRALLOC_H
#define SDCCRALLOC_H 1

#include "common.h"

enum
  {
    A_IDX,
    X_IDX,
    Y_IDX,
    //    Z_IDX,
    CND_IDX,
    XA_IDX,
    //    YA_IDX,   
    YX_IDX,
    SP_IDX
  };

enum
  {
    REG_PTR = 1,
    REG_GPR = 2,
    REG_CND = 4,
  };

/* Must preserve the relation M6502MASK_Y > M6502MASK_X > M6502MASK_A  */
/* so that M6502MASK_REV can be automatically applied when reversing */
/* the usual register pair ordering. */
#define M6502MASK_A 0x01
#define M6502MASK_X 0x02
#define M6502MASK_Y 0x04
//#define M6502MASK_Z 0x10
//#define M6502MASK_REV 0x80
#define M6502MASK_XA (M6502MASK_X | M6502MASK_A)
//#define M6502MASK_YA (M6502MASK_Y | M6502MASK_A)
#define M6502MASK_YX (M6502MASK_Y | M6502MASK_X)
    
/* definition for the registers */
typedef struct reg_info
{
  short type;			/* can have value 
				   REG_GPR, REG_PTR or REG_CND */
  short rIdx;			/* index into register table */
  char *name;			/* name */
  short mask;			/* bitmask for pair allocation */
  struct asmop *aop;		/* last operand */
  int aopofs;			/* last operand offset */
  unsigned isFree:1;		/* is currently unassigned */
  unsigned isDead:1;      /* does not need to survive current instruction */
  unsigned isLitConst:1;      /* has an literal constant loaded */
  int litConst;		/* last literal constant */
} reg_info;

typedef enum {
  M6502OP_REG,
  M6502OP_LD,
  M6502OP_ST,
  M6502OP_RMW,
  M6502OP_INH,
  M6502OP_SPH,
  M6502OP_SPL,
  M6502OP_JMP,
  M6502OP_BR,
  M6502OP_BBR,
  M6502OP_IDD,
  M6502OP_IDI,
  M6502OP_CMP
} m6505_op_type;

/* opcode table */
typedef struct m6502opcodedata
{
  char name[6];
  m6505_op_type type;
  int  dest;
  unsigned char flags;
  /* info for registers used and/or modified by an instruction will be added here */
}
  m6502opcodedata;

extern reg_info regsm6502[];
extern reg_info *m6502_reg_a;
extern reg_info *m6502_reg_x;
extern reg_info *m6502_reg_y;
extern reg_info *m6502_reg_xa;
//extern reg_info *m6502_reg_ya;
extern reg_info *m6502_reg_yx;
extern reg_info *m6502_reg_sp;

reg_info *m6502_regWithIdx (int);
void m6502_useReg (reg_info * reg);
void m6502_freeReg (reg_info * reg);
void m6502_dirtyReg (reg_info * reg);
bitVect *m6502_rUmaskForOp (operand * op);

const m6502opcodedata *m6502_getOpcodeData(const char *inst);
int m6502_opcodeSize(const m6502opcodedata *opcode, const char *arg);

iCode *m6502_ralloc2_cc(ebbIndex *ebbi);

void m6502RegFix (eBBlock **ebbs, int count);

#endif
