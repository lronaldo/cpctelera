/*-------------------------------------------------------------------------

   pcode.h - post code generation
   Written By -  Scott Dattalo scott@dattalo.com

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

#ifndef __PCODEFLOW_H__
#define __PCODEFLOW_H__

#include "pcode.h"

/*************************************************
 * pCode conditions:
 *
 * The "conditions" are bit-mapped flags that describe
 * input and/or output conditions that are affected by
 * the instructions. For example:
 *
 *    MOVF   SOME_REG,W
 *
 * This instruction depends upon 'SOME_REG'. Consequently
 * it has the input condition PCC_REGISTER set to true.
 *
 * In addition, this instruction affects the Z bit in the
 * status register and affects W. Thus the output conditions
 * are the logical or:
 *  PCC_ZERO_BIT | PCC_W
 *
 * The conditions are intialized when the pCode for an
 * instruction is created. They're subsequently used
 * by the pCode optimizer determine state information
 * in the program flow.
 *************************************************/

#define  PCC_NONE          0
#define  PCC_REGISTER      (1<<0)
#define  PCC_C             (1<<1)
#define  PCC_Z             (1<<2)
#define  PCC_DC            (1<<3)
#define  PCC_W             (1<<4)
#define  PCC_EXAMINE_PCOP  (1<<5)
#define  PCC_REG_BANK0     (1<<6)
#define  PCC_REG_BANK1     (1<<7)
#define  PCC_REG_BANK2     (1<<8)
#define  PCC_REG_BANK3     (1<<9)
#define  PCC_LITERAL       (1<<10)

/*------------------------------------------------------------*/

void BuildFlowTree(pBlock *pb);

#endif // __PCODEFLOW_H__

