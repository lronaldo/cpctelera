/*-------------------------------------------------------------------------

  SDCCglue.h - glues everything we have done together into one file.                 
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

#ifndef PIC16_GLUE_H
#define PIC16_GLUE_H

#include "SDCCset.h"

extern set *pic16_localFunctions;

extern void pic16_pCodeInitRegisters(void);
extern void pic16glue (void);
extern unsigned int pic16aopLiteral (value *val, int offset);
extern int pic16_stringInSet(const char *str, set **world, int autoAdd);

#endif
