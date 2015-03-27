/*-------------------------------------------------------------------------

   pcoderegs.h - post code generation register optimizations

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

#ifndef __PCODEREGS_H__
#define __PCODEREGS_H__

#include "common.h"

#include "pcode.h"

/*************************************************

  pCodeRegLives 

  Records the set of registers used in a flow object.

**************************************************/

typedef struct pCodeRegLives {
	set *usedpFlows;       /* set of pFlow objects that use this register */
	set *assignedpFlows;   /* set of pFlow objects that assign values to this register */
	set *usedpCodes;       /* set of all instructions that use this register */

} pCodeRegLives;

void pCodeRegMapLiveRanges(struct pBlock *pb);
void pCodeRegOptimizeRegUsage(int level);
void RegsUnMapLiveRanges(void);
void RemoveUnusedRegisters(void);

#endif //  __PCODEREGS_H__
