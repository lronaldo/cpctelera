/*-------------------------------------------------------------------------
  SDCCdebug.h - header file debug infrastructure

  Copyright (C) 2003, Lenny Story and Bernhard Held

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

#ifndef _DEBUG_INCLUDE_
#define _DEBUG_INCLUDE_

typedef struct DebugFile
{
  int (*openFile) (const char *file);
  int (*closeFile) (void);
  int (*writeModule) (const char *name);
  int (*writeFunction) (symbol *pSym, iCode *ic);
  int (*writeEndFunction) (symbol *pSym, iCode *ic, int offset);
  int (*writeLabel) (symbol *pSym, iCode *ic);
  int (*writeScope) (iCode *ic);
  int (*writeSymbol) (symbol *pSym);
  int (*writeType) (structdef *sdef, int block, int inStruct, const char *tag);
  int (*writeCLine) (iCode *ic);
  int (*writeALine) (const char *module, int Line);
  int (*writeFrameAddress) (const char *variable, struct reg_info *reg, int offset);
}DEBUGFILE;

extern DEBUGFILE *debugFile;

void outputDebugStackSymbols (void);
void outputDebugSymbols (void);
void dumpSymInfo (const char *pcName, memmap *memItem);
void emitDebuggerSymbol (const char * debugSym);

#endif
