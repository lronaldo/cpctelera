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
#include "SDCCicode.h"
#include "SDCCBBlock.h"
#ifndef SDCCRALLOC_H
#define SDCCRALLOC_H 1

#define REG_PTR 0x01 // pointer register
#define REG_GPR 0x02 // general purpose register
#define REG_CND 0x04 // condition (bit) register
#define REG_SCR 0x40 // scratch register
#define REG_STK 0x80 // stack pointer register

// register ID's
enum {
  R0L_ID=0x10,R0H_ID,R1L_ID,R1H_ID,R2L_ID,R2H_ID,R3L_ID,R3H_ID,
  R4L_ID,R4H_ID,R5L_ID,R5H_ID,R6L_ID,R6H_ID,R7L_ID,R7H_ID,
  R0_ID=0x20, R1_ID, R2_ID, R3_ID, R4_ID, R5_ID, R6_ID, R7_ID,
  R8_ID, I9_ID, R10_ID, R11_ID, R12_ID, R13_ID, R14_ID, R15_ID,
  R0R1_ID=0x40, R2R3_ID, R4R5_ID, R6R7_ID
};

typedef struct regs {
  unsigned char rIdx; // the register ID
  unsigned char size; // size of register (1,2,4)
  unsigned char type; // scratch, pointer, general purpose, stack, condition (bit)
  char *name;
  unsigned long regMask;
  bool isFree;
  symbol *sym; // used by symbol
} regs;

extern regs regsXA51[];
extern unsigned long xa51RegsInUse;

regs *xa51_regWithIdx (int);

bitVect *xa51_rUmaskForOp (operand * op);

#endif
