/*-------------------------------------------------------------------------

  main.h - header file for pic16 specific general functions.

  Copyright (C) 2000, Scott Dattalo scott@dattalo.com
  Copyright (C) 2002, Martin Dubuc m.debuc@rogers.com

  Written by - Scott Dattalo scott@dattalo.com
  Ported to PIC16 by - Martin Dubuc m.debuc@rogers.com

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

#ifndef MAIN_INCLUDE
#define MAIN_INCLUDE

#include "ralloc.h"

bool x_parseOptions (char **argv, int *pargc);
void x_setDefaultOptions (void);
void x_finaliseOptions (void);


typedef struct {
  char *at_udata;
} pic16_sectioninfo_t;

typedef struct absSym {
  char name[SDCC_SYMNAME_MAX+1];
  unsigned int address;
} absSym;

typedef struct sectName {
  char *name;
  set *regsSet;
} sectName;

typedef struct sectSym {
  sectName *section;
  char *name;
  reg_info *reg;
} sectSym;

extern set *absSymSet;
extern set *sectNames;
extern set *sectSyms;
extern set *wparamList;

extern int pic16_mplab_comp;

#endif
