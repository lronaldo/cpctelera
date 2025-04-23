/*-------------------------------------------------------------------------

  SDCCralloc.h - header file register allocation

                Written By -  Philipp Krause . pkk@spth.de (2012)

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
  XL_IDX = 0,// Lower byte of X - The main 8-bot accumulator
  XH_IDX,    // Upper byte of X
  YL_IDX,    // Lower byte of Y
  YH_IDX,    // Upper byte of Y
  ZL_IDX,    // Lower byte of Z
  ZH_IDX,    // Upper byte of Z
  F_IDX,     // Flag register
  C_IDX,     // Carry

  X_IDX,     // X - for use with code generation support functions only.
  Y_IDX,     // Y - for use with code generation support functions only.
  Z_IDX,     // Z - for use with code generation support functions only.

  SP_IDX     // SP - for use with debug info.
};

enum
{
  REG_GPR = 2,
  REG_CND = 4,
};

/* definition for the registers */
typedef struct reg_info
{
  short type;                   /* can have value 
                                   REG_GPR, REG_PTR or REG_CND */
  short rIdx;                   /* index into register table */
  char *name;                   /* name */
} reg_info;

extern reg_info f8_regs[];

void f8_assignRegisters (ebbIndex *);

void f8SpillThis (symbol *sym, bool force_spill);
iCode *f8_ralloc2_cc(ebbIndex *ebbi);

void f8RegFix (eBBlock **ebbs, int count);
#endif

