/*-------------------------------------------------------------------------
  SDCCdwarf2.c - generate DWARF2 debug information

             Written By -  Erik Petrich . epetrich@users.sourceforge.net (2004)

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


#include "common.h"
#include "SDCCdwarf2.h"

/*************************************************************
 *
 *
 *
 *
 *************************************************************/

extern set *includeDirsSet;

int dwOpenFile (const char *file);
int dwCloseFile (void);
int dwWriteFunction (symbol *pSym, iCode *ic);
int dwWriteEndFunction (symbol *pSym, iCode *ic, int offset);
int dwWriteLabel (symbol *pSym, iCode *ic);
int dwWriteScope (iCode *ic);
int dwWriteSymbol (symbol *pSym);
int dwWriteType (structdef *sdef, int block, int inStruct, const char *tag);
int dwWriteModule (const char *name);
int dwWriteCLine (iCode *ic);
int dwWriteALine (const char *module, int Line);
int dwWriteFrameAddress (const char *variable, struct reg_info *reg, int offset);
int dwWriteBasicSymbol (symbol *sym, int isStructSym, int isFunc);
     

DEBUGFILE dwarf2DebugFile = 
  {
    &dwOpenFile,
    &dwCloseFile,
    &dwWriteModule,
    &dwWriteFunction,
    &dwWriteEndFunction,
    &dwWriteLabel,
    &dwWriteScope,
    &dwWriteSymbol,
    &dwWriteType,
    &dwWriteCLine,
    &dwWriteALine,
    &dwWriteFrameAddress
  };

FILE *dwarf2FilePtr = NULL;
char *dwModuleName = NULL;
dwtag *dwRootTag = NULL;
dwtag *dwFuncTag = NULL;
dwtag *dwScopeTag = NULL;
hTab * dwAbbrevTable;
int dwAbbrevNum = 0;
hTab * dwTypeTagTable;
int dwRefNum = 0;
int dwScopeBlock = 0;
int dwScopeLevel = 0;
int dwDebugSymbol = 0;
//dwcfins * dwCIEins = NULL;
dwlocregion * dwFrameLastLoc = NULL;
dwloclist * dwRootLocList = NULL;
dwloclist * dwFrameLocList = NULL;
int dwLineBase = -5;
int dwLineRange = 15;
int dwLineOpcodeBase = 10;
set * dwFilenameSet = NULL;
dwline * dwLineFirst = NULL;
dwline * dwLineLast = NULL;


/*----------------------------------------------------------------------*/
/* dwNewDebugSymbol - returns the name for a new debug symbol           */
/*----------------------------------------------------------------------*/
static char *
dwNewDebugSymbol (void)
{
  char debugSym[SDCC_NAME_MAX];
        
  sprintf (debugSym, "S%s$%s$%d", dwModuleName, currFunc->name, dwDebugSymbol);
  dwDebugSymbol++;
  return Safe_strdup (debugSym);
}


/*----------------------------------------------------------------------*/
/* dwWriteByte - generate a single byte assembler constant in the form: */
/*                                                                      */
/*       .db label+offset  ; comment                                    */
/*                                                                      */
/* The label and comment parameters are optional                        */
/*----------------------------------------------------------------------*/
static void
dwWriteByte (const char * label, int offset, const char * comment)
{
  tfprintf (dwarf2FilePtr, "\t!db\t");
  if (label)
    {
      if (offset)
        fprintf (dwarf2FilePtr, "%s+%d", label, offset);
      else
        fprintf (dwarf2FilePtr, "%s", label);
    }
  else
    fprintf (dwarf2FilePtr, "%d", offset);
  
  if (comment)
    fprintf (dwarf2FilePtr, "\t;%s\n", comment);
  else
    fprintf (dwarf2FilePtr, "\n");
}

/*----------------------------------------------------------------------*/
/* dwWriteHalf - generate a two byte assembler constant in the form:    */
/*                                                                      */
/*       .dw label+offset  ; comment                                    */
/*                                                                      */
/* The label and comment parameters are optional                        */
/*----------------------------------------------------------------------*/
static void
dwWriteHalf (const char * label, int offset, char * comment)
{
  tfprintf (dwarf2FilePtr, "\t!dw\t");
  if (label)
    {
      if (offset)
        fprintf (dwarf2FilePtr, "%s+%d", label, offset);
      else
        fprintf (dwarf2FilePtr, "%s", label);
    }
  else
    fprintf (dwarf2FilePtr, "%d", offset);
  
  if (comment)
    fprintf (dwarf2FilePtr, "\t;%s\n", comment);
  else
    fprintf (dwarf2FilePtr, "\n");
}

/*----------------------------------------------------------------------*/
/* dwWriteWord - generate a four byte assembler constant in the form:   */
/*                                                                      */
/*       .dd label+offset  ; comment                                    */
/*                                                                      */
/* The label and comment parameters are optional                        */
/*----------------------------------------------------------------------*/
static void
dwWriteWord (const char * label, int offset, char * comment)
{
  /* FIXME: need to implement !dd pseudo-op in the assember. In the */
  /* meantime, we use dw with zero padding and hope the values fit  */
  /* in only 16 bits.                                               */
#if 0
  tfprintf (dwarf2FilePtr, "\t!dd\t");
  if (label)
    {
      if (offset)
        fprintf (dwarf2FilePtr, "%s+%d", label, offset);
      else
        fprintf (dwarf2FilePtr, "%s", label);
    }
  else
    fprintf (dwarf2FilePtr, "%d", offset);
#else
  tfprintf (dwarf2FilePtr, "\t!dw\t");
  if (port->little_endian)
    {
      if (label)
        {
          if (offset)
            fprintf (dwarf2FilePtr, "(%s+%d),0", label, offset);
          else
            fprintf (dwarf2FilePtr, "(%s),0", label);
        }
      else
        fprintf (dwarf2FilePtr, "%d,%d", offset, offset >> 16);
    }
  else
    {
      if (label)
        {
          if (offset)
            fprintf (dwarf2FilePtr, "0,(%s+%d)", label, offset);
          else
            fprintf (dwarf2FilePtr, "0,(%s)", label);
        }
      else
        fprintf (dwarf2FilePtr, "%d,%d", offset >> 16, offset);
    }
#endif
  
  if (comment)
    fprintf (dwarf2FilePtr, "\t;%s\n", comment);
  else
    fprintf (dwarf2FilePtr, "\n");
}


/*----------------------------------------------------------------------*/
/* dwWriteULEB128 - generate an unsigned variable length assembler      */
/*                  constant in the form:                               */
/*                                                                      */
/*       .uleb128 label+offset  ; comment                               */
/*                                                                      */
/* The label and comment parameters are optional                        */
/*----------------------------------------------------------------------*/
static void
dwWriteULEB128 (char * label, int offset, char * comment)
{
  tfprintf (dwarf2FilePtr, "\t.uleb128\t");
  if (label)
    {
      if (offset)
        fprintf (dwarf2FilePtr, "%s+%d", label, offset);
      else
        fprintf (dwarf2FilePtr, "%s", label);
    }
  else
    fprintf (dwarf2FilePtr, "%d", offset);
  
  if (comment)
    fprintf (dwarf2FilePtr, "\t;%s\n", comment);
  else
    fprintf (dwarf2FilePtr, "\n");
}

/*----------------------------------------------------------------------*/
/* dwWriteSLEB128 - generate a signed variable length assembler         */
/*                  constant in the form:                               */
/*                                                                      */
/*       .sleb128 label+offset  ; comment                               */
/*                                                                      */
/* The label and comment parameters are optional                        */
/*----------------------------------------------------------------------*/
static void
dwWriteSLEB128 (char * label, int offset, char * comment)
{
  tfprintf (dwarf2FilePtr, "\t.sleb128\t");
  if (label)
    {
      if (offset)
        fprintf (dwarf2FilePtr, "%s+%d", label, offset);
      else
        fprintf (dwarf2FilePtr, "%s", label);
    }
  else
    fprintf (dwarf2FilePtr, "%d", offset);
  
  if (comment)
    fprintf (dwarf2FilePtr, "\t;%s\n", comment);
  else
    fprintf (dwarf2FilePtr, "\n");
}


/*----------------------------------------------------------------------*/
/* dwSizeofULEB128 - return the size (in bytes) of an unsigned variable */
/*                   length constant                                    */
/*----------------------------------------------------------------------*/
static int
dwSizeofULEB128 (int unsigned value)
{
  int size = 0;
  
  do
    {
      value >>= 7;
      size++;
    }
  while (value);
  
  return size;
}

/*----------------------------------------------------------------------*/
/* dwSizeofSLEB128 - return the size (in bytes) of a signed variable    */
/*                   length constant                                    */
/*----------------------------------------------------------------------*/
static int
dwSizeofSLEB128 (int value)
{
  int size = 0;
  int negative = (value < 0);
  int sign;
  
  while (1)
    {
      size++;
      sign = value & 0x40;
      value >>= 7;
      if (negative)
        value |= (0x7f << (sizeof(int)*8 - 7));
      if ((value == 0 && !sign) || (value == -1 && sign))
        break;
    }
  
  return size;
}

/*----------------------------------------------------------------------*/
/* dwWriteString - generate a string constant in the form:              */
/*                                                                      */
/*       .ascii /string/  ; comment                                     */
/*                                                                      */
/* The comment parameter is optional. The string may contain any        */
/* non-null characters                                                  */
/*----------------------------------------------------------------------*/
static void
dwWriteString (const char * string, const char * comment)
{
  /* FIXME: need to safely handle nonalphanumeric data in string */
  
  tfprintf (dwarf2FilePtr, "\t!ascii\n", string);
  dwWriteByte (NULL, 0, comment);
}


/*----------------------------------------------------------------------*/
/* dwWriteAddress - generate an assembler constant in the form:         */
/*                                                                      */
/*       .dw label+offset  ; comment                                    */
/* or    .dd label+offset  ; comment                                    */
/*                                                                      */
/* depending on how the relevant ABI defines the address size (may be   */
/* larger than the CPU's actual address size). The label and comment    */
/* parameters are optional                                              */
/*----------------------------------------------------------------------*/
static void
dwWriteAddress (const char * label, int offset, char * comment)
{
  switch (port->debugger.dwarf.addressSize)
    {
    case 2:
      dwWriteHalf (label, offset, comment);
      break;
    case 4:
      dwWriteWord (label, offset, comment);
      break;
    default:
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__,
              "unsupported port->debugger.dwarf.addressSize");
    }
}


/*----------------------------------------------------------------------*/
/* dwWriteHalfDelta - generate a two byte assembler constant in the     */
/*                    form:                                             */
/*                                                                      */
/*       .dw offset+label1-label2                                       */
/*                                                                      */
/* The offset parameter is optional                                     */
/*----------------------------------------------------------------------*/
static void
dwWriteHalfDelta (char * label1, char * label2, int offset)
{
  if (offset)
    tfprintf (dwarf2FilePtr, "\t!dw\t%d+%s-%s\n", offset, label1, label2);
  else
    tfprintf (dwarf2FilePtr, "\t!dw\t%s-%s\n", label1, label2);
}

/*----------------------------------------------------------------------*/
/* dwWriteWordDelta - generate a four byte assembler constant in the    */
/*                    form:                                             */
/*                                                                      */
/*       .dd label1-label2                                              */
/*----------------------------------------------------------------------*/
static void 
dwWriteWordDelta (char * label1, char * label2)
{
  /* FIXME: need to implement !dd pseudo-op; this hack only */
  /* works for positive offsets of less than 64k            */
#if 0
  tfprintf (dwarf2FilePtr, "\t!dd\t%s-%s\n", label1,label2);
#else
  if (port->little_endian)
    {
      tfprintf (dwarf2FilePtr, "\t!dw\t%s-%s,%d\n", label1, label2, 0);
    }
  else
    {
      tfprintf (dwarf2FilePtr, "\t!dw\t%d,%s-%s\n", 0, label1, label2);
    }
#endif
}


/* disabled to eliminiate unused function warning */
#if 0 
/*----------------------------------------------------------------------*/
/* dwWriteAddressDelta - generate an assembler constant in the form:    */
/*                                                                      */
/*       .dw label1-label2                                              */
/* or    .dd label1-label2                                              */
/*                                                                      */
/* depending on how the relevant ABI defines the address size (may be   */
/* larger than the CPU's actual address size)                           */
/*----------------------------------------------------------------------*/
static void
dwWriteAddressDelta (char * label1, char * label2)
{
  switch (port->debugger.dwarf.addressSize)
    {
    case 2:
      dwWriteHalfDelta (label1, label2, 0);
      break;
    case 4:
      dwWriteWordDelta (label1, label2);
      break;
    default:
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__,
              "unsupported port->debugger.dwarf.addressSize");
    }
}

/*----------------------------------------------------------------------*/
/* dwWriteULEB128Delta - generate an unsigned variable byte assembler   */
/*                       constant in the form:                          */
/*                                                                      */
/*       .uleb128 offset+label1-label2                                  */
/*                                                                      */
/* The offset parameter is optional                                     */
/*----------------------------------------------------------------------*/
static void
dwWriteULEB128Delta (char * label1, char * label2, int offset)
{
  if (offset)
    tfprintf (dwarf2FilePtr, "\t.uleb128\t%d+%s-%s\n", offset, label1, label2);
  else
    tfprintf (dwarf2FilePtr, "\t.uleb128\t%s-%s\n", label1, label2);
}
#endif

/*------------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* dwNewLoc - allocates a new location expression node                  */
/*----------------------------------------------------------------------*/
dwloc *
dwNewLoc (int opcode, const char * label, int offset)
{
  dwloc * lp;
  
  lp = Safe_alloc (sizeof (dwloc));
  
  lp->opcode = opcode;
  lp->operand.label = label;
  lp->operand.offset = offset;

  return lp;
}

/*-------------------------------------------------------------------------*/
/* dwSizeofLoc - returns the size (in bytes) of a chain of location        */
/*               expression nodes as they would be encoded by dwWriteLoc() */
/*-------------------------------------------------------------------------*/
static int
dwSizeofLoc (dwloc * lp)
{
  int size = 0;
  
  while (lp)
    {
      size++;
      switch (lp->opcode)
        {
        case DW_OP_addr:
          size += port->debugger.dwarf.addressSize;
          break;
          
        case DW_OP_deref_size:
        case DW_OP_xderef_size:
        case DW_OP_pick:
        case DW_OP_const1u:
        case DW_OP_const1s:
          size += 1;
          break;
        
        case DW_OP_skip:
        case DW_OP_bra:
        case DW_OP_const2u:
        case DW_OP_const2s:
          size += 2;
          break;
        
        case DW_OP_const4u:
        case DW_OP_const4s:
          size += 4;
          break;
        
        case DW_OP_const8u:
        case DW_OP_const8s:
          size += 8;
          break;

        case DW_OP_piece:
        case DW_OP_regx:
        case DW_OP_plus_uconst:
          size += dwSizeofULEB128 (lp->operand.offset);
          break;
        
        case DW_OP_breg0:
        case DW_OP_breg1:
        case DW_OP_breg2:
        case DW_OP_breg3:
        case DW_OP_breg4:
        case DW_OP_breg5:
        case DW_OP_breg6:
        case DW_OP_breg7:
        case DW_OP_breg8:
        case DW_OP_breg9:
        case DW_OP_breg10:
        case DW_OP_breg11:
        case DW_OP_breg12:
        case DW_OP_breg13:
        case DW_OP_breg14:
        case DW_OP_breg15:
        case DW_OP_breg16:
        case DW_OP_breg17:
        case DW_OP_breg18:
        case DW_OP_breg19:
        case DW_OP_breg20:
        case DW_OP_breg21:
        case DW_OP_breg22:
        case DW_OP_breg23:
        case DW_OP_breg24:
        case DW_OP_breg25:
        case DW_OP_breg26:
        case DW_OP_breg27:
        case DW_OP_breg28:
        case DW_OP_breg29:
        case DW_OP_breg30:
        case DW_OP_breg31:
        case DW_OP_fbreg:
          size += dwSizeofSLEB128 (lp->operand.offset);
          break;
        }
      
      lp = lp->next;
    }
  
  return size;
}

/*------------------------------------------------------------------------*/
/* dwWriteLoc - writes a chain of location expression nodes               */
/*------------------------------------------------------------------------*/
static void
dwWriteLoc (dwloc *lp)
{
  while (lp)
    {
      dwWriteByte (NULL, lp->opcode, NULL);
      switch (lp->opcode)
        {
        case DW_OP_addr:
          dwWriteAddress (lp->operand.label, lp->operand.offset, NULL);
          break;
          
        case DW_OP_deref_size:
        case DW_OP_xderef_size:
        case DW_OP_pick:
        case DW_OP_const1u:
        case DW_OP_const1s:
          dwWriteByte (NULL, lp->operand.offset, NULL);
          break;
        
        case DW_OP_skip:
        case DW_OP_bra:
        case DW_OP_const2u:
        case DW_OP_const2s:
          dwWriteHalf (NULL, lp->operand.offset, NULL);
          break;
        
        case DW_OP_const4u:
        case DW_OP_const4s:
          dwWriteWord (NULL, lp->operand.offset, NULL);
          break;
        
        case DW_OP_piece:
        case DW_OP_regx:
        case DW_OP_plus_uconst:
          dwWriteULEB128 (NULL, lp->operand.offset, NULL);
          break;
        
        case DW_OP_breg0:
        case DW_OP_breg1:
        case DW_OP_breg2:
        case DW_OP_breg3:
        case DW_OP_breg4:
        case DW_OP_breg5:
        case DW_OP_breg6:
        case DW_OP_breg7:
        case DW_OP_breg8:
        case DW_OP_breg9:
        case DW_OP_breg10:
        case DW_OP_breg11:
        case DW_OP_breg12:
        case DW_OP_breg13:
        case DW_OP_breg14:
        case DW_OP_breg15:
        case DW_OP_breg16:
        case DW_OP_breg17:
        case DW_OP_breg18:
        case DW_OP_breg19:
        case DW_OP_breg20:
        case DW_OP_breg21:
        case DW_OP_breg22:
        case DW_OP_breg23:
        case DW_OP_breg24:
        case DW_OP_breg25:
        case DW_OP_breg26:
        case DW_OP_breg27:
        case DW_OP_breg28:
        case DW_OP_breg29:
        case DW_OP_breg30:
        case DW_OP_breg31:
        case DW_OP_fbreg:
          dwWriteSLEB128 (NULL, lp->operand.offset, NULL);
          break;
        }
        
      lp = lp->next;
    }
}

/*----------------------------------------------------------------------*/
/* dwNewLocList - allocates a new list of location expression node      */
/*----------------------------------------------------------------------*/
static dwloclist *
dwNewLocList (void)
{
  dwloclist * llp;
  
  llp = Safe_alloc (sizeof (dwloclist));
  
  return llp;
}

/*----------------------------------------------------------------------*/
/* dwSizeofLocRegion - returns the size (in bytes) of a chain of        */
/*                     location regions (inluding their location        */
/*                     expression nodes) as encoded by dwWriteLocLists  */
/*----------------------------------------------------------------------*/
static int
dwSizeofLocRegion (dwlocregion * lrp)
{
  int size = 0;
  
  while (lrp)
    {
      size += 2 * port->debugger.dwarf.addressSize;
      size += 2 + dwSizeofLoc (lrp->loc);
      lrp = lrp->next;
    }
  
  size += 2 * port->debugger.dwarf.addressSize;
  return size;
}

/*-----------------------------------------------------------------------*/
/* dwAssignLocListAddresses - assign the address offsets of the location */
/*                            lists so that they can be referenced from  */
/*                            the tag structure                          */
/*-----------------------------------------------------------------------*/
static void
dwAssignLocListAddresses (void)
{
  dwloclist * llp;
  int address = 0;

  llp = dwRootLocList;
  while (llp)
    {
      llp->baseOffset = address;
      address += dwSizeofLocRegion (llp->region);

      llp = llp->next;
    }
}

/*-----------------------------------------------------------------------*/
/* dwWriteLocLists - write all of the location lists in dwRootLocList to */
/*                   the .debug_loc section                              */
/*-----------------------------------------------------------------------*/
static void
dwWriteLocLists (void)
{
  dwlocregion * lrp;
  dwloclist * llp;

  tfprintf (dwarf2FilePtr, "\n\t!area\n", ".debug_loc (NOLOAD)");
  tfprintf (dwarf2FilePtr, "!slabeldef\n", "Ldebug_loc_start");

  llp = dwRootLocList;
  while (llp)
    {
      //fprintf (dwarf2FilePtr, "; baseOffset = 0x%x\n", llp->baseOffset);
      lrp = llp->region;
      while (lrp)
        {
          dwWriteAddress (lrp->startLabel, 0, NULL);
          dwWriteAddress (lrp->endLabel, 0, NULL);
          dwWriteHalf (NULL, dwSizeofLoc (lrp->loc), NULL);
          dwWriteLoc (lrp->loc);
          lrp = lrp ->next;
        }

      dwWriteAddress (NULL, 0, NULL);
      dwWriteAddress (NULL, 0, NULL);

      llp = llp->next;
    }
    
}


/*------------------------------------------------------------------------*/


/*----------------------------------------------------------------------*/
/* dwNewAttr - allocate a new tag attribute node                        */
/*----------------------------------------------------------------------*/
static dwattr *
dwNewAttr (int attr)
{
  dwattr * ap;
  
  ap = Safe_alloc ( sizeof (dwattr));
  ap->attr = attr;
  
  return ap;
}

/*----------------------------------------------------------------------*/
/* dwFreeAttr - deallocate a tag attribute node                         */
/*----------------------------------------------------------------------*/
static void
dwFreeAttr (dwattr * ap)
{
  Safe_free (ap);
}


/*-------------------------------------------------------------------------*/
/* dwNewAttrString - allocate a new tag attribute node with a string value */
/*-------------------------------------------------------------------------*/
static dwattr *
dwNewAttrString (int attr, const char * string)
{
  dwattr * ap;
  
  ap = dwNewAttr (attr);
  ap->form = DW_FORM_string;
  ap->val.string = string;
  return ap;
}


/*---------------------------------------------------------------------*/
/* dwNewAttrConst - allocate a new tag attribute node with an unsigned */
/*                  numeric constant value                             */
/*---------------------------------------------------------------------*/
static dwattr *
dwNewAttrConst (int attr, unsigned int data)
{
  dwattr * ap;
  
  ap = dwNewAttr (attr);
  if (data <= 0xffu)
    ap->form = DW_FORM_data1;
  else if (data <= 0xffffu)
    ap->form = DW_FORM_data2;
  else
    ap->form = DW_FORM_data4;
  
  ap->val.data = data;
  return ap;
}

/* disabled to eliminiate unused function warning */
#if 0
/*---------------------------------------------------------------------*/
/* dwNewAttrSignedConst - allocate a new tag attribute node with a     */
/*                        signed numeric constant value                */
/*---------------------------------------------------------------------*/
static dwattr *
dwNewAttrSignedConst (int attr, int data)
{
  dwattr * ap;
  
  ap = dwNewAttr (attr);
  if (data <= 0x7f && data >= -0x80)
    ap->form = DW_FORM_data1;
  else if (data <= 0xffff && data >= -0x8000)
    ap->form = DW_FORM_data2;
  else
    ap->form = DW_FORM_data4;
  
  ap->val.data = data;
  return ap;
}
#endif

/*---------------------------------------------------------------------*/
/* dwNewAttrFlag - allocate a new tag attribute node with a boolean    */
/*                 flag value (zero/non-zero)                          */
/*---------------------------------------------------------------------*/
static dwattr *
dwNewAttrFlag (int attr, int data)
{
  dwattr * ap;
  
  ap = dwNewAttr (attr);
  ap->form = DW_FORM_flag;
  
  ap->val.data = data;
  return ap;
}

/*---------------------------------------------------------------------*/
/* dwNewAttrAddrSymbol - allocate a new tag attribute node with the    */
/*                       address of a C symbol plus an offset          */
/*---------------------------------------------------------------------*/
static dwattr *
dwNewAttrAddrSymbol (int attr, symbol * sym, int offset)
{
  dwattr * ap;
  
  ap = dwNewAttr (attr);
  ap->form = DW_FORM_addr;
  
  ap->val.symaddr.label = sym->rname;
  ap->val.symaddr.offset = offset;
  return ap;
}

/*---------------------------------------------------------------------*/
/* dwNewAttrAddrLabel - allocate a new tag attribute node with the     */
/*                      address of an assembler label plus an offset   */
/*---------------------------------------------------------------------*/
static dwattr *
dwNewAttrAddrLabel (int attr, char * label, int offset)
{
  dwattr * ap;
  
  ap = dwNewAttr (attr);
  ap->form = DW_FORM_addr;
  
  ap->val.symaddr.label = label;
  ap->val.symaddr.offset = offset;
  return ap;
}

/*---------------------------------------------------------------------*/
/* dwNewAttrTagRef - allocate a new tag attribute node that references */
/*                   a tag node                                        */
/*---------------------------------------------------------------------*/
static dwattr *
dwNewAttrTagRef (int attr, dwtag * tp)
{
  dwattr * ap;
  
  ap = dwNewAttr (attr);
  ap->form = DW_FORM_ref4;
  
  ap->val.ref = tp;
  return ap;
}

/*---------------------------------------------------------------------*/
/* dwNewAttrLocRef - allocate a new tag attribute node that references */
/*                   a location list                                   */
/*---------------------------------------------------------------------*/
static dwattr *
dwNewAttrLocRef (int attr, dwloclist * llp)
{
  dwattr * ap;
  
  ap = dwNewAttr (attr);
  ap->form = DW_FORM_data4;
  
  ap->val.loclist = llp;
  return ap;
}

/*-----------------------------------------------------------------------*/
/* dwNewAttrLabelRef - allocate a new tag attribute node that references */
/*                     the address of an assembler label plus an offset  */
/*-----------------------------------------------------------------------*/
static dwattr *
dwNewAttrLabelRef (int attr, char * label, int offset)
{
  dwattr * ap;
  
  ap = dwNewAttr (attr);
  ap->form = DW_FORM_data4;
  
  ap->val.symaddr.label = label;
  ap->val.symaddr.offset = offset;
  return ap;
}

/*---------------------------------------------------------------------*/
/* dwNewAttrLoc - allocate a new tag attribute node for a chain of     */
/*                location expression nodes                            */
/*---------------------------------------------------------------------*/
dwattr *
dwNewAttrLoc (int attr, dwloc * lp)
{
  dwattr * ap;
  
  ap = dwNewAttr (attr);
  ap->form = DW_FORM_block1;
  ap->val.loc = lp;
  
  return ap;
}

/*---------------------------------------------------------------------*/
/* dwWriteAttr - write a tag attribute node                            */
/*---------------------------------------------------------------------*/
static void
dwWriteAttr (dwattr * ap)
{
  
  switch (ap->form)
    {
      case DW_FORM_addr:
        dwWriteAddress (ap->val.symaddr.label, ap->val.symaddr.offset, NULL);
        break;
      
      case DW_FORM_block:
        dwWriteULEB128 (NULL, dwSizeofLoc (ap->val.loc), NULL);
        dwWriteLoc (ap->val.loc);
        break;
      
      case DW_FORM_block1:
        dwWriteByte (NULL, dwSizeofLoc (ap->val.loc), NULL);
        dwWriteLoc (ap->val.loc);
        break;
      
      case DW_FORM_block2:
        dwWriteHalf (NULL, dwSizeofLoc (ap->val.loc), NULL);
        dwWriteLoc (ap->val.loc);
        break;
      
      case DW_FORM_block4:
        dwWriteWord (NULL, dwSizeofLoc (ap->val.loc), NULL);
        dwWriteLoc (ap->val.loc);
        break;
      
      case DW_FORM_data1:
      case DW_FORM_flag:
        dwWriteByte (NULL, ap->val.data, NULL);
        break;
      
      case DW_FORM_data2:
        dwWriteHalf (NULL, ap->val.data, NULL);
        break;
      
      case DW_FORM_data4:
        switch (ap->attr)
          {
          case DW_AT_stmt_list:
            dwWriteWord (ap->val.symaddr.label, ap->val.symaddr.offset, NULL);
            break;
          case DW_AT_location:
          case DW_AT_frame_base:
            dwWriteWord ("Ldebug_loc_start", ap->val.loclist->baseOffset, NULL);
            break;
          default:
            dwWriteWord (NULL, ap->val.data, NULL);
          }
        break;
      
      case DW_FORM_udata:
        dwWriteULEB128 (NULL, ap->val.data, NULL);
        break;
      
      case DW_FORM_sdata:
        dwWriteSLEB128 (NULL, ap->val.data, NULL);
        break;

      case DW_FORM_string:
        dwWriteString (ap->val.string, NULL);
        break;
      
      case DW_FORM_ref1:
        dwWriteByte (NULL, ap->val.ref->baseOffset, NULL);
        break;
      
      case DW_FORM_ref2:
        dwWriteHalf (NULL, ap->val.ref->baseOffset, NULL);
        break;
      
      case DW_FORM_ref4:
        dwWriteWord (NULL, ap->val.ref->baseOffset, NULL);
        break;
        
      default:
        werror (E_INTERNAL_ERROR, __FILE__, __LINE__,
                "unsupported DWARF form");
        exit (1);
    }
}

/*---------------------------------------------------------------------*/
/* dwSizeofAttr - returns the size (in bytes) of a tag attribute node  */
/*                as encoded by dwWriteAttr                            */
/*---------------------------------------------------------------------*/
static int
dwSizeofAttr (dwattr * ap)
{
  int size;
  
  switch (ap->form)
    {
      case DW_FORM_addr:
        return port->debugger.dwarf.addressSize;
      
      case DW_FORM_block:
        size = dwSizeofLoc (ap->val.loc);
        return size + dwSizeofULEB128 (size);
      
      case DW_FORM_block1:
        size = dwSizeofLoc (ap->val.loc);
        return size + 1;
      
      case DW_FORM_block2:
        size = dwSizeofLoc (ap->val.loc);
        return size + 2;
      
      case DW_FORM_block4:
        size = dwSizeofLoc (ap->val.loc);
        return size + 4;
      
      case DW_FORM_data1:
      case DW_FORM_flag:
        return 1;
      
      case DW_FORM_data2:
        return 2;
      
      case DW_FORM_data4:
        return 4;
      
      case DW_FORM_udata:
        return dwSizeofULEB128 (ap->val.data);
      
      case DW_FORM_sdata:
        return dwSizeofSLEB128 (ap->val.data);

      case DW_FORM_string:
        return 1 + strlen (ap->val.string);
      
      case DW_FORM_ref1:
        return 1;
      
      case DW_FORM_ref2:
        return 2;
      
      case DW_FORM_ref4:
        return 4;
        
      default:
        werror (E_INTERNAL_ERROR, __FILE__, __LINE__,
                "unsupported DWARF form");
        exit (1);
    }
    
}


/*---------------------------------------------------------------------*/
/* dwFindAttr - for a tag node, return a pointer to a particular       */
/*              attribute node, or NULL if not found                   */
/*---------------------------------------------------------------------*/
static dwattr *
dwFindAttr (dwtag * tp, int attr)
{
  dwattr * ap;
  
  ap = tp->attribs;
  while (ap)
    {
      if (ap->attr == attr)
        return ap;
      ap = ap->next;
    }
  
  return NULL;
}



/*------------------------------------------------------------------------*/


/*----------------------------------------------------------------------*/
/* dwNewTag - allocate a new tag node                                   */
/*----------------------------------------------------------------------*/
static dwtag *
dwNewTag (int tag)
{
  dwtag * tp;
  
  tp = Safe_alloc ( sizeof (dwtag));
  tp->tag = tag;
  
  return tp;
}

/*----------------------------------------------------------------------*/
/* dwAddTagAttr - add an attribute to a tag                             */
/*----------------------------------------------------------------------*/
static void
dwAddTagAttr (dwtag * tp, dwattr * ap)
{
  dwattr * curap;
  
  if (!tp->attribs)
    tp->attribs = ap;
  else if (ap->attr < tp->attribs->attr)
    {
      ap->next = tp->attribs;
      tp->attribs = ap;
    }
  else
    {
      curap = tp->attribs;
      while (curap->next && curap->next->attr < ap->attr)
        curap = curap->next;
      ap->next = curap->next;
      curap->next = ap;
    }
}

/*----------------------------------------------------------------------*/
/* dwSetTagAttr - repleace an existing attribute of a tag with a new    */
/*                attribute or add if non-existent                      */
/*----------------------------------------------------------------------*/
static void
dwSetTagAttr (dwtag *tp, dwattr * ap)
{
  dwattr * curap;
  
  curap = dwFindAttr (tp, ap->attr);
  if (curap)
    {
      ap->next = curap->next;
      *curap = *ap;
      dwFreeAttr (ap);
    }
  else
    dwAddTagAttr (tp, ap);
}


/*----------------------------------------------------------------------*/
/* dwAddTagChild - add a tag as a child of another tag                  */
/*----------------------------------------------------------------------*/
static dwtag *
dwAddTagChild (dwtag * parent, dwtag * child)
{
  child->parent = parent;
  if (parent->lastChild)
    {
      parent->lastChild->siblings = child;
      parent->lastChild = child;
    }
  else
    {
      parent->firstChild = child;
      parent->lastChild = child;
    }
  return parent;
}

/*----------------------------------------------------------------------*/
/* dwMatchTagAttr - returns true if two tags are equal in value,        */
/*                  attributes, and offspring status (the child tags    */
/*                  need not match, but they must both have children or */
/*                  both not have children)                             */
/*----------------------------------------------------------------------*/
static int
dwMatchTagAttr (const void * tp1v, const void * tp2v)
{
  const dwtag * tp1 = tp1v;
  const dwtag * tp2 = tp2v;
  dwattr * ap1;
  dwattr * ap2;

  if (!tp1 || !tp2)
    return 0;
    
  ap1 = tp1->attribs;
  ap2 = tp2->attribs;

  if (tp1->tag != tp2->tag)
    return 0;
  
  if (tp1->firstChild && !tp2->lastChild)
    return 0;
  if (!tp1->firstChild && tp2->lastChild)
    return 0;
    
  while (ap1 && ap2)
    {
      if (ap1->attr != ap2->attr)
        return 0;
      if (ap1->form != ap2->form)
        return 0;

      ap1 = ap1->next;
      ap2 = ap2->next;
    }
  
  return 1;
}

/*----------------------------------------------------------------------*/
/* dwHashTag - return a hash code for a tag based on its value and      */
/*             attributes                                               */
/*----------------------------------------------------------------------*/
static int
dwHashTag (dwtag * tp)
{
  dwattr * ap = tp->attribs;
  int hash = tp->tag;

  while (ap)
    {
      hash = (hash << 6) ^ ((hash >> 11) & 0xff);
      hash ^= (ap->attr) | (ap->form << 8);
      
      ap = ap->next;
    }  
  if (hash<0)
    return -hash;
  else
    return hash;
}

/*----------------------------------------------------------------------*/
/* dwTraverseTag - perform a depth-first preorder traversal of a tag    */
/*                 tree, calling the user function at each node. The    */
/*                 user function is also called with a NULL tag pointer */
/*                 after the last sibling of each immediate family is   */
/*                 processed.                                           */
/*----------------------------------------------------------------------*/
static int
dwTraverseTag (dwtag *tp, int (*somefunc)(dwtag *tp, void * info), void * info)
{
  int rvalue = 0;
  
  while (tp)
    {
      rvalue += (*somefunc)(tp, info);
      if (tp->firstChild)
        rvalue += dwTraverseTag (tp->firstChild, somefunc, info);
      tp = tp->siblings;
    }
  rvalue += (*somefunc)(NULL, info);
  
  return rvalue;
}

/*----------------------------------------------------------------------*/
/* dwAssignAbbrev - find a matching abbreviation for a tag or create a  */
/*                  a new one and assign it                             */
/*----------------------------------------------------------------------*/
static int
dwAssignAbbrev (dwtag *tp, void *info)
{
  dwtag * oldtp;
  int * anp = info;     /* pointer to current abbreviation number */
  int key;
  
  if (!tp)
    return 0;

  key = dwHashTag (tp) % dwAbbrevTable->size;
  oldtp = hTabFindByKey (dwAbbrevTable, key, tp, dwMatchTagAttr);
  if (oldtp)
    {
      tp->abbrev = oldtp->abbrev;
      return 0;
    }
  else
    {
      tp->abbrev = ++(*anp);
      hTabAddItemLong (&dwAbbrevTable, key, tp, tp);
      return 1;
    }
}

/*-----------------------------------------------------------------------*/
/* dwWriteAbbrevs - write the abbreviations to the .debug_abbrev section */
/*-----------------------------------------------------------------------*/
static void
dwWriteAbbrevs (void)
{
  dwtag * tp;
  dwattr * ap;
  int key;
  
  tfprintf (dwarf2FilePtr, "\n\t!area\n", ".debug_abbrev (NOLOAD)");
  tfprintf (dwarf2FilePtr, "!slabeldef\n", "Ldebug_abbrev");

  tp = hTabFirstItem (dwAbbrevTable, &key);
  for (; tp; tp = hTabNextItem (dwAbbrevTable, &key))
    {
      dwWriteULEB128 (NULL, tp->abbrev, NULL);
      dwWriteULEB128 (NULL, tp->tag, NULL);
      dwWriteByte (NULL, tp->firstChild ? DW_CHILDREN_yes : DW_CHILDREN_no,
                   NULL);
      ap = tp->attribs;
      while (ap)
        {
          dwWriteULEB128 (NULL, ap->attr, NULL);
          dwWriteULEB128 (NULL, ap->form, NULL);
          ap = ap->next;
        }
      dwWriteULEB128 (NULL, 0, NULL);
      dwWriteULEB128 (NULL, 0, NULL);
      
    }
  dwWriteULEB128 (NULL, 0, NULL);
  
  hTabDeleteAll (dwAbbrevTable);
}



/*-----------------------------------------------------------------------*/
/* dwWriteTag - write the encoded tag information                        */
/*-----------------------------------------------------------------------*/
static int
dwWriteTag (dwtag *tp, void *info)
{
  dwattr * ap;

  if (!tp)
    {
      /* mark the end of this series of siblings */
      dwWriteULEB128 (NULL, 0, NULL);
      return 0;
    }

  //fprintf (dwarf2FilePtr, "; baseOffset = 0x%x\n", tp->baseOffset);
  
  /* write the tag abbreviation */
  dwWriteULEB128 (NULL, tp->abbrev, NULL);
  
  /* write the values of the attributes */
  ap = tp->attribs;
  while (ap)
    {
      dwWriteAttr (ap);
      ap = ap->next;
    }
    
  return 1;
}


/*-----------------------------------------------------------------------*/
/* dwWriteTags - write all the tags to the .debug_info section           */
/*-----------------------------------------------------------------------*/
static void
dwWriteTags (void)
{  
  tfprintf (dwarf2FilePtr, "\n\t!area\n", ".debug_info (NOLOAD)");
  
  dwWriteWordDelta ("Ldebug_info_end", "Ldebug_info_start");
  
  tfprintf (dwarf2FilePtr, "!slabeldef\n", "Ldebug_info_start");
  
  dwWriteHalf (NULL, 2, NULL); /* DWARF version */
  
  dwWriteWord ("Ldebug_abbrev", 0, NULL);
    
  dwWriteByte (NULL, port->debugger.dwarf.addressSize, NULL);
    
  dwTraverseTag (dwRootTag, dwWriteTag, NULL);
  
  dwWriteULEB128 (NULL, 0, NULL);
  
  tfprintf (dwarf2FilePtr, "!slabeldef\n", "Ldebug_info_end");

}

/*-----------------------------------------------------------------------*/
/* dwAssignTagAddress - assign the current address to the current tag.   */
/*                      Compute the next address based on the tag size   */
/*-----------------------------------------------------------------------*/
static int
dwAssignTagAddress (dwtag *tp, void *info)
{
  int * tap = info;
  dwattr * ap;

  if (!tp)
    {
      *tap += 1;
      return 0;
    }
      
  tp->baseOffset = *tap;

  *tap += dwSizeofULEB128 (tp->abbrev);
  
  ap = tp->attribs;
  while (ap)
    {
      *tap += dwSizeofAttr (ap);
      ap = ap->next;
    }
    
  return 0;
}

/*-----------------------------------------------------------------------*/
/* dwAddSibAttr - if a tag has children and a sibling, add a sibling     */
/*                attribute (it allows debuggers to jump to the sibling  */
/*                and skip the child data)                               */
/*-----------------------------------------------------------------------*/
static int
dwAddSibAttr (dwtag *tp, void *info)
{
  if (!tp)
    return 0;
  if (tp == dwRootTag)
    return 0;
  
  if (tp->firstChild && tp->siblings)
    dwAddTagAttr (tp, dwNewAttrTagRef (DW_AT_sibling, tp->siblings));

  return 0;
}

/*-----------------------------------------------------------------------*/
/* dwDeleteTagAttr - given a pointer to an attribute type, delete any    */
/*                   matching attribute                                  */
/*-----------------------------------------------------------------------*/
static int
dwDeleteTagAttr (dwtag *tp, void *info)
{
  int attr = *((int *) info);
  dwattr * ap;
  
  if (!tp)
    return 0;

  ap = tp->attribs;
  if (ap && ap->attr == attr)
    {
      tp->attribs = ap->next;
      return 1;
    }
  
  while (ap)
    {
      if (ap->next && ap->next->attr == attr)
        {
          ap->next = ap->next->next;
          return 1;
        }
      ap = ap->next;
    }
  
  return 0;
}


/*------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* dwWritePubnames - write all the public names to the .debug_pubnames   */
/*                   section. Externally visible functions and variables */
/*                   are considered to have public names.                */
/*-----------------------------------------------------------------------*/
static void
dwWritePubnames (void)
{
  dwtag * tp;
  dwattr * ap1;
  dwattr * ap2;
  
  tfprintf (dwarf2FilePtr, "\n\t!area\n", ".debug_pubnames (NOLOAD)");
  
  dwWriteWordDelta ("Ldebug_pubnames_end", "Ldebug_pubnames_start");
  
  tfprintf (dwarf2FilePtr, "!slabeldef\n", "Ldebug_pubnames_start");
  
  dwWriteHalf (NULL, 2, NULL); /* DWARF version */
  
  dwWriteWord ("Ldebug_info_start-4", 0, NULL);
  dwWriteWordDelta ("4+Ldebug_info_end", "Ldebug_info_start");

  if (dwRootTag && dwRootTag->firstChild)
    {
      tp = dwRootTag->firstChild;
      while (tp)
        {
          if (tp->tag == DW_TAG_variable || tp->tag == DW_TAG_subprogram)
            {
              /* If it has a name and is externally visible, it's a pubname */
              ap1 = dwFindAttr (tp, DW_AT_external);
              ap2 = dwFindAttr (tp, DW_AT_name);
              if (ap1 && ap1->val.data && ap2)
                {
                  dwWriteWord (NULL, tp->baseOffset, NULL);
                  dwWriteString (ap2->val.string, NULL);
                }
            }
        
          tp = tp->siblings;
        }
    }  
  dwWriteWord (NULL, 0, NULL);
  tfprintf (dwarf2FilePtr, "!slabeldef\n", "Ldebug_pubnames_end");
}

/*------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* dwFindFileIndex - find the index of a filename in dwFilenameSet; if   */
/*                   it does not exist, it is added                      */
/*-----------------------------------------------------------------------*/
static int
dwFindFileIndex (char * filename)
{
  char * includeDir;
  dwfile * srcfile;
  int fileIndex = 1;
  int dirIndex = 1;

  /* Just do a linear search for the file. There should be hardly */
  /* a penalty since 1) most calls search for the first file, and */
  /* 2) the file set is usually small (often just 1 item)         */
  for (srcfile = setFirstItem (dwFilenameSet);
       srcfile;
       srcfile = setNextItem(dwFilenameSet), fileIndex++ )
    {
      if (!strcmp (srcfile->name, filename))
        return fileIndex;
    }

  for (includeDir = setFirstItem (includeDirsSet);
       includeDir;
       includeDir = setNextItem(includeDirsSet), dirIndex++ )
    {
      if (!strncmp (includeDir, filename, strlen (includeDir))
          && strlen (filename) > strlen (includeDir))
        {
          if (IS_DIR_SEPARATOR(filename[strlen (includeDir)]))
            break;
        }
    }
  if (!includeDir)
    dirIndex = 0;

  srcfile = Safe_alloc (sizeof (dwfile));
  srcfile->name = filename;
  srcfile->dirIndex = dirIndex;
  srcfile->timestamp = 0;
  srcfile->length = 0;

  addSet (&dwFilenameSet, srcfile);
  return fileIndex;
}

/*-----------------------------------------------------------------------*/
/* dwWriteLineNumber - write line number (and related position info) to  */
/*                     address corespondence data for a single node      */
/*-----------------------------------------------------------------------*/
static void
dwWriteLineNumber (dwline * lp)
{
  static int curFileIndex = 1;
  static int curLine = 1;
  static char * curLabel = NULL;
  static int curOffset = 0;
  int deltaLine = lp->line - curLine;
  int deltaAddr = lp->offset - curOffset;
  int deltaAddrValid = curLabel && lp->label && !strcmp (lp->label, curLabel);

  //fprintf (dwarf2FilePtr, "; line %d\n", lp->line);
  if (lp->begin_sequence)
    {
      curFileIndex = 1;
      curLine = 1;
      curLabel = NULL;
      curOffset = 0;
      
      if (lp->end_sequence)
        return;
    }
  
  if (lp->fileIndex != curFileIndex)
    {
      dwWriteByte (NULL, DW_LNS_set_file, NULL);
      dwWriteULEB128 (NULL, lp->fileIndex, NULL);
      curFileIndex = lp->fileIndex;
    }
  
  if (lp->basic_block)
    {
      dwWriteByte (NULL, DW_LNS_set_basic_block, NULL);
    }
  
  if (lp->begin_sequence)
    {
      dwWriteByte (NULL, 0, NULL);
      dwWriteULEB128 (NULL, 1+port->debugger.dwarf.addressSize, NULL);
      dwWriteByte (NULL, DW_LNE_set_address, NULL);
      dwWriteAddress (lp->label, lp->offset, NULL);
      curLabel = lp->label;
      curOffset = lp->offset;

      dwWriteByte (NULL, DW_LNS_advance_line, NULL);
      dwWriteSLEB128 (NULL, lp->line - 1, NULL);
      curLine = lp->line;

      dwWriteByte (NULL, DW_LNS_copy, NULL);
    }
  else if (lp->end_sequence)
    {
      if (deltaAddrValid)
        {
          dwWriteByte (NULL, DW_LNS_advance_pc, NULL);
          dwWriteULEB128 (NULL, deltaAddr, NULL);
        }
      else
        {
          dwWriteByte (NULL, DW_LNS_fixed_advance_pc, NULL);
          dwWriteHalfDelta (lp->label, curLabel, lp->offset-curOffset);
          curLabel = lp->label;
          curOffset = lp->offset;
        }
      
      dwWriteByte (NULL, 0, NULL);
      dwWriteULEB128 (NULL, 1, NULL);
      dwWriteByte (NULL, DW_LNE_end_sequence, NULL);
    }
  else
    {
      int usedSpecial = 0;
      
      /* Metrowerks CW08 V3.0 gets confused by this. Just use the long */
      /* encoding until we can find a more compatible phrasing.        */
      #if 0
      if (deltaLine >= dwLineBase && deltaLine < (dwLineBase+dwLineRange))
        {
          int opcode;
          
          /* try to build a "special" opcode */
          opcode = dwLineOpcodeBase + (deltaLine - dwLineBase);
          if (deltaAddrValid)
            opcode += deltaAddr*dwLineRange;
          
          if (opcode >= dwLineOpcodeBase && opcode <= 255)
            {
              /* ok, we can use a "special" opcode */
              
              /* If the deltaAddr value was symbolic, it can't be part */
              /* of the "special" opcode, so encode it seperately      */
              if (!deltaAddrValid)
                {
                  dwWriteByte (NULL, DW_LNS_advance_pc, NULL);
                  dwWriteULEB128Delta (lp->label, curLabel, lp->offset-curOffset);
                  curLabel = lp->label;
                  curOffset = lp->offset;
                }

              /* Write the "special" opcode */        
              dwWriteByte (NULL, opcode, NULL);
              curLine = lp->line;
              usedSpecial = 1;
            }
        }
      #endif
      
      /* If we couldn't use the "special" opcode, we will have to */
      /* encode this the long way.                                */
      if (!usedSpecial)
        {
          dwWriteByte (NULL, DW_LNS_fixed_advance_pc, NULL);
          dwWriteHalfDelta (lp->label, curLabel, lp->offset-curOffset);
          curLabel = lp->label;
          curOffset = lp->offset;
        
          dwWriteByte (NULL, DW_LNS_advance_line, NULL);
          dwWriteSLEB128 (NULL, deltaLine, NULL);
          curLine = lp->line;
          
          dwWriteByte (NULL, DW_LNS_copy, NULL);
        }
        
    }
    
}

/*-----------------------------------------------------------------------*/
/* dwWriteLineNumbers - write all the source line number position data   */
/*                      to the .debug_line section                       */
/*-----------------------------------------------------------------------*/
static void
dwWriteLineNumbers (void)
{
  char * includeDir;
  dwfile * srcfile;
  dwline * lp;
  
  tfprintf (dwarf2FilePtr, "\n\t!area\n", ".debug_line (NOLOAD)");
  
  dwWriteWordDelta ("Ldebug_line_end", "Ldebug_line_start");
  
  tfprintf (dwarf2FilePtr, "!slabeldef\n", "Ldebug_line_start");
  
  dwWriteHalf (NULL, 2, NULL); /* DWARF version */

  dwWriteWordDelta ("Ldebug_line_stmt-6", "Ldebug_line_start");

  dwWriteByte (NULL, 1, NULL); /* we track everything in 1 byte increments */
  
  dwWriteByte (NULL, 1, NULL); /* assume every line is a new statement */

  dwWriteByte (NULL, dwLineBase, NULL);
  dwWriteByte (NULL, dwLineRange, NULL);
  
  dwWriteByte (NULL, 9+1, NULL);  /* there are 9 standard opcodes */
  
  dwWriteByte (NULL, 0, NULL);  /* number of DW_LNS_copy arguments */
  dwWriteByte (NULL, 1, NULL);  /* number of DW_LNS_advance_pc arguments */
  dwWriteByte (NULL, 1, NULL);  /* number of DW_LNS_advance_line arguments */
  dwWriteByte (NULL, 1, NULL);  /* number of DW_LNS_set_file arguments */
  dwWriteByte (NULL, 1, NULL);  /* number of DW_LNS_set_column arguments */
  dwWriteByte (NULL, 0, NULL);  /* number of DW_LNS_negate_stmt arguments */
  dwWriteByte (NULL, 0, NULL);  /* number of DW_LNS_set_basic_block arguments */
  dwWriteByte (NULL, 0, NULL);  /* number of DW_LNS_const_add_pc arguments */
  dwWriteByte (NULL, 1, NULL);  /* number of DW_LNS_fixed_advance_pc arguments */

  /* Write the list of source directories searched */
  for (includeDir = setFirstItem (includeDirsSet);
       includeDir;
       includeDir = setNextItem(includeDirsSet) )
    dwWriteString (includeDir, NULL);
  dwWriteByte (NULL, 0, NULL);
  
  /* Write the list of source files used */
  for (srcfile = setFirstItem (dwFilenameSet);
       srcfile;
       srcfile = setNextItem(dwFilenameSet) )
    {
      dwWriteString (srcfile->name, NULL);
      dwWriteULEB128 (NULL, srcfile->dirIndex, NULL);
      dwWriteULEB128 (NULL, srcfile->timestamp, NULL);
      dwWriteULEB128 (NULL, srcfile->length, NULL);
    }
  dwWriteByte (NULL, 0, NULL);
  
  tfprintf (dwarf2FilePtr, "!slabeldef\n", "Ldebug_line_stmt");

  lp = dwLineFirst;
  if (lp)
    lp->begin_sequence = 1;
  while (lp)
    {
      dwWriteLineNumber (lp);
      if (lp->end_sequence && lp->next)
        lp->next->begin_sequence = 1;
      lp = lp->next;
    }
    
  tfprintf (dwarf2FilePtr, "!slabeldef\n", "Ldebug_line_end");
}

/*------------------------------------------------------------------------*/


/* I have disabled all of this canonical frame address related code */
/* until I better understand this part of the DWARF2 spec. -- EEP   */
#if 0
static void
dwWriteCFAinstructions (dwcfins *ip)
{
  dwcfop * op = ip->first;
  
  while (op)
    {
      dwWriteByte (NULL, op->opcode, NULL);
      switch (op->opcode >> 6)
        {
        case 0:
          switch (op->opcode)
            {
            case DW_CFA_set_loc:
              dwWriteAddress (NULL, op->label, op->operand1);
              break;
            
            case DW_CFA_advance_loc1:
              dwWriteByte (NULL, op->operand1, NULL);
              break;
            
            case DW_CFA_advance_loc2:
              dwWriteHalf (NULL, op->operand1, NULL);
              break;
            
            case DW_CFA_advance_loc4:
              dwWriteWord (NULL, op->operand1, NULL);
              break;
            
            case DW_CFA_def_cfa:
            case DW_CFA_register:
            case DW_CFA_offset_extended:
              dwWriteULEB128 (NULL, op->operand1, NULL);
              dwWriteULEB128 (NULL, op->operand2, NULL);
              break;
            
            case DW_CFA_undefined:
            case DW_CFA_same_value:
            case DW_CFA_def_cfa_register:
            case DW_CFA_def_cfa_offset:
            case DW_CFA_restore_extended:
              dwWriteULEB128 (NULL, op->operand1, NULL);
              break;
            }
          break;
        
        case DW_CFA_restore >> 6:
        case DW_CFA_advance_loc >> 6:
          break;
        
        case DW_CFA_offset >> 6:
          dwWriteULEB128 (NULL, op->operand1, NULL);
          break;
        }
      op = op->next;
    }
}

static int
dwSizeofCFAinstructions (dwcfins *ip)
{
  int size = 0;
  dwcfop * op = ip->first;
  
  while (op)
    {
      size++;
      switch (op->opcode >> 6)
        {
        case 0:
          switch (op->opcode)
            {
            case DW_CFA_set_loc:
              size += port->debugger.dwarf.addressSize;
              break;
            
            case DW_CFA_advance_loc1:
              size += 1;
              break;
            
            case DW_CFA_advance_loc2:
              size += 2;
              break;
            
            case DW_CFA_advance_loc4:
              size += 4;
              break;
            
            case DW_CFA_def_cfa:
            case DW_CFA_register:
            case DW_CFA_offset_extended:
              size += dwSizeofULEB128 (op->operand1);
              size += dwSizeofULEB128 (op->operand2);
              break;
            
            case DW_CFA_undefined:
            case DW_CFA_same_value:
            case DW_CFA_def_cfa_register:
            case DW_CFA_def_cfa_offset:
            case DW_CFA_restore_extended:
              size += dwSizeofULEB128 (op->operand1);
              break;
            }
          break;
        
        case DW_CFA_restore >> 6:
        case DW_CFA_advance_loc >> 6:
          break;
        
        case DW_CFA_offset >> 6:
          size += dwSizeofULEB128 (op->operand1);
          break;
        }
      op = op->next;
    }
  return size;
}

static dwcfop *
dwNewCFop (int opcode)
{
  dwcfop * op;
  
  op = Safe_alloc (sizeof (dwcfop));
  op->opcode = opcode;
  
  return op;
}

static dwcfins *
dwNewCFins (void)
{
  return (dwcfins *) Safe_alloc (sizeof (dwcfins));
}

static void
dwAddCFinsOp (dwcfins * ip, dwcfop *op)
{
  if (ip->last)
    ip->last->next = op;
  else
    ip->first = op;
  ip->last = op;
}

static dwcfins *
dwGenCFIins (void)
{
  dwcfins * ip;
  dwcfop * op;
  int i;
  
  ip = dwNewCFins ();
  
  /* Define the CFA as the top of the stack at function start. */
  /* The return address is then at cfa+0                       */
  op = dwNewCFop (DW_CFA_def_cfa);
  op->operand1 = port->debugger.dwarf.regNumSP;
  op->operand2 = port->debugger.dwarf.offsetSP;
  dwAddCFinsOp (ip, op);

  op = dwNewCFop (DW_CFA_offset + port->debugger.dwarf.regNumRet);
  op->operand1 = 0;
  dwAddCFinsOp (ip, op);

  if (port->debugger.dwarf.cfiUndef)
    for (i=0; i < port->debugger.dwarf.cfiUndef->size; i++)
      {
        if (bitVectBitValue (port->debugger.dwarf.cfiUndef, i))
          {
            op = dwNewCFop (DW_CFA_undefined);
            dwAddCFinsOp (ip, op);
          }
    }
  
  if (port->debugger.dwarf.cfiSame)
    for (i=0; i < port->debugger.dwarf.cfiSame->size; i++)
      {
        if (bitVectBitValue (port->debugger.dwarf.cfiSame, i))
          {
            op = dwNewCFop (DW_CFA_undefined);
            dwAddCFinsOp (ip, op);
          }
      }

  return ip;
}


static void
dwWriteFDE (dwfde * fp)
{
  dwWriteWord (NULL, dwSizeofCFAinstructions(fp->ins) + 4
                + port->debugger.dwarf.addressSize * 2, NULL);
  
  dwWriteWord ("Ldebug_CIE_start-4", 0, NULL);
  
  dwWriteAddressDelta (fp->endLabel, fp->startLabel);
  
  dwWriteCFAinstructions (fp->ins);
  
}

static void
dwWriteFrames (void)
{  
  tfprintf (dwarf2FilePtr, "\n\t!area\n", ".debug_frame (NOLOAD)");

  /* FIXME: these two dw should be combined into a dd */
  tfprintf (dwarf2FilePtr, "\t!dw\t0\n");
  tfprintf (dwarf2FilePtr, "\t!dw\t%s\n", "Ldebug_CIE_end-Ldebug_CIE_start");

  tfprintf (dwarf2FilePtr, "!slabeldef\n", "Ldebug_CIE_start");
  
  tfprintf (dwarf2FilePtr, "\t!dw\t0xffff\n");
  tfprintf (dwarf2FilePtr, "\t!dw\t0xffff\n");  /* CIE_id */

  tfprintf (dwarf2FilePtr, "\t!db\t%d\n",1);    /* CIE version number */

  tfprintf (dwarf2FilePtr, "\t!db\t%d\n",0);    /* augmentation (none) */

  dwWriteULEB128 (NULL, 1, NULL);       /* code alignment factor */
  
  dwWriteSLEB128 (NULL, (port->stack.direction > 0) ? -1 : 1, NULL); /* data alignment factor */
  
  dwWriteByte (NULL, port->debugger.dwarf.regNumRet, NULL);
  
  if (!dwCIEins)
    {
      #if 0
      if (port->debugger.dwarf.genCFIins)
        dwCIEins = port->debugger.dwarf.genCFIins ();
      else
      #endif
        dwCIEins = dwGenCFIins ();
    }
  dwWriteCFAinstructions (dwCIEins);
  
  tfprintf (dwarf2FilePtr, "!slabeldef\n", "Ldebug_CIE_end");
}
#endif




/*------------------------------------------------------------------------*/




/*-----------------------------------------------------------------------*/
/* dwHashType - return a hash code for a type chain                      */
/*-----------------------------------------------------------------------*/
static int
dwHashType (sym_link * type)
{
  int hash = 0;

  while (type)
    {
      hash = (hash << 5) ^ ((hash >> 8) & 0xff);
      if (IS_DECL (type))
        {
          hash ^= DCL_TYPE (type);
        }
      else
        {
          hash ^= SPEC_NOUN (type)
               | (SPEC_CONST (type) << 4)
               | (SPEC_VOLATILE (type) << 5)
               | (SPEC_LONG (type) << 6);
        }
      
      type = type->next;
    }  

  if (hash<0)
    return -hash;
  else
    return hash;
}

/*-----------------------------------------------------------------------*/
/* dwMatchType - returns true if two types match exactly (including type */
/*               qualifiers)                                             */
/*-----------------------------------------------------------------------*/
static int
dwMatchTypes (const void * type1v, const void * type2v)
{
  sym_link * type1 = (sym_link *)type1v;
  sym_link * type2 = (sym_link *)type2v;
  
  if (!type1 || !type2)
    return 0;
  
  while (type1 && type2)
    {
      if (IS_SPEC(type1))
        {
          if (IS_SPEC (type2))
            {
              if (SPEC_NOUN (type1) != SPEC_NOUN (type2))
                return 0;
              if (SPEC_NOUN (type1) == V_STRUCT
                  && SPEC_STRUCT (type1) != SPEC_STRUCT (type2))
                return 0;
              if (SPEC_CONST (type1) != SPEC_CONST (type2))
                return 0;
              if (SPEC_VOLATILE (type1) != SPEC_VOLATILE (type2))
                return 0;
              if (SPEC_SHORT (type1) != SPEC_SHORT (type2))
                return 0;
              if (SPEC_LONG (type1) != SPEC_LONG (type2))
                return 0;
              if (SPEC_USIGN (type1) != SPEC_USIGN (type2))
                return 0;
            }
          else
            return 0;
        }
      else
        {
          if (IS_DECL (type2))
            {
              if (DCL_TYPE (type1) != DCL_TYPE (type2))
                return 0;
              if (DCL_PTR_CONST (type1) != DCL_PTR_CONST (type2))
                return 0;
              if (DCL_PTR_VOLATILE (type1) != DCL_PTR_VOLATILE (type2))
                return 0;
              if (DCL_TYPE (type1) == ARRAY
                  && DCL_ELEM (type1) != DCL_ELEM (type2))
                return 0;
              /* FIXME: need to match function pointer parameters */
            }
          else
            return 0;
        }
      
      type1 = type1->next;
      type2 = type2->next;
    }

  if (!type1 && !type2)
    return 1;
  else
    return 0;
}

/*-----------------------------------------------------------------------*/
/* dwTagFromType - returns the tag describing a type. If new tags need   */
/*                 to be created, they will be added under the specified */
/*                 parent tag                                            */
/*-----------------------------------------------------------------------*/
static dwtag *
dwTagFromType (sym_link * type, dwtag * parent)
{
  dwtag * oldtp;
  dwtag * tp = NULL;
  dwtag * modtp;
  dwtag * subtp;
  int key;
  int tableUpdated = 0;
  
  key = dwHashType (type) % dwTypeTagTable->size;
  oldtp = hTabFindByKey (dwTypeTagTable, key, type, dwMatchTypes);
  if (oldtp)
    return oldtp;
  else
    {
      if (IS_DECL (type))
        {
          switch (DCL_TYPE (type))
            {
            case POINTER:
            case FPOINTER:
            case CPOINTER:
            case GPOINTER:
            case PPOINTER:
            case IPOINTER:
            case EEPPOINTER:
            case UPOINTER:
              tp = dwNewTag (DW_TAG_pointer_type);
              if (type->next && !IS_VOID (type->next))
                {
                  subtp = dwTagFromType (type->next, parent);
                  dwAddTagAttr (tp, dwNewAttrTagRef (DW_AT_type, subtp));
                }
              dwAddTagAttr (tp, dwNewAttrConst (DW_AT_byte_size,
                                                getSize (type)));
              dwAddTagChild (parent, tp);
              if (DCL_PTR_VOLATILE (type))
                {
                  modtp = dwNewTag (DW_TAG_volatile_type);
                  dwAddTagAttr (modtp, dwNewAttrTagRef (DW_AT_type, tp));
                  dwAddTagChild (parent, modtp);
                  tp = modtp;
                }
              if (DCL_PTR_CONST (type))
                {
                  modtp = dwNewTag (DW_TAG_const_type);
                  dwAddTagAttr (modtp, dwNewAttrTagRef (DW_AT_type, tp));
                  dwAddTagChild (parent, modtp);
                  tp = modtp;
                }
              break;
              
            case ARRAY:
              tp = dwNewTag (DW_TAG_array_type);
              subtp = dwTagFromType (type->next, parent);
              dwAddTagAttr (tp, dwNewAttrTagRef (DW_AT_type, subtp));
              if (!subtp->parent)
                dwAddTagChild (tp, subtp);
              if (DCL_ELEM (type))
                {
                  dwAddTagAttr (tp, dwNewAttrConst (DW_AT_byte_size,
                                                    getSize (type)));
                  subtp = dwNewTag (DW_TAG_subrange_type);
                  dwAddTagAttr (subtp, dwNewAttrConst (DW_AT_upper_bound,
                                                       DCL_ELEM (type)-1));
                  dwAddTagChild (tp, subtp);
                }

              break;
            
            case FUNCTION:
              tp = dwNewTag (DW_TAG_subroutine_type);
              if (type->next && !IS_VOID (type->next))
                {
                  subtp = dwTagFromType (type->next, parent);
                  dwAddTagAttr (tp, dwNewAttrTagRef (DW_AT_type, subtp));
                }
              /* FIXME: need to handle function parameters */
              break;
              
            default:
              werror (E_INTERNAL_ERROR, __FILE__, __LINE__,
                      "unknown DCL_TYPE");
              exit (1);
            }
        }
      else
        {
          if (IS_STRUCT (type))
            {
              struct structdef * sdp = SPEC_STRUCT (type);
              symbol * field;
              
              tp = dwNewTag (sdp->type == STRUCT ? DW_TAG_structure_type
                                                 : DW_TAG_union_type);
              if (*(sdp->tag))
                dwAddTagAttr (tp, dwNewAttrString (DW_AT_name, sdp->tag));
              
              /* FIXME: should only specify the size if we know this */
              /* is a complete type */
              dwAddTagAttr (tp, dwNewAttrConst (DW_AT_byte_size,
                                                getSize (type)));
              
              /* Must add this before processing the struct fields */
              /* in case there is a recursive definition.          */
              hTabAddItemLong (&dwTypeTagTable, key, type, tp);
              tableUpdated = 1;

              field = sdp->fields;
              while (field)
                {
                  dwtag * memtp;
                  dwloc * lp;

                  if (IS_BITFIELD (field->type) && !SPEC_BLEN(field->type))
                    {
                      field = field->next;
                      continue;
                    }

                  memtp = dwNewTag (DW_TAG_member);
                  if (*(field->name))
                    dwAddTagAttr (memtp, dwNewAttrString (DW_AT_name,
                                                          field->name));
                  if (IS_BITFIELD (field->type))
                    {
                      unsigned blen = SPEC_BLEN (field->type);
                      unsigned bstr = SPEC_BSTR (field->type);
                      sym_link * type;
                      
                      dwAddTagAttr (memtp,
                                    dwNewAttrConst (DW_AT_byte_size,
                                                    (blen+7)/8));
                      dwAddTagAttr (memtp,
                                    dwNewAttrConst (DW_AT_bit_size, blen));
                      dwAddTagAttr (memtp,
                                    dwNewAttrConst (DW_AT_bit_offset,
                                                    ((blen+7) & ~7)
                                                    - (blen+bstr)));
                      if (blen < 8)
                        type = typeFromStr ("uc");
                      else
                        type = typeFromStr ("ui");
                      subtp = dwTagFromType (type, tp);
                      dwAddTagAttr (memtp, dwNewAttrTagRef (DW_AT_type, subtp));
                    }
                  else
                    {
                      subtp = dwTagFromType (field->type, tp);
                      dwAddTagAttr (memtp, dwNewAttrTagRef (DW_AT_type, subtp));
                      if (!subtp->parent)
                        dwAddTagChild (parent, subtp);
                    }

                  lp = dwNewLoc (DW_OP_plus_uconst, NULL, field->offset);
                  dwAddTagAttr (memtp,
                                dwNewAttrLoc (DW_AT_data_member_location, lp));

                  dwAddTagChild (tp, memtp);
                  
                  field = field->next;
                }
            }
          else if (SPEC_VOLATILE (type) || SPEC_CONST (type))
            {
              sym_link temptype = *type;
              
              SPEC_VOLATILE (&temptype) = 0;
              SPEC_CONST (&temptype) = 0;
              tp = dwTagFromType (&temptype, parent);
              if (SPEC_VOLATILE (type))
                {
                  modtp = dwNewTag (DW_TAG_volatile_type);
                  dwAddTagAttr (modtp, dwNewAttrTagRef (DW_AT_type, tp));
                  dwAddTagChild (parent, modtp);
                  tp = modtp;
                }
              if (SPEC_CONST (type))
                {
                  modtp = dwNewTag (DW_TAG_const_type);
                  dwAddTagAttr (modtp, dwNewAttrTagRef (DW_AT_type, tp));
                  dwAddTagChild (parent, modtp);
                  tp = modtp;
                }
            }
          else
            {
              switch (SPEC_NOUN (type))
                {
                case V_INT:
                  tp = dwNewTag (DW_TAG_base_type);
                  if (SPEC_USIGN (type))
                    {
                      dwAddTagAttr (tp, dwNewAttrConst (DW_AT_encoding,
                                                        DW_ATE_unsigned));
                      if (SPEC_LONG (type))
                        dwAddTagAttr (tp, dwNewAttrString (DW_AT_name,
                                                           "unsigned long"));
                      else
                        dwAddTagAttr (tp, dwNewAttrString (DW_AT_name,
                                                           "unsigned int"));
                    }
                  else
                    {
                      dwAddTagAttr (tp, dwNewAttrConst (DW_AT_encoding,
                                                        DW_ATE_signed));
                      if (SPEC_LONG (type))
                        dwAddTagAttr (tp, dwNewAttrString (DW_AT_name, "long"));
                      else
                        dwAddTagAttr (tp, dwNewAttrString (DW_AT_name, "int"));
                    }
                  dwAddTagAttr (tp, dwNewAttrConst (DW_AT_byte_size,
                                                    getSize (type)));
                  dwAddTagChild (dwRootTag, tp);
                  break;
                  
                case V_FLOAT:
                  tp = dwNewTag (DW_TAG_base_type);
                  dwAddTagAttr (tp, dwNewAttrConst (DW_AT_encoding,
                                                    DW_ATE_float));
                  dwAddTagAttr (tp, dwNewAttrString (DW_AT_name, "float"));
                  dwAddTagAttr (tp, dwNewAttrConst (DW_AT_byte_size,
                                                    getSize (type)));
                  dwAddTagChild (dwRootTag, tp);
                  break;

                case V_FIXED16X16:
                  tp = dwNewTag (DW_TAG_base_type);
                  dwAddTagAttr (tp, dwNewAttrConst (DW_AT_encoding,
                                                    DW_ATE_float));
                  dwAddTagAttr (tp, dwNewAttrString (DW_AT_name, "fixed16x16"));
                  dwAddTagAttr (tp, dwNewAttrConst (DW_AT_byte_size,
                                                    getSize (type)));
                  dwAddTagChild (dwRootTag, tp);
                  break;
                
                case V_BOOL:
                  tp = dwNewTag (DW_TAG_base_type);
                  dwAddTagAttr (tp, dwNewAttrConst (DW_AT_encoding,
                                                    DW_ATE_boolean));
                  dwAddTagAttr (tp, dwNewAttrString (DW_AT_name, "_Bool"));
                  dwAddTagAttr (tp, dwNewAttrConst (DW_AT_byte_size,
                                                    getSize (type)));
                  dwAddTagChild (dwRootTag, tp);
                  break;

                case V_CHAR:
                  tp = dwNewTag (DW_TAG_base_type);
                  if (SPEC_USIGN (type))
                    {
                      dwAddTagAttr (tp, dwNewAttrConst (DW_AT_encoding,
                                                        DW_ATE_unsigned_char));
                      dwAddTagAttr (tp, dwNewAttrString (DW_AT_name,
                                                         "unsigned char"));
                    }
                  else
                    {
                      dwAddTagAttr (tp, dwNewAttrConst (DW_AT_encoding,
                                                        DW_ATE_signed));
                      dwAddTagAttr (tp, dwNewAttrString (DW_AT_name, "signed char"));
                    }
                  dwAddTagAttr (tp, dwNewAttrConst (DW_AT_byte_size,
                                                    getSize (type)));
                  dwAddTagChild (dwRootTag, tp);
                  break;
                
                case V_VOID:
                case V_BIT:
                case V_BITFIELD:
                case V_SBIT:
                case V_DOUBLE:
                default:
                  
                  werror (E_INTERNAL_ERROR, __FILE__, __LINE__,
                      "unhandled base type");
                  printTypeChain (type, NULL);
                  exit (1);
  
                }
            }
        }
    }
  
  if (!tableUpdated)
    hTabAddItemLong (&dwTypeTagTable, key, type, tp);
  if (!tp->parent)
    dwAddTagChild (parent, tp);
  return tp;
}
/*------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* dwOpenFile - open the debugging file (just initialize, since all      */
/*              DWARF data goes into the assembly output file)           */
/*-----------------------------------------------------------------------*/
int
dwOpenFile(const char *file)
{
  dwTypeTagTable = newHashTable (128);
  
  return 1;
}

/*-----------------------------------------------------------------------*/
/* dwCloseFile - close the debugging file (do nothing, since all DWARF   */
/*               data goes into the assembly output file)                */
/*-----------------------------------------------------------------------*/
int
dwCloseFile (void)
{
  return 1;
}
  

/*-----------------------------------------------------------------------*/
/* dwGenerateScopes - recursively traverse an ast, generating lexical    */
/*                    block tags for block scopes found                  */
/*-----------------------------------------------------------------------*/
static void  
dwGenerateScopes (dwtag *tp, ast * tree)
{
  dwtag *subtp;

  if (!tree)
    return;
  
  if (!IS_AST_OP (tree))
    return;
      
  if (tree->opval.op == BLOCK)
    {
      subtp = dwNewTag (DW_TAG_lexical_block);
      if (tree->right)
        {
          dwAddTagAttr (subtp, dwNewAttrConst (DW_AT_user_block, tree->right->block));
          dwAddTagAttr (subtp, dwNewAttrConst (DW_AT_user_level, tree->right->level));
      
          dwAddTagChild (tp, subtp);
        }
      dwGenerateScopes (subtp, tree->right);
    }
  else
    {
      dwGenerateScopes (tp, tree->left);
      dwGenerateScopes (tp, tree->right);
    }
}

/*-----------------------------------------------------------------------*/
/* dwFindScope - return the lexical block tag for a particular block     */
/*               scope, or NULL if not found                             */
/*-----------------------------------------------------------------------*/
static dwtag *
dwFindScope (dwtag * tp, int block)
{
  dwtag * rettp;
  dwattr * ap;
  
  if (!tp)
    return NULL;
  
  while (tp)
    {
      if (tp->tag == DW_TAG_lexical_block)
        {
          ap = tp->attribs;
          while (ap)
            {
              if (ap->attr == DW_AT_user_block)
                {
                  if (ap->val.data == block)
                    return tp;
                }
              ap = ap->next;
            }
            
          rettp = dwFindScope (tp->firstChild, block);
          if (rettp)
            return rettp;
        }
      tp = tp->siblings;
    }
  
  return NULL;  
}

/*------------------------------------------------------------------------*/
/* dwWriteSymbolInternal - create tag information for a variable or       */
/*                         parameter and place it in the correct position */
/*                         within the tag tree                            */
/*------------------------------------------------------------------------*/
static int
dwWriteSymbolInternal (symbol *sym)
{
  dwtag * tp;
  dwtag * subtp;
  dwloc * lp;
  dwtag * scopetp;
  symbol * symloc;
  dwtag * functp;
  dwattr * funcap;
  int inregs = 0;

  if (!sym->level || IS_EXTERN (sym->etype))
    scopetp = dwRootTag;
  else
    {
      assert(sym->localof);
      if (!sym->localof)
        return 0;
        
      /* Find the tag for the function this symbol is defined in */
      functp = dwRootTag->firstChild;
      while (functp)
        {
          if (functp->tag == DW_TAG_subprogram)
            {
              funcap = dwFindAttr (functp, DW_AT_name);
              if (funcap && !strcmp (funcap->val.string, sym->localof->name))
                break;
            }
          functp = functp->siblings;
        }
      assert (functp);
      if (!functp)
        return 0;
        
      /* Find the correct scope within this function */
      scopetp = dwFindScope (functp->firstChild, sym->block);
      if (!scopetp)
        scopetp = functp;
    }
  
  tp = dwNewTag (sym->_isparm ? DW_TAG_formal_parameter : DW_TAG_variable);
  
  dwAddTagAttr (tp, dwNewAttrString (DW_AT_name, sym->name));
  
  /* Find the ultimate symbol holding the value. */
  /* Might be:                                   */
  /*   a) original symbol,                       */
  /*   b) register equivalent,                   */
  /*   c) spill location                         */
  symloc = sym;
  if (!sym->allocreq && sym->reqv)
    {
      symloc = OP_SYMBOL (symloc->reqv);
      if (symloc->isspilt && !symloc->remat)
        symloc = symloc->usl.spillLoc;
      else
        inregs = 1;
    }
  
  lp = NULL;
  if (inregs && symloc->regs[0])
    {
      dwloc * reglp;
      dwloc * lastlp = NULL;
      int regNum;
      int i;
      
      /* register allocation */
      for (i = (port->little_endian ? 0 : symloc->nRegs-1);
           (port->little_endian ? (i < symloc->nRegs) : (i >= 0));
           (port->little_endian ? i++ : i--))
        {
          regNum = port->debugger.dwarf.regNum (symloc->regs[i]);
          if (regNum >= 0 && regNum <= 31)
            reglp = dwNewLoc (DW_OP_reg0 + regNum, NULL, 0);
          else if (regNum >= 0)
            reglp = dwNewLoc (DW_OP_regx, NULL, regNum);
          else
            {
              /* We are forced to give up if the ABI for this port */
              /* does not define a number for this register        */
              lp = NULL;
              break;
            }
          
          if (lastlp)
            lastlp->next = reglp;
          else
            lp = reglp;
          lastlp = reglp;
          
          if (symloc->nRegs != 1)
            {
              reglp = dwNewLoc (DW_OP_piece, NULL, 1);
              lastlp->next = reglp;
              lastlp = reglp;
            }
        }
    }
  else if (symloc->onStack)
    {
      /* stack allocation */
      lp = dwNewLoc (DW_OP_fbreg, NULL, symloc->stack);
    }
  else
    {
      /* global allocation */
      if (sym->level && !sym->allocreq)
        lp = NULL;
      else
        lp = dwNewLoc (DW_OP_addr, symloc->rname, 0);
    }

  /* Only create the DW_AT_location if a known location exists.   */
  /* It might not exist if the variable has been optimized away   */
  /* or if the compiler has lost track of it (not good, but still */
  /* happens sometimes -- need to improve induction)              */
  if (lp)
    dwAddTagAttr (tp, dwNewAttrLoc (DW_AT_location, lp));
  
  if (!IS_STATIC (sym->etype) && !sym->level)
    dwAddTagAttr (tp, dwNewAttrFlag (DW_AT_external, 1));
  if (IS_EXTERN (sym->etype))
    dwAddTagAttr (tp, dwNewAttrFlag (DW_AT_declaration, 1));
  
  subtp = dwTagFromType (sym->type, scopetp);
  dwAddTagAttr (tp, dwNewAttrTagRef (DW_AT_type, subtp));
  if (!subtp->parent)
    dwAddTagChild (scopetp, subtp);
  
  dwAddTagChild (scopetp, tp);
  return 1;
}
 

/*-----------------------------------------------------------------------*/
/* dwWriteFunction - generate a tag for a function.                      */
/*-----------------------------------------------------------------------*/
int
dwWriteFunction (symbol *sym, iCode *ic)
{
  dwtag * tp;
  value * args;
    
  dwFuncTag = tp = dwNewTag (DW_TAG_subprogram);
  
  dwAddTagAttr (dwFuncTag, dwNewAttrString (DW_AT_name, sym->name));
  
  dwAddTagAttr (dwFuncTag, dwNewAttrAddrSymbol (DW_AT_low_pc, sym, 0));
  
  if (FUNC_ISISR (sym->type))
    dwAddTagAttr (dwFuncTag, dwNewAttrConst (DW_AT_calling_convention,
                                              DW_CC_nocall));
  
  dwAddTagAttr (dwFuncTag, dwNewAttrFlag (DW_AT_external, 
                                           !IS_STATIC (sym->etype)));

  if (sym->type->next && !IS_VOID (sym->type->next))
    {
      dwtag * subtp;

      subtp = dwTagFromType (sym->type->next, dwRootTag);
      dwAddTagAttr (dwFuncTag, dwNewAttrTagRef (DW_AT_type, subtp));
    }
  dwAddTagChild (dwRootTag, dwFuncTag);
  
  args = FUNC_ARGS(sym->type);
  while (args)
    {
      dwWriteSymbolInternal (args->sym);
      args = args->next;
    }
  if (FUNC_HASVARARGS (sym->type))
    {
      dwAddTagChild (dwFuncTag, dwNewTag (DW_TAG_unspecified_parameters));
    }
  
  while (ic && ic->op != FUNCTION)
    ic = ic->next;
  if (ic && ic->op == FUNCTION && ic->tree && ic->tree->right)
    {
      dwGenerateScopes (dwFuncTag, ic->tree->right->left);
      dwGenerateScopes (dwFuncTag, ic->tree->right->right);
    }
  
  dwScopeTag = NULL;
  dwScopeLevel = 0;
  
  return 1;
}


/*-----------------------------------------------------------------------*/
/* dwWriteEndFunction - write attributes to the current function tag     */
/*                      that are only known after code generation is     */
/*                      complete                                         */
/*-----------------------------------------------------------------------*/
int
dwWriteEndFunction (symbol *sym, iCode *ic, int offset)
{
  char debugSym[SDCC_NAME_MAX + 1];
  
  if (ic)
    {
      dwWriteCLine (ic);
      dwLineLast->offset += offset;
      dwLineLast->end_sequence = 1;
    }

  if (IS_STATIC (sym->etype))
    sprintf (debugSym, "XF%s$%s$0$0", dwModuleName, sym->name);
  else
    sprintf (debugSym, "XG$%s$0$0", sym->name);
  emitDebuggerSymbol (debugSym);

  dwAddTagAttr (dwFuncTag, dwNewAttrAddrLabel (DW_AT_high_pc,
                                               Safe_strdup(debugSym),
                                               offset));

  if (dwFrameLocList)
    {
      dwAddTagAttr (dwFuncTag, dwNewAttrLocRef (DW_AT_frame_base,
                                                dwFrameLocList));

      dwFrameLocList->next = dwRootLocList;
      dwRootLocList = dwFrameLocList;
      dwFrameLocList = NULL;
    }

  return 1;
}


/*-----------------------------------------------------------------------*/
/* dwWriteLabel - generate a tag for a source level label                */
/*-----------------------------------------------------------------------*/
int
dwWriteLabel (symbol *sym, iCode *ic)
{
  char debugSym[SDCC_NAME_MAX + 1];
  dwtag * tp;
  
  /* ignore the compiler generated labels */
  if (sym->isitmp)
    return 1;

  sprintf (debugSym, "L%s$%s$%s", dwModuleName, currFunc->name, sym->name);
  emitDebuggerSymbol (debugSym);

  tp = dwNewTag (DW_TAG_label);
  dwAddTagAttr (tp, dwNewAttrString (DW_AT_name, sym->name));
  dwAddTagAttr (tp, dwNewAttrAddrLabel (DW_AT_low_pc,
                                        Safe_strdup (debugSym), 0));

  dwAddTagChild (dwFuncTag, tp);
  
  return 1;
}


/*-----------------------------------------------------------------------*/
/* dwWriteScope - add the starting and ending address attributes to a    */
/*                a lexical block tag (created during dwWriteFunction)   */
/*-----------------------------------------------------------------------*/
int
dwWriteScope (iCode *ic)
{
  char * debugSym = NULL;
  dwtag * scopetp;
  dwattr * ap;
  
  scopetp = dwFindScope (dwFuncTag->firstChild, ic->block);
  
  if (dwScopeTag && ic->level <= dwScopeLevel)
    {
      debugSym = dwNewDebugSymbol ();
      emitDebuggerSymbol (debugSym);
      dwSetTagAttr (dwScopeTag, dwNewAttrAddrLabel (DW_AT_high_pc, debugSym, 0));
                                                    
      dwScopeTag = scopetp;
      dwScopeLevel = ic->level;
    }
  if (scopetp)
    {
      ap = dwFindAttr (scopetp, DW_AT_low_pc);
      if (ap)
        return 1;
      
      if (!debugSym)
        debugSym = dwNewDebugSymbol ();
      emitDebuggerSymbol (debugSym);
      dwAddTagAttr (scopetp, dwNewAttrAddrLabel (DW_AT_low_pc, debugSym, 0));
                                                 
      dwScopeTag = scopetp;
      dwScopeLevel = ic->level;
    }
  
  return 1;
}

/*-----------------------------------------------------------------------*/
/* dwWriteSymbol - generate tags for global variables. This is actually  */
/*                 called for all variables and parameters, but we       */
/*                 process the non-global variables elsewhere.           */
/*-----------------------------------------------------------------------*/
int
dwWriteSymbol (symbol *sym)
{
  if (IS_FUNC (sym->type))
    return 1;

  /* If it is an iTemp, then it is a local variable; ignore it */
  if (sym->isitmp)
    return 1;

  /* If it is an unused extern ignore it, or it might produce link failure */
  if (IS_EXTERN (sym->etype) && !sym->used)
    return 1;

  /* Ignore parameters; they must be handled specially so that they will */
  /* appear in the correct order */
  if (sym->_isparm)
    return 1;
    
  return dwWriteSymbolInternal (sym);
}


/*-----------------------------------------------------------------------*/
/* dwWriteType                                                           */
/*-----------------------------------------------------------------------*/
int
dwWriteType (structdef *sdef, int block, int inStruct, const char *tag)
{
  /* FIXME: needs implementation */
  return 1;
}


/*-----------------------------------------------------------------------*/
/* dwWriteModule - generates the root tag for this compilation unit      */
/*-----------------------------------------------------------------------*/
int
dwWriteModule (const char *name)
{
  dwtag * tp;
  char *verid = (char*)Safe_alloc(125);
  
  dwModuleName = Safe_strdup (name);
  
  sprintf(verid, "SDCC version %s #%s", SDCC_VERSION_STR, getBuildNumber());
    
  tp = dwNewTag (DW_TAG_compile_unit);
  dwAddTagAttr (tp, dwNewAttrString (DW_AT_producer, verid));

  dwAddTagAttr (tp, dwNewAttrConst (DW_AT_language, DW_LANG_C89));

  dwAddTagAttr (tp, dwNewAttrString (DW_AT_name, fullSrcFileName));

  dwAddTagAttr (tp, dwNewAttrLabelRef (DW_AT_stmt_list,
                                       "Ldebug_line_start", -4));

  dwRootTag = tp;
  
  return 1;
}


/*-----------------------------------------------------------------------*/
/* dwWriteCLine - generates a line number/position to address record for */
/*                C source                                               */
/*-----------------------------------------------------------------------*/
int
dwWriteCLine (iCode *ic)
{
  dwline * lp;
  char * debugSym;
  
  lp = Safe_alloc (sizeof (dwline));

  lp->line = ic->lineno;
  
  debugSym = dwNewDebugSymbol ();
  emitDebuggerSymbol (debugSym);
  lp->label = debugSym;
  lp->offset = 0;

  lp->fileIndex = dwFindFileIndex (ic->filename);

  if (!dwLineFirst)
    dwLineFirst = lp;
  else
    dwLineLast->next = lp;
  dwLineLast = lp;
  
  return 1;
}


/*-----------------------------------------------------------------------*/
/* dwWriteFrameAddress - note the current position of the frame pointer  */
/*                       address. The base address can be specified by   */
/*                       either a register or pointer variable, leaving  */
/*                       the other as NULL. If both are NULL, there is   */
/*                       no current frame pointer address defined.       */
/*-----------------------------------------------------------------------*/
int
dwWriteFrameAddress(const char *variable, struct reg_info *reg, int offset)
{
  char * debugSym = NULL;
  dwlocregion * lrp;
  dwloc * lp;
  int regNum;
    
  /* If there was a region open, close it */
  if (dwFrameLastLoc)
    {
      debugSym = dwNewDebugSymbol ();
      emitDebuggerSymbol (debugSym);
      
      dwFrameLastLoc->endLabel = debugSym;
      dwFrameLastLoc = NULL;
    }

  if (!variable && !reg)
    return 1;

  /* Create a new debugger symbol for the start of the region if */
  /* we can't recycle the symbol at the end of the previous      */
  if (!debugSym)
    {
      debugSym = dwNewDebugSymbol ();
      emitDebuggerSymbol (debugSym);
    }

  lrp = Safe_alloc (sizeof (dwlocregion));
  lrp->startLabel = debugSym;

  if (variable)         /* frame pointer based from a global variable */
    {
      dwloc * lp;

      lrp->loc = dwNewLoc (DW_OP_addr, variable, 0);
      lrp->loc->next = lp = dwNewLoc (DW_OP_deref_size, NULL, PTRSIZE);
      if (offset)
        {
          lp->next = dwNewLoc (DW_OP_consts, NULL, offset);
          lp->next->next = dwNewLoc (DW_OP_plus, NULL, 0);
        }
    }
  else if (reg)         /* frame pointer based from a register */
    {
      regNum = port->debugger.dwarf.regNum (reg);
      assert (regNum>=0);
      
      if (regNum>=0 && regNum<=31)
        {
          if (offset)
            lrp->loc = dwNewLoc (DW_OP_breg0 + regNum, NULL, offset);
          else
            lrp->loc = dwNewLoc (DW_OP_reg0 + regNum, NULL, 0);
        }
      else
        {
          lrp->loc = lp = dwNewLoc (DW_OP_regx, NULL, regNum);
          if (offset)
            {
              lp->next = dwNewLoc (DW_OP_consts, NULL, offset);
              lp->next->next = dwNewLoc (DW_OP_plus, NULL, 0);
            }
        }
    }
  dwFrameLastLoc = lrp;
  
  if (!dwFrameLocList)
    dwFrameLocList = dwNewLocList();
  lrp->next = dwFrameLocList->region;
  dwFrameLocList->region = lrp;
  
  return 1;
}


/*-----------------------------------------------------------------------*/
/* dwWriteALine - generates a line number/position to address record for */
/*                assembly source                                        */
/*-----------------------------------------------------------------------*/
int
dwWriteALine(const char *module, int Line)
{
  return 1;
}


/*-----------------------------------------------------------------------*/
/* dwarf2FinalizeFile - write all of the DWARF debugging data to the     */
/*                      debug file                                       */
/*-----------------------------------------------------------------------*/
int
dwarf2FinalizeFile (FILE *of)
{
  int tagAddress = 11;
  int abbrevNum = 0;
  int attr;

  dwarf2FilePtr = of;
  
  /* Write the .debug_line section */
  dwWriteLineNumbers ();
 
  /* Assign the location list addresses (for cross references) */
  dwAssignLocListAddresses ();
  
  /* Write the .debug_loc section */
  dwWriteLocLists ();

  /* Delete our scope related user attributes; they were only needed to help */
  /* build the tag tree and have no meaning to (and may confuse) debuggers   */
  attr = DW_AT_user_block;
  dwTraverseTag (dwRootTag, dwDeleteTagAttr, &attr);
  attr = DW_AT_user_level;
  dwTraverseTag (dwRootTag, dwDeleteTagAttr, &attr);
  
  /* Add a DW_AT_sibling attribute to all tags with children and siblings */
  dwTraverseTag (dwRootTag, dwAddSibAttr, NULL);

  /* Assign the tag abbreviations. The tags, attributes, and forms must   */
  /* not change after this point. The attribute values may change as long */
  /* as the size of the value does not.                                   */
  dwAbbrevTable = newHashTable (128);
  dwTraverseTag (dwRootTag, dwAssignAbbrev, &abbrevNum);
  
  /* Assign the tag addresses (for cross references) */
  dwTraverseTag (dwRootTag, dwAssignTagAddress, &tagAddress);
  
  /* Write the .debug_abbrev section */
  dwWriteAbbrevs ();  
  
  /* Write the .debug_info section */
  dwWriteTags ();

  /* Write the .debug_pubnames section */
  dwWritePubnames ();
  
  return 1;
}
