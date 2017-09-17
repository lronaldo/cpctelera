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
    AOP_REG, AOP_DIR,
    AOP_DPTR, AOP_X, AOP_Z,
    AOP_STK, AOP_IMMD, AOP_STR,
    AOP_CRY, AOP_ACC, AOP_STK_D
  };

/* type asmop : a homogenised type for 
   all the different spaces an operand can be
   in */
typedef struct asmop
  {

    short type;			/* can have values
				   AOP_LIT    -  operand is a literal value
				   AOP_REG    -  is in registers
				   AOP_DIR    -  direct just a name
				   AOP_DPTR   -  dptr contains address of operand
				   AOP_R0/R1  -  r0/r1 contains address of operand               
				   AOP_STK    -  should be pushed on stack this
				   can happen only for the result
				   AOP_IMMD   -  immediate value for eg. remateriazable 
				   AOP_CRY    -  carry contains the value of this
				   AOP_STR    -  array of strings
				   AOP_ACC    -  result is in the acc:b pair
				 */
    short coff;			/* current offset */
    short size;			/* total size */
    unsigned code:1;		/* is in Code space */
    unsigned paged:1;		/* in paged memory  */
    unsigned freed:1;		/* already freed    */
    union
      {
	value *aop_lit;		/* if literal */
	regs *aop_reg[4];	/* array of registers */
	char *aop_dir;		/* if direct  */
	regs *aop_ptr;		/* either -> R26 or R30 */
	char *aop_immd;		/* if immediate others are implied */
	int aop_stk;		/* stack offset when AOP_STK */
	char *aop_str[4];	/* just a string array containing the location */
      }
    aopu;
    regs *aop_ptr2;		/* either -> R27 or R31 */
  }
asmop;

void genAVRCode (iCode *);
void avr_emitDebuggerSymbol (const char *);

extern char *fReturn8051[];
extern char *fReturn390[];
extern unsigned fAVRReturnSize;
extern char **fAVRReturn;

#endif
