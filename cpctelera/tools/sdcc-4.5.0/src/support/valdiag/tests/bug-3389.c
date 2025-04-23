/* bug-3389.c

   Missing diagnostic on multiple declarations, some banked some not.
 */
 
/*-------------------------------------------------------------------------
  SDCCgen.c - source files for target code generation common functions

  Copyright (C) 2019, Paul Chandler (SourceForge user under4mhz)
  Copyright (C) 2022, Philipp Klaus Krause (SourceForge user spth)

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

#ifdef TEST1
#if !defined(__SDCC_mcs51) && !defined(__SDCC_z80) && !defined(__SDCC_sm83) 
#define __banked
#define __nonbanked
#endif

typedef struct { char x; } maths_point_i8;

typedef maths_point_i8 PointType;

typedef struct moongate_data {
    unsigned char active;
} MoongateData;


void MoongatePositionGet( unsigned char gate, PointType *position ) __banked; /* IGNORE */

void MoongatePositionGet( unsigned char, PointType *position ) __nonbanked; /* ERROR(SDCC_mcs51|SDCC_z80|SDCC_sm83) */

#endif

