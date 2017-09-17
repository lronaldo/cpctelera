/*-------------------------------------------------------------------------
  main.c - mcs51 specific general header file

  Copyright (C) 1998, Sandeep Dutta . sandeep.dutta@usa.net
  Copyright (C) 1999, Jean-Louis VERN.jlvern@writeme.com
  Copyright (C) 2000, Michael Hope

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
-------------------------------------------------------------------*/

#ifndef MAIN_INCLUDE
#define MAIN_INCLUDE

#include "SDCCgen.h"

typedef struct asmLineNode
  {
#ifdef UNNAMED_STRUCT_TAG
    struct asmLineNodeBase;
#else
    /* exactly the same members as of struct asmLineNodeBase from SDCCgen.h */
    int size;
    bitVect *regsRead;
    bitVect *regsWritten;
#endif
  }
asmLineNode;

bool x_parseOptions (char **argv, int *pargc);
void x_setDefaultOptions (void);
void x_finaliseOptions (void);

#endif
