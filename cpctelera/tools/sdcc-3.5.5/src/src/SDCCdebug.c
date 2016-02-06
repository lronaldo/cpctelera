/*-------------------------------------------------------------------------
  SDCCdebug.c - source file for debug infrastructure

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

#include "common.h"

extern DEBUGFILE cdbDebugFile;

DEBUGFILE *debugFile = &cdbDebugFile;

void
outputDebugStackSymbols (void)
{
  if (getenv ("SDCC_DEBUG_VAR_STORAGE"))
    {
      dumpSymInfo ("XStack", xstack);
      dumpSymInfo ("IStack", istack);
    }

  if (options.debug && debugFile)
    {
      symbol *sym;

      if (xstack)
        {
          for (sym = setFirstItem (xstack->syms); sym; sym = setNextItem (xstack->syms))
            debugFile->writeSymbol (sym);
        }

      if (istack)
        {
          for (sym = setFirstItem (istack->syms); sym; sym = setNextItem (istack->syms))
            debugFile->writeSymbol(sym);
        }
    }
}


void
outputDebugSymbols (void)
{
  if (getenv ("SDCC_DEBUG_VAR_STORAGE"))
    {
      dumpSymInfo ("Initialized", initialized);
      dumpSymInfo ("Code", code);
      dumpSymInfo ("Data", data);
      dumpSymInfo ("PData", pdata);
      dumpSymInfo ("XData", xdata);
      dumpSymInfo ("XIData", xidata);
      dumpSymInfo ("XInit", xinit);
      dumpSymInfo ("IData", idata);
      dumpSymInfo ("Bit", bit);
      dumpSymInfo ("Statics", statsg);
      dumpSymInfo ("SFR", sfr);
      dumpSymInfo ("SFRBits", sfrbit);
      dumpSymInfo ("Reg", reg);
      dumpSymInfo ("Generic", generic);
      dumpSymInfo ("Overlay", overlay);
      dumpSymInfo ("EEProm", eeprom);
      dumpSymInfo ("Home", home);
    }

  if (options.debug && debugFile)
    {
      symbol *sym;
           
      if(initialized)
        {
          for (sym = setFirstItem (initialized->syms); sym; sym = setNextItem (initialized->syms))
            debugFile->writeSymbol (sym);
        }

      if (data) 
        {
          for (sym = setFirstItem (data->syms); sym; sym = setNextItem (data->syms))
            debugFile->writeSymbol (sym);
        }

      if (idata)
        {
          for (sym = setFirstItem (idata->syms); sym; sym = setNextItem (idata->syms))
            debugFile->writeSymbol (sym);
        }

      if (bit)
        {
          for (sym = setFirstItem (bit->syms); sym; sym = setNextItem (bit->syms))
            debugFile->writeSymbol (sym);
        }

      if (pdata)
        {
          for (sym = setFirstItem (pdata->syms); sym; sym = setNextItem (pdata->syms))
            debugFile->writeSymbol (sym);
        }

      if (xdata)
        {
          for (sym = setFirstItem (xdata->syms); sym; sym = setNextItem (xdata->syms))
            debugFile->writeSymbol (sym);
        }

      if (port->genXINIT && xidata)
        {
          for (sym = setFirstItem (xidata->syms); sym; sym = setNextItem (xidata->syms))
            debugFile->writeSymbol (sym);        
        }

      if (sfr)
        {
          for (sym = setFirstItem (sfr->syms); sym; sym = setNextItem (sfr->syms))
            debugFile->writeSymbol (sym);
        }

      if (sfrbit)
        {
          for (sym = setFirstItem (sfrbit->syms); sym; sym = setNextItem (sfrbit->syms))
            debugFile->writeSymbol (sym);
        }

      if (home)
        {
          for (sym = setFirstItem (home->syms); sym; sym = setNextItem (home->syms))
            debugFile->writeSymbol (sym);
        }

      if (code)
        {
          for (sym = setFirstItem (code->syms); sym; sym = setNextItem (code->syms))
            debugFile->writeSymbol (sym);
        }

      if (statsg)
        {
          for (sym = setFirstItem (statsg->syms); sym; sym = setNextItem (statsg->syms))
            debugFile->writeSymbol (sym);
        }

      if (port->genXINIT && xinit)
        {
          for (sym = setFirstItem (xinit->syms); sym; sym = setNextItem (xinit->syms))
            debugFile->writeSymbol (sym);        
        }
    }

  return;
}


void
dumpSymInfo(const char *pcName, memmap *memItem)
{
  symbol *sym;

  if (!memItem)
    return;

  printf ("--------------------------------------------\n");
  printf (" %s\n", pcName);

  for (sym = setFirstItem (memItem->syms); sym; sym = setNextItem (memItem->syms))
    {          
      printf ("   %s, isReqv:%d, reqv:0x%p, onStack:%d, Stack:%d, nRegs:%d, [", 
              sym->rname, sym->isreqv, sym->reqv, sym->onStack, sym->stack, sym->nRegs);

      if (sym->reqv)
        {       
          int i;
          symbol *TempSym = OP_SYMBOL (sym->reqv);
           
          for (i = 0; i < 4; i++)           
            if (TempSym->regs[i])
              printf ("%s,", port->getRegName (TempSym->regs[i]));             
        }

      printf ("]\n");
    }

  printf ("\n");
}


/*------------------------------------------------------------------*/
/* emitDebuggerSymbol - call the port specific routine to associate */
/* the current code location with a debugger symbol                 */
/*------------------------------------------------------------------*/
void
emitDebuggerSymbol (const char * debugSym)
{
  if (port->debugger.emitDebuggerSymbol)
    port->debugger.emitDebuggerSymbol (debugSym);
}
