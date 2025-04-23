/*-------------------------------------------------------------------------
  SDCCpeeph.h - Header file for The peep hole optimizer: for interpreting 
                the peep hole rules

  Copyright (C) 1999, Sandeep Dutta . sandeep.dutta@usa.net

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

#ifndef  SDCCPEEPH_H
#define SDCCPEEPH_H 1

#include "SDCCgen.h"

#define MAX_PATTERN_LEN 256

typedef struct peepRule
  {
    lineNode *match;
    lineNode *replace;
    unsigned int restart:1;
    unsigned int barrier:1;
    char *cond;
    hTab *vars;
    struct peepRule *next;
  }
peepRule;

typedef struct
  {
    char name[SDCC_NAME_MAX + 1];
    int refCount;
    /* needed for deadMove: */
    bool passedLabel;
    int jmpToCount;
  }
labelHashEntry;

bool isLabelDefinition (const char *line, const char **start, int *len,
                        bool isPeepRule);

extern hTab *labelHash;
labelHashEntry *getLabelRef (const char *label, lineNode *head);

void initPeepHole (void);
void peepHole (lineNode **);

const char * StrStr (const char * str1, const char * str2);

#endif
