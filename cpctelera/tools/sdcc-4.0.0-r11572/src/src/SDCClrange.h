/*-------------------------------------------------------------------------

  SDCClrange.h - header file for live range computations

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

#ifndef SDCCLRANGE_H
#define SDCCLRANGE_H 1

extern hTab *liveRanges;
extern hTab *iCodehTab;
extern hTab *iCodeSeqhTab;

int  notUsedInBlock (symbol *, eBBlock *, iCode *);
bool allDefsOutOfRange (bitVect *, int, int);
void computeLiveRanges (eBBlock **ebbs, int count, bool emitWarnings);
void recomputeLiveRanges (eBBlock **ebbs, int count, bool emitWarnings);

void setToRange (operand *, int, bool);
void hashiCodeKeys (eBBlock **, int);

void adjustIChain (eBBlock ** ebbs, int count);

int separateLiveRanges (iCode *sic, ebbIndex *ebbi); /* Split iTemps that have non-connected live-ranges. */

int shortenLiveRanges (iCode *sic, ebbIndex *ebbi); /* Do some optimizations that shorten live ranges. */

#endif

