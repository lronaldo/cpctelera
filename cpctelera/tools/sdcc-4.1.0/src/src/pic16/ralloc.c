/*------------------------------------------------------------------------

  ralloc.c - source file for register allocation. PIC16 specific

                Written By -  Sandeep Dutta . sandeep.dutta@usa.net (1998)
                Added Pic Port T.scott Dattalo scott@dattalo.com (2000)
                Added Pic16 Port Martin Dubuc m.dubuc@rogers.com (2002)

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
#include "ralloc.h"
#include "pcode.h"
#include "gen.h"
#include "device.h"

#ifndef debugf
#define debugf(frm, rest)       _debugf(__FILE__, __LINE__, frm, rest)
#endif
void _debugf(char *f, int l, char *frm, ...);

#define NEWREG_DEBUG    0
//#define USE_ONSTACK


/*-----------------------------------------------------------------*/
/* At this point we start getting processor specific although      */
/* some routines are non-processor specific & can be reused when   */
/* targetting other processors. The decision for this will have    */
/* to be made on a routine by routine basis                        */
/* routines used to pack registers are most definitely not reusable */
/* since the pack the registers depending strictly on the MCU      */
/*-----------------------------------------------------------------*/

reg_info *pic16_typeRegWithIdx (int idx, int type, int fixed);
extern void genpic16Code (iCode *);

/* Global data */
static struct
  {
    bitVect *spiltSet;
    set *stackSpil;
    bitVect *regAssigned;
    short blockSpil;
    int slocNum;
    bitVect *funcrUsed;         /* registers used in a function */
    int stackExtend;
    int dataExtend;
  }
_G;

/* Shared with gen.c */
int pic16_ptrRegReq;            /* one byte pointer register required */


set *pic16_dynAllocRegs=NULL;
set *pic16_dynStackRegs=NULL;
set *pic16_dynProcessorRegs=NULL;
set *pic16_dynDirectRegs=NULL;
set *pic16_dynDirectBitRegs=NULL;
set *pic16_dynInternalRegs=NULL;
set *pic16_dynAccessRegs=NULL;

static hTab *dynDirectRegNames=NULL;
static hTab *dynAllocRegNames=NULL;
static hTab *dynProcRegNames=NULL;
static hTab *dynAccessRegNames=NULL;
//static hTab  *regHash = NULL;    /* a hash table containing ALL registers */

extern set *sectNames;

set *pic16_rel_udata=NULL;      /* relocatable uninitialized registers */
set *pic16_fix_udata=NULL;      /* absolute uninitialized registers */
set *pic16_equ_data=NULL;       /* registers used by equates */
set *pic16_int_regs=NULL;       /* internal registers placed in access bank 0 to 0x7f */
set *pic16_acs_udata=NULL;      /* access bank variables */

set *pic16_builtin_functions=NULL;

static int dynrIdx=0x00;                //0x20;         // starting temporary register rIdx
static int rDirectIdx=0;

int pic16_nRegs = 128;   // = sizeof (regspic16) / sizeof (regs);

int pic16_Gstack_base_addr=0; /* The starting address of registers that
                         * are used to pass and return parameters */


int _inRegAllocator=0;  /* flag that marks whther allocReg happens while
                         * inside the register allocator function */


static void spillThis (symbol *);
int pic16_ralloc_debug = 0;
static FILE *debugF = NULL;
/*-----------------------------------------------------------------*/
/* debugLog - open a file for debugging information                */
/*-----------------------------------------------------------------*/
//static void debugLog(char *inst,char *fmt, ...)
static void
debugLog (const char *fmt,...)
{
  static int append = 0;        // First time through, open the file without append.

  char buffer[256];
  //char *bufferP=buffer;
  va_list ap;

  if (!pic16_ralloc_debug || !dstFileName)
    return;


  if (!debugF)
    {
      /* create the file name */
      SNPRINTF(buffer, sizeof(buffer), "%s.d", dstFileName);

      if (!(debugF = fopen (buffer, (append ? "a+" : "w"))))
        {
          werror (E_OUTPUT_FILE_OPEN_ERR, buffer, strerror (errno));
          exit (1);
        }
      append = 1;               // Next time debubLog is called, we'll append the debug info
    }

  va_start (ap, fmt);

  vsprintf (buffer, fmt, ap);

  fprintf (debugF, "%s", buffer);
  //fprintf (stderr, "%s", buffer);
/*
   while (isspace((unsigned char)*bufferP)) bufferP++;

   if (bufferP && *bufferP)
   lineCurr = (lineCurr ?
   connectLine(lineCurr,newLineNode(lb)) :
   (lineHead = newLineNode(lb)));
   lineCurr->isInline = _G.inLine;
   lineCurr->isDebug  = _G.debugLine;
 */
  va_end (ap);

}

static void
debugNewLine (void)
{
  if(!pic16_ralloc_debug)return;

  if (debugF)
    fputc ('\n', debugF);
}
/*-----------------------------------------------------------------*/
/* debugLogClose - closes the debug log file (if opened)           */
/*-----------------------------------------------------------------*/
static void
debugLogClose (void)
{
  if (debugF) {
    fclose (debugF);
    debugF = NULL;
  }
}

#define AOP(op) op->aop

static char *
debugAopGet (char *str, operand * op)
{
        if(!pic16_ralloc_debug)return NULL;

        if (str)
                debugLog (str);

        printOperand (op, debugF);
        debugNewLine ();

  return NULL;
}

char *
pic16_decodeOp (unsigned int op)
{
        if (op < 128 && op > ' ') {
                buffer[0] = op & 0xff;
                buffer[1] = '\0';
          return buffer;
        }

        switch (op) {
                case IDENTIFIER:        return "IDENTIFIER";
                case TYPE_NAME:         return "TYPE_NAME";
                case CONSTANT:          return "CONSTANT";
                case STRING_LITERAL:    return "STRING_LITERAL";
                case SIZEOF:            return "SIZEOF";
                case PTR_OP:            return "PTR_OP";
                case INC_OP:            return "INC_OP";
                case DEC_OP:            return "DEC_OP";
                case LEFT_OP:           return "LEFT_OP";
                case RIGHT_OP:          return "RIGHT_OP";
                case LE_OP:             return "LE_OP";
                case GE_OP:             return "GE_OP";
                case EQ_OP:             return "EQ_OP";
                case NE_OP:             return "NE_OP";
                case AND_OP:            return "AND_OP";
                case OR_OP:             return "OR_OP";
                case MUL_ASSIGN:        return "MUL_ASSIGN";
                case DIV_ASSIGN:        return "DIV_ASSIGN";
                case MOD_ASSIGN:        return "MOD_ASSIGN";
                case ADD_ASSIGN:        return "ADD_ASSIGN";
                case SUB_ASSIGN:        return "SUB_ASSIGN";
                case LEFT_ASSIGN:       return "LEFT_ASSIGN";
                case RIGHT_ASSIGN:      return "RIGHT_ASSIGN";
                case AND_ASSIGN:        return "AND_ASSIGN";
                case XOR_ASSIGN:        return "XOR_ASSIGN";
                case OR_ASSIGN:         return "OR_ASSIGN";
                case TYPEDEF:           return "TYPEDEF";
                case EXTERN:            return "EXTERN";
                case STATIC:            return "STATIC";
                case AUTO:              return "AUTO";
                case REGISTER:          return "REGISTER";
                case CODE:              return "CODE";
                case EEPROM:            return "EEPROM";
                case INTERRUPT:         return "INTERRUPT";
                case SFR:               return "SFR";
                case AT:                return "AT";
                case SBIT:              return "SBIT";
                case REENTRANT:         return "REENTRANT";
                case USING:             return "USING";
                case XDATA:             return "XDATA";
                case DATA:              return "DATA";
                case IDATA:             return "IDATA";
                case PDATA:             return "PDATA";
                case VAR_ARGS:          return "VAR_ARGS";
                case CRITICAL:          return "CRITICAL";
                case NONBANKED:         return "NONBANKED";
                case BANKED:            return "BANKED";
                case SD_CHAR:           return "CHAR";
                case SD_SHORT:          return "SHORT";
                case SD_INT:            return "INT";
                case SD_LONG:           return "LONG";
                case SIGNED:            return "SIGNED";
                case UNSIGNED:          return "UNSIGNED";
                case SD_FLOAT:          return "FLOAT";
                case DOUBLE:            return "DOUBLE";
                case SD_CONST:          return "CONST";
                case VOLATILE:          return "VOLATILE";
                case SD_VOID:           return "VOID";
                case BIT:               return "BIT";
                case STRUCT:            return "STRUCT";
                case UNION:             return "UNION";
                case ENUM:              return "ENUM";
                case RANGE:             return "RANGE";
                case SD_FAR:            return "FAR";
                case CASE:              return "CASE";
                case DEFAULT:           return "DEFAULT";
                case IF:                return "IF";
                case ELSE:              return "ELSE";
                case SWITCH:            return "SWITCH";
                case WHILE:             return "WHILE";
                case DO:                return "DO";
                case FOR:               return "FOR";
                case GOTO:              return "GOTO";
                case CONTINUE:          return "CONTINUE";
                case BREAK:             return "BREAK";
                case RETURN:            return "RETURN";
                case INLINEASM:         return "INLINEASM";
                case IFX:               return "IFX";
                case ADDRESS_OF:        return "ADDRESS_OF";
                case GET_VALUE_AT_ADDRESS:      return "GET_VALUE_AT_ADDRESS";
                case SPIL:              return "SPIL";
                case UNSPIL:            return "UNSPIL";
                case GETHBIT:           return "GETHBIT";
                case BITWISEAND:        return "BITWISEAND";
                case UNARYMINUS:        return "UNARYMINUS";
                case IPUSH:             return "IPUSH";
                case IPOP:              return "IPOP";
                case PCALL:             return "PCALL";
                case FUNCTION:          return "FUNCTION";
                case ENDFUNCTION:       return "ENDFUNCTION";
                case JUMPTABLE:         return "JUMPTABLE";
                case RRC:               return "RRC";
                case RLC:               return "RLC";
                case CAST:              return "CAST";
                case CALL:              return "CALL";
                case PARAM:             return "PARAM  ";
                case NULLOP:            return "NULLOP";
                case BLOCK:             return "BLOCK";
                case LABEL:             return "LABEL";
                case RECEIVE:           return "RECEIVE";
                case SEND:              return "SEND";
                case DUMMY_READ_VOLATILE:       return "DUMMY_READ_VOLATILE";
        }
        SNPRINTF(buffer, sizeof(buffer), "unknown op %d %c", op, op & 0xff);

  return buffer;
}

#if 0
static const char *decodeRegType(short type)
{
        switch(type) {
                case REG_GPR: return "REG_GPR";
                case REG_PTR: return "REG_PTR";
                case REG_CND: return "REG_CNT";

        default:
                return "<unknown>";
        }
}
#endif

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static const char *
debugLogRegType (short type)
{
        if(!pic16_ralloc_debug)return NULL;
        switch (type) {
                case REG_GPR: return "REG_GPR";
                case REG_PTR: return "REG_PTR";
                case REG_CND: return "REG_CND";
        }
        SNPRINTF(buffer, sizeof(buffer), "unknown reg type %d", type);

  return buffer;
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
static int regname2key(char const *name)
{
  int key = 0;

  if(!name)
    return 0;

  while(*name) {
    key += (*name++) + 1;
  }

  return ((key + (key >> 4) + (key>>8)) & 0x3f);
}

/*-----------------------------------------------------------------*/
/* newReg - allocate and init memory for a new register            */
/*-----------------------------------------------------------------*/
reg_info* newReg(int type, short pc_type, int rIdx, const char *name, unsigned size, int alias, operand *refop)
{

  reg_info *dReg;

        dReg = Safe_alloc(sizeof(reg_info));
        dReg->type = type;
        dReg->pc_type = pc_type;
        dReg->rIdx = rIdx;
        if(name)
                dReg->name = Safe_strdup(name);
        else {
          if(pic16_options.xinst && pc_type == PO_GPR_TEMP) {
            SNPRINTF(buffer, sizeof(buffer), "0x%02x", dReg->rIdx);
          } else {
            SNPRINTF(buffer, sizeof(buffer), "r0x%02x", dReg->rIdx);
          }

          if(type == REG_STK) {
            *buffer = 's';
          }
          dReg->name = Safe_strdup(buffer);
        }


        dReg->isFree = 0;
        dReg->wasUsed = 1;
        dReg->isEmitted = 0;

        if(type == REG_SFR) {
                dReg->isFixed = 1;
                dReg->address = rIdx;
                dReg->accessBank = 1;
        } else {
                dReg->isFixed = 0;
                dReg->address = 0;
                dReg->accessBank = 0;
        }

#if NEWREG_DEBUG
        fprintf(stderr,"newReg @ %p: %s, rIdx = 0x%02x\taccess= %d\tregop= %p\n",dReg, dReg->name,rIdx, dReg->accessBank, refop);
#endif
        dReg->size = size;
        dReg->alias = alias;
        dReg->reg_alias = NULL;
        dReg->reglives.usedpFlows = newSet();
        dReg->reglives.assignedpFlows = newSet();
        dReg->regop = refop;

        if(!(type == REG_SFR && alias == 0x80))
                hTabAddItem(&dynDirectRegNames, regname2key(dReg->name), dReg);

  return dReg;
}

/*-----------------------------------------------------------------*/
/* regWithIdx - Search through a set of registers that matches idx */
/*-----------------------------------------------------------------*/
static reg_info *
regWithIdx (set *dRegs, int idx, unsigned fixed)
{
  reg_info *dReg;

//#define D(text)       text
#define D(text)

  for (dReg = setFirstItem(dRegs) ; dReg ;
       dReg = setNextItem(dRegs)) {

        D(fprintf(stderr, "%s:%d testing reg w/rIdx = %d (%d f:%d)\t", __FUNCTION__, __LINE__, dReg->rIdx, idx, fixed));
    if(idx == dReg->rIdx && (fixed == dReg->isFixed)) {
          D(fprintf(stderr, "found!\n"));
      return dReg;
    } else
          D(fprintf(stderr, "not found!\n"));
  }

  return NULL;
}

/*-----------------------------------------------------------------*/
/* regFindFree - Search for a free register in a set of registers  */
/*-----------------------------------------------------------------*/
static reg_info *
regFindFree (set *dRegs)
{
  reg_info *dReg;

  for (dReg = setFirstItem(dRegs) ; dReg ;
       dReg = setNextItem(dRegs)) {

//      fprintf(stderr, "%s:%d checking register %s (%p) [rIdx: 0x%02x] if free= %d\n",
//              __FILE__, __LINE__, dReg->name, dReg, dReg->rIdx, dReg->isFree);

    if(dReg->isFree) {
//              fprintf(stderr, "%s:%d free register found, rIdx = %d\n", __FILE__, __LINE__, dReg->rIdx);

      return dReg;
    }
  }

  return NULL;
}

static reg_info *
regFindFreeNext(set *dRegs, reg_info *creg)
{
  reg_info *dReg;

    if(creg) {
      /* position at current register */
      for(dReg = setFirstItem(dRegs); dReg != creg; dReg = setNextItem(dRegs));
    }

    for(dReg = setNextItem(dRegs); dReg; dReg = setNextItem(dRegs)) {
      if(dReg->isFree) {
        return dReg;
      }
    }

  return NULL;
}

/*-----------------------------------------------------------------*
 *-----------------------------------------------------------------*/
reg_info *
pic16_allocProcessorRegister(int rIdx, const char * name, short po_type, int alias)
{
  reg_info *reg = newReg(REG_SFR, po_type, rIdx, name, 1, alias, NULL);

//      fprintf(stderr,"%s: %s addr =0x%x\n",__FUNCTION__, name,rIdx);

        reg->wasUsed = 0;               // we do not know if they are going to be used at all
        reg->accessBank = 1;            // implicit add access Bank

        hTabAddItem(&dynProcRegNames, regname2key(reg->name), reg);

  return addSet(&pic16_dynProcessorRegs, reg);
}

/*-----------------------------------------------------------------*
 *-----------------------------------------------------------------*/

reg_info *
pic16_allocInternalRegister(int rIdx, const char *name, short po_type, int alias)
{
  reg_info * reg = newReg(REG_GPR, po_type, rIdx, name, 1, alias, NULL);

//  fprintf(stderr,"%s:%d: %s   %s addr =0x%x\n",__FILE__, __LINE__, __FUNCTION__, name, rIdx);

    if(reg) {
      reg->wasUsed = 0;
      return addSet(&pic16_dynInternalRegs, reg);
    }

  return NULL;
}


/*-----------------------------------------------------------------*/
/* allocReg - allocates register of given type                     */
/*-----------------------------------------------------------------*/
static reg_info *
allocReg (short type)
{
  reg_info * reg=NULL;

#define MAX_P16_NREGS   16


#if 0
  if(dynrIdx > pic16_nRegs)
        werror(W_POSSBUG2, __FILE__, __LINE__);
        return NULL;
#endif

        /* try to reuse some unused registers */
        reg = regFindFree( pic16_dynAllocRegs );

        if(reg) {
//              fprintf(stderr, "%s: [%s][cf:%p] found FREE register %s, rIdx: %d\n", __FILE__, (_inRegAllocator)?"ralloc":"", currFunc, reg->name, reg->rIdx);
        }

        if(!reg) {
                reg = newReg(REG_GPR, PO_GPR_TEMP, dynrIdx++, NULL, 1, 0, NULL);
//              fprintf(stderr, "%s [%s][cf:%p] allocating NEW register %s, rIdx: %d\n", __FILE__,
//                                      (_inRegAllocator)?"ralloc":"", currFunc, reg->name, reg->rIdx);

#if 1
                if(_inRegAllocator && (dynrIdx > MAX_P16_NREGS)) {
                  //                  debugf("allocating more registers than available\n", 0);
                  //                  return (NULL);
                }

                addSet(&pic16_dynAllocRegs, reg);
                hTabAddItem(&dynAllocRegNames, regname2key(reg->name), reg);
//              fprintf(stderr, "%s:%d added reg to pic16_dynAllocRegs = %p\n", __FUNCTION__, __LINE__, pic16_dynAllocRegs);
#endif
        }

        debugLog ("%s of type %s for register rIdx: %d (0x%x)\n", __FUNCTION__, debugLogRegType (type), dynrIdx-1, dynrIdx-1);

#if 0
        fprintf(stderr,"%s:%d: %s\t%s addr= 0x%x\trIdx= 0x%02x isFree: %d\n",
                __FILE__, __LINE__, __FUNCTION__, reg->name, reg->address, reg->rIdx, reg->isFree);
#endif
        if(reg) {
                reg->isFree=0;
                reg->accessBank = 1;    /* this is a temporary register alloc in accessBank */
                reg->isLocal = 1;       /* this is a local frame register */
//              reg->wasUsed = 1;
        }

        if (currFunc) {
//              fprintf(stderr, "%s:%d adding %s into function %s regsUsed\n", __FUNCTION__, __LINE__, reg->name, currFunc->name);
                currFunc->regsUsed = bitVectSetBit (currFunc->regsUsed, reg->rIdx);
        }

  return (reg);         // addSet(&pic16_dynAllocRegs,reg);

}


/*-----------------------------------------------------------------*/
/* pic16_dirregWithName - search for register by name                    */
/*-----------------------------------------------------------------*/
reg_info *
pic16_dirregWithName(const char *name)
{
  int hkey;
  reg_info *reg;

  if(!name)
    return NULL;

  /* hash the name to get a key */

  hkey = regname2key(name);

//      fprintf(stderr, "%s:%d: name = %s\thash = %d\n", __FUNCTION__, __LINE__, name, hkey);

  reg = hTabFirstItemWK(dynDirectRegNames, hkey);

  while(reg) {

    if(STRCASECMP(reg->name, name) == 0) {
//              fprintf(stderr, "%s:%d: FOUND name = %s\thash = %d\n", __FUNCTION__, __LINE__, reg->name, hkey);
      return(reg);
    }

    reg = hTabNextItemWK (dynDirectRegNames);

  }

  return NULL; // name wasn't found in the hash table
}

/*-----------------------------------------------------------------*/
/* pic16_allocregWithName - search for register by name                    */
/*-----------------------------------------------------------------*/
reg_info *
pic16_allocregWithName(const char *name)
{
  int hkey;
  reg_info *reg;

  if(!name)
    return NULL;

  /* hash the name to get a key */

  hkey = regname2key(name);

  //fprintf(stderr, "%s:%d: name = %s\thash = %d\n", __FUNCTION__, __LINE__, name, hkey);

  reg = hTabFirstItemWK(dynAllocRegNames, hkey);

  while(reg) {

    if(STRCASECMP(reg->name, name) == 0) {
      return(reg);
    }

    reg = hTabNextItemWK (dynAllocRegNames);

  }

  return NULL; // name wasn't found in the hash table

}


/*-----------------------------------------------------------------*/
/* pic16_procregWithName - search for register by name                    */
/*-----------------------------------------------------------------*/
reg_info *
pic16_procregWithName(const char *name)
{
  int hkey;
  reg_info *reg;

  if(!name)
    return NULL;

  /* hash the name to get a key */

  hkey = regname2key(name);

//      fprintf(stderr, "%s:%d: name = %s\thash = %d\n", __FUNCTION__, __LINE__, name, hkey);

  reg = hTabFirstItemWK(dynProcRegNames, hkey);

  while(reg) {

    if(STRCASECMP(reg->name, name) == 0) {
      return(reg);
    }

    reg = hTabNextItemWK (dynProcRegNames);

  }

  return NULL; // name wasn't found in the hash table

}

/*-----------------------------------------------------------------*/
/* pic16_accessregWithName - search for register by name           */
/*-----------------------------------------------------------------*/
reg_info *
pic16_accessregWithName(const char *name)
{
  int hkey;
  reg_info *reg;

  if(!name)
    return NULL;

  /* hash the name to get a key */

  hkey = regname2key(name);

//      fprintf(stderr, "%s:%d: name = %s\thash = %d\n", __FUNCTION__, __LINE__, name, hkey);

  reg = hTabFirstItemWK(dynAccessRegNames, hkey);

  while(reg) {

    if(STRCASECMP(reg->name, name) == 0) {
      return(reg);
    }

    reg = hTabNextItemWK (dynAccessRegNames);

  }

  return NULL; // name wasn't found in the hash table

}

reg_info *pic16_regWithName(const char *name)
{
  reg_info *reg;

        reg = pic16_dirregWithName( name );
        if(reg)return reg;

        reg = pic16_procregWithName( name );
        if(reg)return reg;

        reg = pic16_allocregWithName( name );
        if(reg)return reg;

        reg = pic16_accessregWithName( name );
        if(reg)return reg;

  return NULL;
}


/*-----------------------------------------------------------------*/
/* pic16_allocDirReg - allocates register of given type                  */
/*-----------------------------------------------------------------*/
reg_info *
pic16_allocDirReg (operand *op )
{
  reg_info *reg;
  char *name;

        if(!IS_SYMOP(op)) {
                debugLog ("%s BAD, op is NULL\n", __FUNCTION__);
//              fprintf(stderr, "%s BAD, op is NULL\n", __FUNCTION__);
          return NULL;
        }

        name = OP_SYMBOL (op)->rname[0] ? OP_SYMBOL (op)->rname : OP_SYMBOL (op)->name;


        if(!SPEC_OCLS( OP_SYM_ETYPE(op))) {
#if 0
                if(pic16_debug_verbose)
                {
                        fprintf(stderr, "%s:%d symbol %s(r:%s) is not assigned to a memmap\n", __FILE__, __LINE__,
                                OP_SYMBOL(op)->name, OP_SYMBOL(op)->rname);
                }
#endif
                return NULL;
        }

        if(!IN_DIRSPACE( SPEC_OCLS( OP_SYM_ETYPE(op)))
                || !IN_FARSPACE(SPEC_OCLS( OP_SYM_ETYPE(op))) ) {

#if 0
                if(pic16_debug_verbose) {
                        fprintf(stderr, "dispace:%d farspace:%d codespace:%d regspace:%d stack:%d eeprom: %d regparm: %d isparm: %d\n",
                                IN_DIRSPACE( SPEC_OCLS( OP_SYM_ETYPE(op))),
                                IN_FARSPACE( SPEC_OCLS( OP_SYM_ETYPE(op))),
                                IN_CODESPACE( SPEC_OCLS( OP_SYM_ETYPE(op))),
                                IN_REGSP( SPEC_OCLS( OP_SYM_ETYPE(op))),
                                IN_STACK( OP_SYM_ETYPE(op)),
                                SPEC_OCLS(OP_SYM_ETYPE(op)) == eeprom,
                                IS_REGPARM(OP_SYM_ETYPE(op)),
                                IS_PARM(op));

                        fprintf(stderr, "%s:%d symbol %s NOT in dirspace\n", __FILE__, __LINE__,
                        OP_SYMBOL(op)->name);
                }
#endif

        }



        if (IS_CODE ( OP_SYM_ETYPE(op)) ) {
//              fprintf(stderr, "%s:%d sym: %s in codespace\n", __FUNCTION__, __LINE__, OP_SYMBOL(op)->name);
                return NULL;
        }

        if(IS_ITEMP(op))return NULL;

//      if(IS_STATIC(OP_SYM_ETYPE(op)))return NULL;

        if(IN_STACK(OP_SYM_ETYPE(op)))return NULL;

        debugLog ("%s:%d symbol name %s\n", __FUNCTION__, __LINE__, name);
//      fprintf(stderr, "%s symbol name %s\tSTATIC:%d\n", __FUNCTION__,name, IS_STATIC(OP_SYM_ETYPE(op)));

        {
                if(SPEC_CONST ( OP_SYM_ETYPE(op)) && (IS_CHAR ( OP_SYM_ETYPE(op)) )) {
                        debugLog(" %d  const char\n",__LINE__);
                        debugLog(" value = %s \n",SPEC_CVAL( OP_SYM_ETYPE(op)));
//                      fprintf(stderr, " %d  const char\n",__LINE__);
//                      fprintf(stderr, " value = %s \n",SPEC_CVAL( OP_SYM_ETYPE(op)));
                }


                debugLog("  %d  storage class %d \n",__LINE__,SPEC_SCLS( OP_SYM_ETYPE(op)));
                if (IS_CODE ( OP_SYM_ETYPE(op)) )
                        debugLog(" %d  code space\n",__LINE__);

                if (IS_INTEGRAL ( OP_SYM_ETYPE(op)) )
                        debugLog(" %d  integral\n",__LINE__);

                if (IS_LITERAL ( OP_SYM_ETYPE(op)) )
                        debugLog(" %d  literal\n",__LINE__);

                if (IS_SPEC ( OP_SYM_ETYPE(op)) )
                        debugLog(" %d  specifier\n",__LINE__);

                debugAopGet(NULL, op);
        }


        reg = pic16_dirregWithName(name);

        if(!reg) {
          int regtype = REG_GPR;

                /* if this is at an absolute address, then get the address. */
                if (0 && SPEC_ABSA ( OP_SYM_ETYPE(op)) )
                  {
                    int address = 0;
                    address = SPEC_ADDR ( OP_SYM_ETYPE(op));
                    fprintf(stderr,"reg %s is at an absolute address: 0x%03x\n",name,address);
                  }

                /* Register wasn't found in hash, so let's create
                 * a new one and put it in the hash table AND in the
                 * dynDirectRegNames set */
                if(IS_CODE(OP_SYM_ETYPE(op)) || IN_CODESPACE( SPEC_OCLS( OP_SYM_ETYPE(op)))) {
                        debugLog("%s:%d sym: %s in codespace\n", __FUNCTION__, __LINE__, OP_SYMBOL(op)->name);
                  return NULL;
                }


#if 0
                if(OP_SYMBOL(op)->onStack) {
                        fprintf(stderr, "%s:%d onStack %s offset: %d\n", __FILE__, __LINE__,
                                OP_SYMBOL(op)->name, OP_SYMBOL(op)->stack);
                }
#endif

                if(!IN_DIRSPACE( SPEC_OCLS( OP_SYM_ETYPE(op)))
                        || !IN_FARSPACE(SPEC_OCLS( OP_SYM_ETYPE(op))) ) {

#if 0
                        if(pic16_debug_verbose)
                        {
                                fprintf(stderr, "dispace:%d farspace:%d codespace:%d regspace:%d stack:%d eeprom: %d\n",
                                        IN_DIRSPACE( SPEC_OCLS( OP_SYM_ETYPE(op))),
                                        IN_FARSPACE( SPEC_OCLS( OP_SYM_ETYPE(op))),
                                        IN_CODESPACE( SPEC_OCLS( OP_SYM_ETYPE(op))),
                                        IN_REGSP( SPEC_OCLS( OP_SYM_ETYPE(op))),
                                        IN_STACK( OP_SYM_ETYPE(op)),
                                        SPEC_OCLS(OP_SYM_ETYPE(op)) == eeprom);

                                        fprintf(stderr, "%s:%d symbol %s NOT in dirspace\n", __FILE__, __LINE__,
                                        OP_SYMBOL(op)->name);
                        }
#endif
                }

                reg = newReg(regtype, PO_DIR, rDirectIdx++, name,getSize (OP_SYMBOL (op)->type),0, op);
                debugLog ("%d  -- added %s to hash, size = %d\n", __LINE__, name,reg->size);

                if( SPEC_SCLS( OP_SYM_ETYPE( op ) ) == S_REGISTER ) {
                        fprintf(stderr, "%s:%d symbol %s is declared as register\n", __FILE__, __LINE__,
                                name);

                        reg->accessBank = 1;
                        checkAddReg(&pic16_dynAccessRegs, reg);
                        hTabAddItem(&dynAccessRegNames, regname2key(name), reg);

                  return (reg);
                }


//              if (SPEC_ABSA ( OP_SYM_ETYPE(op)) ) {
//                      fprintf(stderr, " ralloc.c at fixed address: %s - changing to REG_SFR\n",name);
//                      reg->type = REG_SFR;
//              }

                if (IS_BITVAR (OP_SYM_ETYPE(op))) {
//                      fprintf(stderr, "%s:%d adding %s in bit registers\n", __FILE__, __LINE__, reg->name);
                        addSet(&pic16_dynDirectBitRegs, reg);
                        reg->isBitField = 1;
                } else {
//                      fprintf(stderr, "%s:%d adding %s in direct registers\n", __FILE__, __LINE__, reg->name);
//                      addSet(&pic16_dynDirectRegs, reg);

#if 1
                  if(!(IS_STATIC(OP_SYM_ETYPE(op))
                      && OP_SYMBOL(op)->ival
                  ))
#endif
                    checkAddReg(&pic16_dynDirectRegs, reg);
                }

        } else {
//              debugLog ("  -- %s is declared at address 0x30000x\n",name);
          return (reg);                 /* This was NULL before, but since we found it
                                         * why not just return it?! */
        }

        if (SPEC_ABSA ( OP_SYM_ETYPE(op)) ) {
                reg->isFixed = 1;
                reg->address = SPEC_ADDR ( OP_SYM_ETYPE(op));

                /* work around for user defined registers in access bank */
                if((reg->address>= 0x00 && reg->address < pic16->acsSplitOfs)
                        || (reg->address >= (0xf00 + pic16->acsSplitOfs) && reg->address <= 0xfff))
                        reg->accessBank = 1;

                debugLog ("  -- and it is at a fixed address 0x%02x\n",reg->address);
        }

  return reg;
}

/*-----------------------------------------------------------------*/
/* pic16_allocRegByName - allocates register of given type                  */
/*-----------------------------------------------------------------*/
reg_info *
pic16_allocRegByName(const char *name, int size, operand *op)
{

  reg_info *reg;

  if(!name) {
    fprintf(stderr, "%s - allocating a NULL register\n",__FUNCTION__);
    exit(1);
  }

  /* First, search the hash table to see if there is a register with this name */
  reg = pic16_dirregWithName(name);

  if(!reg) {

    /* Register wasn't found in hash, so let's create
     * a new one and put it in the hash table AND in the
     * dynDirectRegNames set */

        //fprintf (stderr,"%s:%d symbol name %s\tregop= %p\n", __FUNCTION__, __LINE__, name, op);

    reg = newReg(REG_GPR, PO_DIR, rDirectIdx++, name,size,0, op);

    debugLog ("%d  -- added %s to hash, size = %d\n", __LINE__, name,reg->size);
        //fprintf(stderr, "  -- added %s to hash, size = %d\n", name,reg->size);

    //hTabAddItem(&dynDirectRegNames, regname2key(name), reg);  /* initially commented out */
    addSet(&pic16_dynDirectRegs, reg);
  }

  return reg;
}

/*-----------------------------------------------------------------*/
/* RegWithIdx - returns pointer to register with index number       */
/*-----------------------------------------------------------------*/
reg_info *pic16_typeRegWithIdx (int idx, int type, int fixed)
{

  reg_info *dReg;

  debugLog ("%s - requesting index = 0x%x\n", __FUNCTION__,idx);
//  fprintf(stderr, "%s - requesting index = 0x%x (type = %d [%s])\n", __FUNCTION__, idx, type, decodeRegType(type));

  switch (type) {

  case REG_GPR:
    if( (dReg = regWithIdx ( pic16_dynAllocRegs, idx, fixed)) != NULL) {

      debugLog ("Found a Dynamic Register!\n");
      return dReg;
    }
    if( (dReg = regWithIdx ( pic16_dynDirectRegs, idx, fixed)) != NULL ) {
      debugLog ("Found a Direct Register!\n");
      return dReg;
    }

        if( (dReg = regWithIdx ( pic16_dynInternalRegs, idx, fixed)) != NULL ) {
      debugLog ("Found an Internal Register!\n");
      return dReg;
    }

    break;
  case REG_STK:
    if( (dReg = regWithIdx ( pic16_dynStackRegs, idx, fixed)) != NULL ) {
      debugLog ("Found a Stack Register!\n");
      return dReg;
    }
    break;
  case REG_SFR:
    if( (dReg = regWithIdx ( pic16_dynProcessorRegs, idx, 1)) != NULL ) {
      debugLog ("Found a Processor Register!\n");
      return dReg;
    }

  case REG_CND:
  case REG_PTR:
  default:
    break;
  }


  return NULL;
}

/*-----------------------------------------------------------------*/
/* pic16_regWithIdx - returns pointer to register with index number*/
/*-----------------------------------------------------------------*/
reg_info *
pic16_regWithIdx (int idx)
{
  reg_info *dReg;

  if( (dReg = pic16_typeRegWithIdx(idx,REG_GPR,0)) != NULL)
    return dReg;

  if( (dReg = pic16_typeRegWithIdx(idx,REG_SFR,0)) != NULL)
    return dReg;

#if 0
  if( (dReg = pic16_typeRegWithIdx(idx,REG_STK,0)) != NULL)
    return dReg;
#endif

  return NULL;
}

/*-----------------------------------------------------------------*/
/* pic16_regWithIdx - returns pointer to register with index number       */
/*-----------------------------------------------------------------*/
reg_info *
pic16_allocWithIdx (int idx)
{

  reg_info *dReg=NULL;

  debugLog ("%s - allocating with index = 0x%x\n", __FUNCTION__,idx);
//  fprintf(stderr, "%s - allocating with index = 0x%x\n", __FUNCTION__,idx);

  if( (dReg = regWithIdx ( pic16_dynAllocRegs, idx,0)) != NULL) {

    debugLog ("Found a Dynamic Register!\n");
  } else if( (dReg = regWithIdx ( pic16_dynStackRegs, idx,0)) != NULL ) {
    debugLog ("Found a Stack Register!\n");
  } else if( (dReg = regWithIdx ( pic16_dynProcessorRegs, idx,1)) != NULL ) {
    debugLog ("Found a Processor Register!\n");
    fprintf(stderr, "Found a processor register! %s\n", dReg->name);
  } else if( (dReg = regWithIdx ( pic16_dynInternalRegs, idx,0)) != NULL ) {
    debugLog ("Found an Internal Register!\n");
  } else {

    debugLog ("Dynamic Register not found\n");

#if 1
        dReg = newReg(REG_GPR, PO_GPR_TEMP, idx, NULL, 1, 0, NULL);
        addSet(&pic16_dynAllocRegs, dReg);
        hTabAddItem(&dynAllocRegNames, regname2key(dReg->name), dReg);
#endif

        if(!dReg) {
//      return (NULL);
    //fprintf(stderr,"%s %d - requested register: 0x%x\n",__FUNCTION__,__LINE__,idx);
            werror (E_INTERNAL_ERROR, __FILE__, __LINE__,
                    "allocWithIdx not found");
            exit (1);
        }
  }

  dReg->wasUsed = 1;
  dReg->isFree = 0;

  return dReg;
}
/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
reg_info *
pic16_findFreeReg(short type)
{
  //  int i;
  reg_info* dReg;

  switch (type) {
  case REG_GPR:
    if((dReg = regFindFree(pic16_dynAllocRegs)) != NULL)
      return dReg;
//      return (addSet(&pic16_dynAllocRegs,newReg(REG_GPR, PO_GPR_TEMP,dynrIdx++,NULL,1,0, NULL)));
    return allocReg( REG_GPR );

  case REG_STK:

    if((dReg = regFindFree(pic16_dynStackRegs)) != NULL)
      return dReg;

    return NULL;

  case REG_PTR:
  case REG_CND:
  case REG_SFR:
  default:
    return NULL;
  }
}

reg_info *
pic16_findFreeRegNext(short type, reg_info *creg)
{
  //  int i;
  reg_info* dReg;

  switch (type) {
  case REG_GPR:
    if((dReg = regFindFreeNext(pic16_dynAllocRegs, creg)) != NULL)
      return dReg;
//        return (addSet(&pic16_dynAllocRegs,newReg(REG_GPR, PO_GPR_TEMP,dynrIdx++,NULL,1,0, NULL)));
    return (allocReg( REG_GPR ) );

  case REG_STK:

    if((dReg = regFindFreeNext(pic16_dynStackRegs, creg)) != NULL)
      return dReg;

    return NULL;

  case REG_PTR:
  case REG_CND:
  case REG_SFR:
  default:
    return NULL;
  }
}
/*-----------------------------------------------------------------*/
/* freeReg - frees a register                                      */
/*-----------------------------------------------------------------*/
static void
freeReg (reg_info * reg)
{
        debugLog ("%s\n", __FUNCTION__);
//      fprintf(stderr, "%s:%d register %s (%p) is freed\n", __FILE__, __LINE__, reg->name, reg);
        reg->isFree = 1;
}


/*-----------------------------------------------------------------*/
/* nFreeRegs - returns number of free registers                    */
/*-----------------------------------------------------------------*/
static int
nFreeRegs (int type)
{
  reg_info *reg;
  int nfr=0;


                /* although I fixed the register allocation/freeing scheme
                 * the for loop below doesn't give valid results. I do not
                 * know why yet. -- VR 10-Jan-2003 */

        return 100;


  /* dynamically allocate as many as we need and worry about
   * fitting them into a PIC later */

  debugLog ("%s\n", __FUNCTION__);

        for(reg = setFirstItem(pic16_dynAllocRegs); reg; reg=setNextItem(pic16_dynAllocRegs))
                if((reg->type == type) && reg->isFree)nfr++;

        fprintf(stderr, "%s:%d # of free registers= %d\n", __FILE__, __LINE__, nfr);
  return nfr;
}

/*-----------------------------------------------------------------*/
/* nfreeRegsType - free registers with type                         */
/*-----------------------------------------------------------------*/
static int
nfreeRegsType (int type)
{
  int nfr;
  debugLog ("%s\n", __FUNCTION__);
  if (type == REG_PTR)
    {
      if ((nfr = nFreeRegs (type)) == 0)
        return nFreeRegs (REG_GPR);
    }

  return nFreeRegs (type);
}
#if 0
static void writeSetUsedRegs(FILE *of, set *dRegs)
{

  reg_info *dReg;

  for (dReg = setFirstItem(dRegs) ; dReg ;
       dReg = setNextItem(dRegs)) {

    if(dReg->wasUsed)
      fprintf (of, "\t%s\n",dReg->name);
  }

}
#endif

extern void pic16_groupRegistersInSection(set *regset);

extern void pic16_dump_equates(FILE *of, set *equs);
extern void pic16_dump_access(FILE *of, set *section);
//extern void pic16_dump_map(void);
extern void pic16_dump_usection(FILE *of, set *section, int fix);
extern void pic16_dump_isection(FILE *of, set *section, int fix);
extern void pic16_dump_int_registers(FILE *of, set *section);
extern void pic16_dump_idata(FILE *of, set *idataSymSet);

extern void pic16_dump_gsection(FILE *of, set *sections);

static void packBits(set *bregs)
{
  set *regset;
  reg_info *breg;
  reg_info *bitfield=NULL;
  reg_info *relocbitfield=NULL;
  int bit_no=0;
  int byte_no=-1;
  char buffer[20];


  for (regset = bregs ; regset ;
       regset = regset->next) {

    breg = regset->item;
    breg->isBitField = 1;
    //fprintf(stderr,"bit reg: %s\n",breg->name);

    if(breg->isFixed) {
      //fprintf(stderr,"packing bit at fixed address = 0x%03x\n",breg->address);

      bitfield = pic16_typeRegWithIdx (breg->address >> 3, -1 , 1);
      breg->rIdx = breg->address & 7;
      breg->address >>= 3;

      if(!bitfield) {
        SNPRINTF(buffer, sizeof(buffer), "fbitfield%02x", breg->address);
        //fprintf(stderr,"new bit field\n");
        bitfield = newReg(REG_SFR, PO_GPR_BIT,breg->address,buffer,1,0, NULL);
        bitfield->isBitField = 1;
        bitfield->isFixed = 1;
        bitfield->address = breg->address;
        addSet(&pic16_dynDirectRegs,bitfield);
        //hTabAddItem(&dynDirectRegNames, regname2key(buffer), bitfield);
      } else {
        //fprintf(stderr,"  which is occupied by %s (addr = %d)\n",bitfield->name,bitfield->address);
        ;
      }
      breg->reg_alias = bitfield;
      bitfield = NULL;

    } else {
      if(!relocbitfield || bit_no >7) {
        byte_no++;
        bit_no=0;
        SNPRINTF(buffer, sizeof(buffer), "bitfield%d", byte_no);
        //fprintf(stderr,"new relocatable bit field\n");
        relocbitfield = newReg(REG_GPR, PO_GPR_BIT,rDirectIdx++,buffer,1,0, NULL);
        relocbitfield->isBitField = 1;
        addSet(&pic16_dynDirectRegs,relocbitfield);
        //hTabAddItem(&dynDirectRegNames, regname2key(buffer), relocbitfield);

      }

      breg->reg_alias = relocbitfield;
      breg->address = rDirectIdx;   /* byte_no; */
      breg->rIdx = bit_no++;
    }
  }

}

void pic16_writeUsedRegs(FILE *of)
{
  packBits(pic16_dynDirectBitRegs);

  pic16_groupRegistersInSection(pic16_dynAllocRegs);
  pic16_groupRegistersInSection(pic16_dynInternalRegs);
  pic16_groupRegistersInSection(pic16_dynStackRegs);
  pic16_groupRegistersInSection(pic16_dynDirectRegs);
  pic16_groupRegistersInSection(pic16_dynDirectBitRegs);
  pic16_groupRegistersInSection(pic16_dynProcessorRegs);
  pic16_groupRegistersInSection(pic16_dynAccessRegs);

  /* dump equates */
  pic16_dump_equates(of, pic16_equ_data);

//      pic16_dump_esection(of, pic16_rel_eedata, 0);
//      pic16_dump_esection(of, pic16_fix_eedata, 0);

  /* dump access bank symbols */
  pic16_dump_access(of, pic16_acs_udata);

  /* dump initialised data */
  pic16_dump_isection(of, rel_idataSymSet, 0);
  pic16_dump_isection(of, fix_idataSymSet, 1);

  if(!pic16_options.xinst) {
    /* dump internal registers */
    pic16_dump_int_registers(of, pic16_int_regs);
  }

  /* dump generic section variables */
  pic16_dump_gsection(of, sectNames);

  /* dump other variables */
  pic16_dump_usection(of, pic16_rel_udata, 0);
  pic16_dump_usection(of, pic16_fix_udata, 1);
}


/*-----------------------------------------------------------------*/
/* computeSpillable - given a point find the spillable live ranges */
/*-----------------------------------------------------------------*/
static bitVect *
computeSpillable (iCode * ic)
{
  bitVect *spillable;

  debugLog ("%s\n", __FUNCTION__);
  /* spillable live ranges are those that are live at this
     point . the following categories need to be subtracted
     from this set.
     a) - those that are already spilt
     b) - if being used by this one
     c) - defined by this one */

  spillable = bitVectCopy (ic->rlive);
  spillable =
    bitVectCplAnd (spillable, _G.spiltSet);     /* those already spilt */
  spillable =
    bitVectCplAnd (spillable, ic->uses);        /* used in this one */
  bitVectUnSetBit (spillable, ic->defKey);
  spillable = bitVectIntersect (spillable, _G.regAssigned);
  return spillable;

}

/*-----------------------------------------------------------------*/
/* noSpilLoc - return true if a variable has no spil location      */
/*-----------------------------------------------------------------*/
static int
noSpilLoc (symbol * sym, eBBlock * ebp, iCode * ic)
{
  debugLog ("%s\n", __FUNCTION__);
  return (SYM_SPIL_LOC (sym) ? 0 : 1);
}

/*-----------------------------------------------------------------*/
/* hasSpilLoc - will return 1 if the symbol has spil location      */
/*-----------------------------------------------------------------*/
static int
hasSpilLoc (symbol * sym, eBBlock * ebp, iCode * ic)
{
  debugLog ("%s\n", __FUNCTION__);
  return (SYM_SPIL_LOC (sym) ? 1 : 0);
}

/*-----------------------------------------------------------------*/
/* directSpilLoc - will return 1 if the splilocation is in direct  */
/*-----------------------------------------------------------------*/
static int
directSpilLoc (symbol * sym, eBBlock * ebp, iCode * ic)
{
  debugLog ("%s\n", __FUNCTION__);
  if (SYM_SPIL_LOC (sym) &&
      (IN_DIRSPACE (SPEC_OCLS (SYM_SPIL_LOC (sym)->etype))))
    return 1;
  else
    return 0;
}

/*-----------------------------------------------------------------*/
/* hasSpilLocnoUptr - will return 1 if the symbol has spil location */
/*                    but is not used as a pointer                 */
/*-----------------------------------------------------------------*/
static int
hasSpilLocnoUptr (symbol * sym, eBBlock * ebp, iCode * ic)
{
  debugLog ("%s\n", __FUNCTION__);
  return ((SYM_SPIL_LOC (sym) && !sym->uptr) ? 1 : 0);
}

/*-----------------------------------------------------------------*/
/* rematable - will return 1 if the remat flag is set              */
/*-----------------------------------------------------------------*/
static int
rematable (symbol * sym, eBBlock * ebp, iCode * ic)
{
  debugLog ("%s\n", __FUNCTION__);
  return sym->remat;
}

/*-----------------------------------------------------------------*/
/* notUsedInRemaining - not used or defined in remain of the block */
/*-----------------------------------------------------------------*/
static int
notUsedInRemaining (symbol * sym, eBBlock * ebp, iCode * ic)
{
  debugLog ("%s\n", __FUNCTION__);
  return ((usedInRemaining (operandFromSymbol (sym), ic) ? 0 : 1) &&
          allDefsOutOfRange (sym->defs, ebp->fSeq, ebp->lSeq));
}

/*-----------------------------------------------------------------*/
/* allLRs - return true for all                                    */
/*-----------------------------------------------------------------*/
static int
allLRs (symbol * sym, eBBlock * ebp, iCode * ic)
{
  debugLog ("%s\n", __FUNCTION__);
  return 1;
}

/*-----------------------------------------------------------------*/
/* liveRangesWith - applies function to a given set of live range  */
/*-----------------------------------------------------------------*/
static set *
liveRangesWith (bitVect * lrs, int (func) (symbol *, eBBlock *, iCode *),
                eBBlock * ebp, iCode * ic)
{
  set *rset = NULL;
  int i;

  debugLog ("%s\n", __FUNCTION__);
  if (!lrs || !lrs->size)
    return NULL;

  for (i = 1; i < lrs->size; i++)
    {
      symbol *sym;
      if (!bitVectBitValue (lrs, i))
        continue;

      /* if we don't find it in the live range
         hash table we are in serious trouble */
      if (!(sym = hTabItemWithKey (liveRanges, i)))
        {
          werror (E_INTERNAL_ERROR, __FILE__, __LINE__,
                  "liveRangesWith could not find liveRange");
          exit (1);
        }

      if (func (sym, ebp, ic) && bitVectBitValue (_G.regAssigned, sym->key))
        addSetHead (&rset, sym);
    }

  return rset;
}


/*-----------------------------------------------------------------*/
/* leastUsedLR - given a set determines which is the least used    */
/*-----------------------------------------------------------------*/
static symbol *
leastUsedLR (set * sset)
{
  symbol *sym = NULL, *lsym = NULL;

  debugLog ("%s\n", __FUNCTION__);
  sym = lsym = setFirstItem (sset);

  if (!lsym)
    return NULL;

  for (; lsym; lsym = setNextItem (sset))
    {

      /* if usage is the same then prefer
         the spill the smaller of the two */
      if (lsym->used == sym->used)
        if (getSize (lsym->type) < getSize (sym->type))
          sym = lsym;

      /* if less usage */
      if (lsym->used < sym->used)
        sym = lsym;

    }

  setToNull ((void *) &sset);
  sym->blockSpil = 0;
  return sym;
}

/*-----------------------------------------------------------------*/
/* noOverLap - will iterate through the list looking for over lap  */
/*-----------------------------------------------------------------*/
static int
noOverLap (set * itmpStack, symbol * fsym)
{
  symbol *sym;
  debugLog ("%s\n", __FUNCTION__);


  for (sym = setFirstItem (itmpStack); sym;
       sym = setNextItem (itmpStack))
    {
      if (sym->liveTo > fsym->liveFrom)
        return 0;

    }

  return 1;
}

/*-----------------------------------------------------------------*/
/* isFree - will return 1 if the a free spil location is found     */
/*-----------------------------------------------------------------*/
static
DEFSETFUNC (isFree)
{
  symbol *sym = item;
  V_ARG (symbol **, sloc);
  V_ARG (symbol *, fsym);

  debugLog ("%s\n", __FUNCTION__);
  /* if already found */
  if (*sloc)
    return 0;

  /* if it is free && and the itmp assigned to
     this does not have any overlapping live ranges
     with the one currently being assigned and
     the size can be accomodated  */
  if (sym->isFree &&
      noOverLap (sym->usl.itmpStack, fsym) &&
      getSize (sym->type) >= getSize (fsym->type))
    {
      *sloc = sym;
      return 1;
    }

  return 0;
}

/*-----------------------------------------------------------------*/
/* spillLRWithPtrReg :- will spil those live ranges which use PTR  */
/*-----------------------------------------------------------------*/
static void
spillLRWithPtrReg (symbol * forSym)
{
  symbol *lrsym;
  reg_info *r0, *r1;
  int k;

  debugLog ("%s\n", __FUNCTION__);
  if (!_G.regAssigned ||
      bitVectIsZero (_G.regAssigned))
    return;

  r0 = pic16_regWithIdx (R0_IDX);
  r1 = pic16_regWithIdx (R1_IDX);

  /* for all live ranges */
  for (lrsym = hTabFirstItem (liveRanges, &k); lrsym;
       lrsym = hTabNextItem (liveRanges, &k))
    {
      int j;

      /* if no registers assigned to it or
         spilt */
      /* if it does not overlap with this then
         not need to spill it */

      if (lrsym->isspilt || !lrsym->nRegs ||
          (lrsym->liveTo < forSym->liveFrom))
        continue;

      /* go thru the registers : if it is either
         r0 or r1 then spil it */
      for (j = 0; j < lrsym->nRegs; j++)
        if (lrsym->regs[j] == r0 ||
            lrsym->regs[j] == r1)
          {
            spillThis (lrsym);
            break;
          }
    }

}

/*-----------------------------------------------------------------*/
/* createStackSpil - create a location on the stack to spil        */
/*-----------------------------------------------------------------*/
static symbol *
createStackSpil (symbol * sym)
{
  symbol *sloc = NULL;
  int useXstack, model, noOverlay;
  char slocBuffer[120];

  debugLog ("%s\n", __FUNCTION__);

  /* first go try and find a free one that is already
     existing on the stack */
  if (applyToSet (_G.stackSpil, isFree, &sloc, sym))
    {
      /* found a free one : just update & return */
      SYM_SPIL_LOC (sym) = sloc;
      sym->stackSpil = 1;
      sloc->isFree = 0;
      addSetHead (&sloc->usl.itmpStack, sym);
      return sym;
    }

  SNPRINTF(slocBuffer, sizeof(slocBuffer), "sloc%d", _G.slocNum++);
  sloc = newiTemp(slocBuffer);

  /* set the type to the spilling symbol */
  sloc->type = copyLinkChain (sym->type);
  sloc->etype = getSpec (sloc->type);
  SPEC_SCLS (sloc->etype) = S_DATA;
  SPEC_EXTR (sloc->etype) = 0;
  SPEC_STAT (sloc->etype) = 0;

  /* we don't allow it to be allocated`
     onto the external stack since : so we
     temporarily turn it off ; we also
     turn off memory model to prevent
     the spil from going to the external storage
     and turn off overlaying
   */

  useXstack = options.useXstack;
  model = options.model;
  noOverlay = options.noOverlay;
  options.noOverlay = 1;
  options.model = options.useXstack = 0;

  allocLocal (sloc);

  options.useXstack = useXstack;
  options.model = model;
  options.noOverlay = noOverlay;
  sloc->isref = 1;              /* to prevent compiler warning */

  /* if it is on the stack then update the stack */
  if (IN_STACK (sloc->etype))
    {
      currFunc->stack += getSize (sloc->type);
      _G.stackExtend += getSize (sloc->type);
    }
  else
    _G.dataExtend += getSize (sloc->type);

  /* add it to the _G.stackSpil set */
  addSetHead (&_G.stackSpil, sloc);
  SYM_SPIL_LOC (sym) = sloc;
  sym->stackSpil = 1;

  /* add it to the set of itempStack set
     of the spill location */
  addSetHead (&sloc->usl.itmpStack, sym);
  return sym;
}

/*-----------------------------------------------------------------*/
/* isSpiltOnStack - returns true if the spil location is on stack  */
/*-----------------------------------------------------------------*/
static bool
isSpiltOnStack (symbol * sym)
{
  sym_link *etype;

  debugLog ("%s\n", __FUNCTION__);
  if (!sym)
    return FALSE;

  if (!sym->isspilt)
    return FALSE;

/*     if (sym->_G.stackSpil) */
/*      return TRUE; */

  if (!SYM_SPIL_LOC (sym))
    return FALSE;

  etype = getSpec (SYM_SPIL_LOC (sym)->type);
  if (IN_STACK (etype))
    return TRUE;

  return FALSE;
}

/*-----------------------------------------------------------------*/
/* spillThis - spils a specific operand                            */
/*-----------------------------------------------------------------*/
static void
spillThis (symbol * sym)
{
  int i;
  debugLog ("%s : %s\n", __FUNCTION__, sym->rname);

  /* if this is rematerializable or has a spillLocation
     we are okay, else we need to create a spillLocation
     for it */
  if (!(sym->remat || SYM_SPIL_LOC (sym)))
    createStackSpil (sym);


  /* mark it has spilt & put it in the spilt set */
  sym->isspilt = 1;
  _G.spiltSet = bitVectSetBit (_G.spiltSet, sym->key);

  bitVectUnSetBit (_G.regAssigned, sym->key);

  for (i = 0; i < sym->nRegs; i++)

    if (sym->regs[i])
      {
        freeReg (sym->regs[i]);
        sym->regs[i] = NULL;
      }

  /* if spilt on stack then free up r0 & r1
     if they could have been assigned to some
     LIVE ranges */
  if (!pic16_ptrRegReq && isSpiltOnStack (sym))
    {
      pic16_ptrRegReq++;
      spillLRWithPtrReg (sym);
    }

  if (SYM_SPIL_LOC (sym) && !sym->remat)
    SYM_SPIL_LOC (sym)->allocreq = 1;
  return;
}

/*-----------------------------------------------------------------*/
/* selectSpil - select a iTemp to spil : rather a simple procedure */
/*-----------------------------------------------------------------*/
static symbol *
selectSpil (iCode * ic, eBBlock * ebp, symbol * forSym)
{
  bitVect *lrcs = NULL;
  set *selectS;
  symbol *sym;

  debugLog ("%s\n", __FUNCTION__);
  /* get the spillable live ranges */
  lrcs = computeSpillable (ic);

  /* get all live ranges that are rematerizable */
  if ((selectS = liveRangesWith (lrcs, rematable, ebp, ic)))
    {

      /* return the least used of these */
      return leastUsedLR (selectS);
    }

  /* get live ranges with spillLocations in direct space */
  if ((selectS = liveRangesWith (lrcs, directSpilLoc, ebp, ic)))
    {
      sym = leastUsedLR (selectS);
      strcpy (sym->rname, (SYM_SPIL_LOC (sym)->rname[0] ?
                           SYM_SPIL_LOC (sym)->rname :
                           SYM_SPIL_LOC (sym)->name));
      sym->spildir = 1;
      /* mark it as allocation required */
      SYM_SPIL_LOC (sym)->allocreq = 1;
      return sym;
    }

  /* if the symbol is local to the block then */
  if (forSym->liveTo < ebp->lSeq)
    {

      /* check if there are any live ranges allocated
         to registers that are not used in this block */
      if (!_G.blockSpil && (selectS = liveRangesWith (lrcs, notUsedInBlock, ebp, ic)))
        {
          sym = leastUsedLR (selectS);
          /* if this is not rematerializable */
          if (!sym->remat)
            {
              _G.blockSpil++;
              sym->blockSpil = 1;
            }
          return sym;
        }

      /* check if there are any live ranges that not
         used in the remainder of the block */
      if (!_G.blockSpil &&
          !isiCodeInFunctionCall (ic) &&
          (selectS = liveRangesWith (lrcs, notUsedInRemaining, ebp, ic)))
        {
          sym = leastUsedLR (selectS);
          if (!sym->remat)
            {
              sym->remainSpil = 1;
              _G.blockSpil++;
            }
          return sym;
        }
    }

  /* find live ranges with spillocation && not used as pointers */
  if ((selectS = liveRangesWith (lrcs, hasSpilLocnoUptr, ebp, ic)))
    {

      sym = leastUsedLR (selectS);
      /* mark this as allocation required */
      SYM_SPIL_LOC (sym)->allocreq = 1;
      return sym;
    }

  /* find live ranges with spillocation */
  if ((selectS = liveRangesWith (lrcs, hasSpilLoc, ebp, ic)))
    {

      sym = leastUsedLR (selectS);
      SYM_SPIL_LOC (sym)->allocreq = 1;
      return sym;
    }

  /* couldn't find then we need to create a spil
     location on the stack , for which one? the least
     used ofcourse */
  if ((selectS = liveRangesWith (lrcs, noSpilLoc, ebp, ic)))
    {

      /* return a created spil location */
      sym = createStackSpil (leastUsedLR (selectS));
      SYM_SPIL_LOC (sym)->allocreq = 1;
      return sym;
    }

  /* this is an extreme situation we will spill
     this one : happens very rarely but it does happen */
  spillThis (forSym);
  return forSym;

}

/*-----------------------------------------------------------------*/
/* spilSomething - spil some variable & mark registers as free     */
/*-----------------------------------------------------------------*/
static bool
spilSomething (iCode * ic, eBBlock * ebp, symbol * forSym)
{
  symbol *ssym;
  int i;

  debugLog ("%s\n", __FUNCTION__);
  /* get something we can spil */
  ssym = selectSpil (ic, ebp, forSym);

  /* mark it as spilt */
  ssym->isspilt = 1;
  _G.spiltSet = bitVectSetBit (_G.spiltSet, ssym->key);

  /* mark it as not register assigned &
     take it away from the set */
  bitVectUnSetBit (_G.regAssigned, ssym->key);

  /* mark the registers as free */
  for (i = 0; i < ssym->nRegs; i++)
    if (ssym->regs[i])
      freeReg (ssym->regs[i]);

  /* if spilt on stack then free up r0 & r1
     if they could have been assigned to as gprs */
  if (!pic16_ptrRegReq && isSpiltOnStack (ssym))
    {
      pic16_ptrRegReq++;
      spillLRWithPtrReg (ssym);
    }

  /* if this was a block level spil then insert push & pop
     at the start & end of block respectively */
  if (ssym->blockSpil)
    {
      iCode *nic = newiCode (IPUSH, operandFromSymbol (ssym), NULL);
      /* add push to the start of the block */
      addiCodeToeBBlock (ebp, nic, (ebp->sch->op == LABEL ?
                                    ebp->sch->next : ebp->sch));
      nic = newiCode (IPOP, operandFromSymbol (ssym), NULL);
      /* add pop to the end of the block */
      addiCodeToeBBlock (ebp, nic, NULL);
    }

  /* if spilt because not used in the remainder of the
     block then add a push before this instruction and
     a pop at the end of the block */
  if (ssym->remainSpil)
    {

      iCode *nic = newiCode (IPUSH, operandFromSymbol (ssym), NULL);
      /* add push just before this instruction */
      addiCodeToeBBlock (ebp, nic, ic);

      nic = newiCode (IPOP, operandFromSymbol (ssym), NULL);
      /* add pop to the end of the block */
      addiCodeToeBBlock (ebp, nic, NULL);
    }

  if (ssym == forSym)
    return FALSE;
  else
    return TRUE;
}

/*-----------------------------------------------------------------*/
/* getRegPtr - will try for PTR if not a GPR type if not spil      */
/*-----------------------------------------------------------------*/
static reg_info *
getRegPtr (iCode * ic, eBBlock * ebp, symbol * sym)
{
  reg_info *reg;
  int j;

  debugLog ("%s\n", __FUNCTION__);
tryAgain:
  /* try for a ptr type */
  if ((reg = allocReg (REG_PTR)))
    return reg;

  /* try for gpr type */
  if ((reg = allocReg (REG_GPR)))
    return reg;

  /* we have to spil */
  if (!spilSomething (ic, ebp, sym))
    return NULL;

  /* make sure partially assigned registers aren't reused */
  for (j=0; j<=sym->nRegs; j++)
    if (sym->regs[j])
      sym->regs[j]->isFree = 0;

  /* this looks like an infinite loop but
     in really selectSpil will abort  */
  goto tryAgain;
}

/*-----------------------------------------------------------------*/
/* getRegGpr - will try for GPR if not spil                        */
/*-----------------------------------------------------------------*/
static reg_info *
getRegGpr (iCode * ic, eBBlock * ebp, symbol * sym)
{
  reg_info *reg;
  int j;

  debugLog ("%s\n", __FUNCTION__);
tryAgain:
  /* try for gpr type */
  if ((reg = allocReg (REG_GPR)))
    return reg;

  if (!pic16_ptrRegReq)
    if ((reg = allocReg (REG_PTR)))
      return reg;

  /* we have to spil */
  if (!spilSomething (ic, ebp, sym))
    return NULL;

  /* make sure partially assigned registers aren't reused */
  for (j=0; j<=sym->nRegs; j++)
    if (sym->regs[j])
      sym->regs[j]->isFree = 0;

  /* this looks like an infinite loop but
     in really selectSpil will abort  */
  goto tryAgain;
}

/*-----------------------------------------------------------------*/
/* symHasReg - symbol has a given register                         */
/*-----------------------------------------------------------------*/
static bool
symHasReg (symbol *sym, reg_info *reg)
{
  int i;

  debugLog ("%s\n", __FUNCTION__);
  for (i = 0; i < sym->nRegs; i++)
    if (sym->regs[i] == reg)
      return TRUE;

  return FALSE;
}

/*-----------------------------------------------------------------*/
/* deassignLRs - check the live to and if they have registers & are */
/*               not spilt then free up the registers              */
/*-----------------------------------------------------------------*/
static void
deassignLRs (iCode * ic, eBBlock * ebp)
{
  symbol *sym;
  int k;
  symbol *result;

  debugLog ("%s\n", __FUNCTION__);
  for (sym = hTabFirstItem (liveRanges, &k); sym;
       sym = hTabNextItem (liveRanges, &k))
    {

      symbol *psym = NULL;
      /* if it does not end here */
      if (sym->liveTo > ic->seq)
        continue;

      /* if it was spilt on stack then we can
         mark the stack spil location as free */
      if (sym->isspilt)
        {
          if (sym->stackSpil)
            {
              SYM_SPIL_LOC (sym)->isFree = 1;
              sym->stackSpil = 0;
            }
          continue;
        }

      if (!bitVectBitValue (_G.regAssigned, sym->key))
        continue;

      /* special case for shifting: there is a case where shift count
       * can be allocated in the same register as the result, so do not
       * free right registers if same as result registers, cause genShiftLeft
       * will fail -- VR */
       if(ic->op == LEFT_OP)
         continue;

      /* special case check if this is an IFX &
         the privious one was a pop and the
         previous one was not spilt then keep track
         of the symbol */
      if (ic->op == IFX && ic->prev &&
          ic->prev->op == IPOP &&
          !ic->prev->parmPush &&
          !OP_SYMBOL (IC_LEFT (ic->prev))->isspilt)
        psym = OP_SYMBOL (IC_LEFT (ic->prev));

      if (sym->nRegs)
        {
          int i = 0;

          bitVectUnSetBit (_G.regAssigned, sym->key);

          /* if the result of this one needs registers
             and does not have it then assign it right
             away */
          if (IC_RESULT (ic) &&
              !(SKIP_IC2 (ic) ||        /* not a special icode */
                ic->op == JUMPTABLE ||
                ic->op == IFX ||
                ic->op == IPUSH ||
                ic->op == IPOP ||
                ic->op == RETURN ||
                POINTER_SET (ic)) &&
              (result = OP_SYMBOL (IC_RESULT (ic))) &&  /* has a result */
              result->liveTo > ic->seq &&       /* and will live beyond this */
              result->liveTo <= ebp->lSeq &&    /* does not go beyond this block */
              result->liveFrom == ic->seq &&    /* does not start before here */
              result->regType == sym->regType &&        /* same register types */
              result->nRegs &&  /* which needs registers */
              !result->isspilt &&       /* and does not already have them */
              !result->remat &&
              !bitVectBitValue (_G.regAssigned, result->key) &&
          /* the number of free regs + number of regs in this LR
             can accomodate the what result Needs */
              ((nfreeRegsType (result->regType) +
                sym->nRegs) >= result->nRegs)
            )
            {

              for (i = 0; i < result->nRegs; i++)
                if (i < sym->nRegs)
                  result->regs[i] = sym->regs[i];
                else
                  result->regs[i] = getRegGpr (ic, ebp, result);

              _G.regAssigned = bitVectSetBit (_G.regAssigned, result->key);

            }

          /* free the remaining */
          for (; i < sym->nRegs; i++)
            {
              if (psym)
                {
                  if (!symHasReg (psym, sym->regs[i]))
                    freeReg (sym->regs[i]);
                }
              else
                freeReg (sym->regs[i]);
            }
        }
    }
}


/*-----------------------------------------------------------------*/
/* reassignLR - reassign this to registers                         */
/*-----------------------------------------------------------------*/
static void
reassignLR (operand * op)
{
  symbol *sym = OP_SYMBOL (op);
  int i;

  debugLog ("%s\n", __FUNCTION__);
  /* not spilt any more */
  sym->isspilt = sym->blockSpil = sym->remainSpil = 0;
  bitVectUnSetBit (_G.spiltSet, sym->key);

  _G.regAssigned = bitVectSetBit (_G.regAssigned, sym->key);

  _G.blockSpil--;

  for (i = 0; i < sym->nRegs; i++)
    sym->regs[i]->isFree = 0;
}

/*-----------------------------------------------------------------*/
/* willCauseSpill - determines if allocating will cause a spill    */
/*-----------------------------------------------------------------*/
static int
willCauseSpill (int nr, int rt)
{
  debugLog ("%s\n", __FUNCTION__);
  /* first check if there are any avlb registers
     of te type required */
  if (rt == REG_PTR)
    {
      /* special case for pointer type
         if pointer type not avlb then
         check for type gpr */
      if (nFreeRegs (rt) >= nr)
        return 0;
      if (nFreeRegs (REG_GPR) >= nr)
        return 0;
    }
  else
    {
      if (pic16_ptrRegReq)
        {
          if (nFreeRegs (rt) >= nr)
            return 0;
        }
      else
        {
          if (nFreeRegs (REG_PTR) +
              nFreeRegs (REG_GPR) >= nr)
            return 0;
        }
    }

  debugLog (" ... yep it will (cause a spill)\n");
  /* it will cause a spil */
  return 1;
}

/*-----------------------------------------------------------------*/
/* positionRegs - the allocator can allocate same registers to res- */
/* ult and operand, if this happens make sure they are in the same */
/* position as the operand otherwise chaos results                 */
/*-----------------------------------------------------------------*/
static void
positionRegs (symbol * result, symbol * opsym, int lineno)
{
  int count = min (result->nRegs, opsym->nRegs);
  int i, j = 0, shared = 0;

  debugLog ("%s\n", __FUNCTION__);
  /* if the result has been spilt then cannot share */
  if (opsym->isspilt)
    return;
again:
  shared = 0;
  /* first make sure that they actually share */
  for (i = 0; i < count; i++)
    {
      for (j = 0; j < count; j++)
        {
          if (result->regs[i] == opsym->regs[j] && i != j)
            {
              shared = 1;
              goto xchgPositions;
            }
        }
    }
xchgPositions:
  if (shared)
    {
      reg_info *tmp = result->regs[i];
      result->regs[i] = result->regs[j];
      result->regs[j] = tmp;
      goto again;
    }
}

/*------------------------------------------------------------------*/
/* verifyRegsAssigned - make sure an iTemp is properly initialized; */
/* it should either have registers or have beed spilled. Otherwise, */
/* there was an uninitialized variable, so just spill this to get   */
/* the operand in a valid state.                                    */
/*------------------------------------------------------------------*/
static void
verifyRegsAssigned (operand *op, iCode * ic)
{
  symbol * sym;

  if (!op) return;
  if (!IS_ITEMP (op)) return;

  sym = OP_SYMBOL (op);
  if (sym->isspilt) return;
  if (!sym->nRegs) return;
  if (sym->regs[0]) return;

  werrorfl (ic->filename, ic->lineno, W_LOCAL_NOINIT,
            sym->prereqv ? sym->prereqv->name : sym->name);
  spillThis (sym);
}


/*-----------------------------------------------------------------*/
/* serialRegAssign - serially allocate registers to the variables  */
/*-----------------------------------------------------------------*/
static void
serialRegAssign (eBBlock ** ebbs, int count)
{
  int i;
  iCode *ic;

  debugLog ("%s\n", __FUNCTION__);
  /* for all blocks */
  for (i = 0; i < count; i++)
    {
      if (ebbs[i]->noPath &&
          (ebbs[i]->entryLabel != entryLabel &&
           ebbs[i]->entryLabel != returnLabel))
        continue;

      /* of all instructions do */
      for (ic = ebbs[i]->sch; ic; ic = ic->next)
        {

          debugLog ("  op: %s\n", pic16_decodeOp (ic->op));

                if(IC_RESULT(ic) && !IS_ITEMP( IC_RESULT(ic)))
                        pic16_allocDirReg(IC_RESULT(ic));

                if(IC_LEFT(ic) && !IS_ITEMP( IC_LEFT(ic)))
                        pic16_allocDirReg(IC_LEFT(ic));

                if(IC_RIGHT(ic) && !IS_ITEMP( IC_RIGHT(ic)))
                        pic16_allocDirReg(IC_RIGHT(ic));

          /* if this is an ipop that means some live
             range will have to be assigned again */
          if (ic->op == IPOP)
            reassignLR (IC_LEFT (ic));

          /* if result is present && is a true symbol */
          if (IC_RESULT (ic) && ic->op != IFX &&
              IS_TRUE_SYMOP (IC_RESULT (ic)))
            OP_SYMBOL (IC_RESULT (ic))->allocreq = 1;

          /* take away registers from live
             ranges that end at this instruction */
          deassignLRs (ic, ebbs[i]);

          /* some don't need registers */
          if (SKIP_IC2 (ic) ||
              ic->op == JUMPTABLE ||
              ic->op == IFX ||
              ic->op == IPUSH ||
              ic->op == IPOP ||
              (IC_RESULT (ic) && POINTER_SET (ic)))
            continue;

          /* now we need to allocate registers
             only for the result */
          if (IC_RESULT (ic))
            {
              symbol *sym = OP_SYMBOL (IC_RESULT (ic));
              bitVect *spillable;
              int willCS;
              int j;
              int ptrRegSet = 0;

              /* Make sure any spill location is definately allocated */
              if (sym->isspilt && !sym->remat && SYM_SPIL_LOC (sym) &&
                  !SYM_SPIL_LOC (sym)->allocreq)
                {
                  SYM_SPIL_LOC (sym)->allocreq++;
                }

              /* if it does not need or is spilt
                 or is already assigned to registers
                 or will not live beyond this instructions */
              if (!sym->nRegs ||
                  sym->isspilt ||
                  bitVectBitValue (_G.regAssigned, sym->key) ||
                  sym->liveTo <= ic->seq)
                continue;

              /* if some liverange has been spilt at the block level
                 and this one live beyond this block then spil this
                 to be safe */
              if (_G.blockSpil && sym->liveTo > ebbs[i]->lSeq)
                {
                  spillThis (sym);
                  continue;
                }
              /* if trying to allocate this will cause
                 a spill and there is nothing to spill
                 or this one is rematerializable then
                 spill this one */
              willCS = willCauseSpill (sym->nRegs, sym->regType);

              /* explicit turn off register spilling */
              willCS = 0;

              spillable = computeSpillable (ic);
              if (sym->remat ||
                  (willCS && bitVectIsZero (spillable)))
                {

                  spillThis (sym);
                  continue;

                }

              /* If the live range preceeds the point of definition
                 then ideally we must take into account registers that
                 have been allocated after sym->liveFrom but freed
                 before ic->seq. This is complicated, so spill this
                 symbol instead and let fillGaps handle the allocation. */
              if (sym->liveFrom < ic->seq)
                {
                    spillThis (sym);
                    continue;
                }

              /* if it has a spillocation & is used less than
                 all other live ranges then spill this */
                if (willCS) {
                    if (SYM_SPIL_LOC (sym)) {
                        symbol *leastUsed = leastUsedLR (liveRangesWith (spillable,
                                                                         allLRs, ebbs[i], ic));
                        if (leastUsed && leastUsed->used > sym->used) {
                            spillThis (sym);
                            continue;
                        }
                    } else {
                        /* if none of the liveRanges have a spillLocation then better
                           to spill this one than anything else already assigned to registers */
                        if (liveRangesWith(spillable,noSpilLoc,ebbs[i],ic)) {
                            /* if this is local to this block then we might find a block spil */
                            if (!(sym->liveFrom >= ebbs[i]->fSeq && sym->liveTo <= ebbs[i]->lSeq)) {
                                spillThis (sym);
                                continue;
                            }
                        }
                    }
                }

              if (ic->op == RECEIVE)
                debugLog ("When I get clever, I'll optimize the receive logic\n");

              if(POINTER_GET(ic) && IS_BITFIELD(getSpec(operandType(IC_RESULT(ic))))
                && (SPEC_BLEN(getSpec(operandType(IC_RESULT(ic))))==1)
                && (ic->next->op == IFX)
                && (OP_LIVETO(IC_RESULT(ic)) == ic->next->seq)) {

                /* skip register allocation since none will be used */
                for(j=0;j<sym->nRegs;j++)
                  sym->regs[j] = newReg(REG_TMP, PO_GPR_TEMP, 0, "bad", 1, 0, NULL);
//                OP_SYMBOL(IC_RESULT(ic))->nRegs = 0;

                continue;
              }

              /* if we need ptr regs for the right side
                 then mark it */
              if (POINTER_GET (ic) && IS_SYMOP( IC_LEFT(ic) ) && getSize (OP_SYMBOL (IC_LEFT (ic))->type)
                  <= (unsigned) NEARPTRSIZE)
                {
                  pic16_ptrRegReq++;
                  ptrRegSet = 1;
                }
              /* else we assign registers to it */
              _G.regAssigned = bitVectSetBit (_G.regAssigned, sym->key);

              if(debugF)
                bitVectDebugOn(_G.regAssigned, debugF);

              for (j = 0; j < sym->nRegs; j++)
                {
                  if (sym->regType == REG_PTR)
                    sym->regs[j] = getRegPtr (ic, ebbs[i], sym);
                  else
                    sym->regs[j] = getRegGpr (ic, ebbs[i], sym);

                  /* if the allocation falied which means
                     this was spilt then break */
                  if (!sym->regs[j])
                    break;
                }
              debugLog ("  %d - \n", __LINE__);

              /* if it shares registers with operands make sure
                 that they are in the same position */
              if (IC_LEFT (ic) && IS_SYMOP (IC_LEFT (ic)) &&
                  OP_SYMBOL (IC_LEFT (ic))->nRegs && ic->op != '=')
                positionRegs (OP_SYMBOL (IC_RESULT (ic)),
                              OP_SYMBOL (IC_LEFT (ic)), ic->lineno);
              /* do the same for the right operand */
              if (IC_RIGHT (ic) && IS_SYMOP (IC_RIGHT (ic)) &&
                  OP_SYMBOL (IC_RIGHT (ic))->nRegs && ic->op != '=')
                positionRegs (OP_SYMBOL (IC_RESULT (ic)),
                              OP_SYMBOL (IC_RIGHT (ic)), ic->lineno);

              debugLog ("  %d - \n", __LINE__);
              if (ptrRegSet)
                {
                  debugLog ("  %d - \n", __LINE__);
                  pic16_ptrRegReq--;
                  ptrRegSet = 0;
                }

            }
        }
    }

    /* Check for and fix any problems with uninitialized operands */
    for (i = 0; i < count; i++)
      {
        iCode *ic;

        if (ebbs[i]->noPath &&
            (ebbs[i]->entryLabel != entryLabel &&
             ebbs[i]->entryLabel != returnLabel))
            continue;

        for (ic = ebbs[i]->sch; ic; ic = ic->next)
          {
            if (SKIP_IC2 (ic))
              continue;

            if (ic->op == IFX)
              {
                verifyRegsAssigned (IC_COND (ic), ic);
                continue;
              }

            if (ic->op == JUMPTABLE)
              {
                verifyRegsAssigned (IC_JTCOND (ic), ic);
                continue;
              }

            verifyRegsAssigned (IC_RESULT (ic), ic);
            verifyRegsAssigned (IC_LEFT (ic), ic);
            verifyRegsAssigned (IC_RIGHT (ic), ic);
          }
      }

}

/*-----------------------------------------------------------------*/
/* rUmaskForOp :- returns register mask for an operand             */
/*-----------------------------------------------------------------*/
static bitVect *
rUmaskForOp (operand * op)
{
  bitVect *rumask;
  symbol *sym;
  int j;

  debugLog ("%s\n", __FUNCTION__);
  /* only temporaries are assigned registers */
  if (!IS_ITEMP (op))
    return NULL;

  sym = OP_SYMBOL (op);

  /* if spilt or no registers assigned to it
     then nothing */
  if (sym->isspilt || !sym->nRegs)
    return NULL;

  rumask = newBitVect (pic16_nRegs);

  for (j = 0; j < sym->nRegs; j++)
    {
      rumask = bitVectSetBit (rumask,
                              sym->regs[j]->rIdx);
    }

  return rumask;
}

/*-----------------------------------------------------------------*/
/* regsUsedIniCode :- returns bit vector of registers used in iCode */
/*-----------------------------------------------------------------*/
static bitVect *
regsUsedIniCode (iCode * ic)
{
  bitVect *rmask = newBitVect (pic16_nRegs);

  debugLog ("%s\n", __FUNCTION__);
  /* do the special cases first */
  if (ic->op == IFX)
    {
      rmask = bitVectUnion (rmask,
                            rUmaskForOp (IC_COND (ic)));
      goto ret;
    }

  /* for the jumptable */
  if (ic->op == JUMPTABLE)
    {
      rmask = bitVectUnion (rmask,
                            rUmaskForOp (IC_JTCOND (ic)));

      goto ret;
    }

  /* of all other cases */
  if (IC_LEFT (ic))
    rmask = bitVectUnion (rmask,
                          rUmaskForOp (IC_LEFT (ic)));


  if (IC_RIGHT (ic))
    rmask = bitVectUnion (rmask,
                          rUmaskForOp (IC_RIGHT (ic)));

  if (IC_RESULT (ic))
    rmask = bitVectUnion (rmask,
                          rUmaskForOp (IC_RESULT (ic)));

ret:
  return rmask;
}

/*-----------------------------------------------------------------*/
/* createRegMask - for each instruction will determine the regsUsed */
/*-----------------------------------------------------------------*/
static void
createRegMask (eBBlock ** ebbs, int count)
{
  int i;

  debugLog ("%s\n", __FUNCTION__);
  /* for all blocks */
  for (i = 0; i < count; i++)
    {
      iCode *ic;

      if (ebbs[i]->noPath &&
          (ebbs[i]->entryLabel != entryLabel &&
           ebbs[i]->entryLabel != returnLabel))
        continue;

      /* for all instructions */
      for (ic = ebbs[i]->sch; ic; ic = ic->next)
        {

          int j;

          if (SKIP_IC2 (ic) || !ic->rlive)
            continue;

          /* first mark the registers used in this
             instruction */
          ic->rUsed = regsUsedIniCode (ic);
          _G.funcrUsed = bitVectUnion (_G.funcrUsed, ic->rUsed);

          /* now create the register mask for those
             registers that are in use : this is a
             super set of ic->rUsed */
          ic->rMask = newBitVect (pic16_nRegs + 1);

          /* for all live Ranges alive at this point */
          for (j = 1; j < ic->rlive->size; j++)
            {
              symbol *sym;
              int k;

              /* if not alive then continue */
              if (!bitVectBitValue (ic->rlive, j))
                continue;

              /* find the live range we are interested in */
              if (!(sym = hTabItemWithKey (liveRanges, j)))
                {
                  werror (E_INTERNAL_ERROR, __FILE__, __LINE__,
                          "createRegMask cannot find live range");
                  exit (0);
                }

              /* if no register assigned to it */
              if (!sym->nRegs || sym->isspilt)
                continue;

              /* for all the registers allocated to it */
              for (k = 0; k < sym->nRegs; k++)
                if (sym->regs[k])
                  ic->rMask =
                    bitVectSetBit (ic->rMask, sym->regs[k]->rIdx);
            }
        }
    }
}

/*-----------------------------------------------------------------*/
/* rematStr - returns the rematerialized string for a remat var    */
/*-----------------------------------------------------------------*/
static symbol *
rematStr (symbol * sym)
{
  iCode *ic = sym->rematiCode;
  symbol *psym = NULL;
  int offset = 0;

  debugLog ("%s\n", __FUNCTION__);

  while (ic->op == '+' || ic->op == '-') {
    /* if plus or minus print the right hand side */

    offset += (int) operandLitValue (IC_RIGHT (ic));
    ic = OP_SYMBOL (IC_LEFT (ic))->rematiCode;
  } // while

  psym = newSymbol (OP_SYMBOL (IC_LEFT (ic))->rname, 1);
  psym->offset = offset;
  return psym;
}

#if 0
/*-----------------------------------------------------------------*/
/* rematStr - returns the rematerialized string for a remat var    */
/*-----------------------------------------------------------------*/
static char *
rematStr (symbol * sym)
{
  char *s = buffer;
  iCode *ic = sym->rematiCode;

  debugLog ("%s\n", __FUNCTION__);
  while (1)
    {

      printf ("%s\n", s);
      /* if plus or minus print the right hand side */
/*
   if (ic->op == '+' || ic->op == '-') {
   sprintf(s,"0x%04x %c ",(int) operandLitValue(IC_RIGHT(ic)),
   ic->op );
   s += strlen(s);
   ic = OP_SYMBOL(IC_LEFT(ic))->rematiCode;
   continue ;
   }
 */
      if (ic->op == '+' || ic->op == '-')
        {
          iCode *ric = OP_SYMBOL (IC_LEFT (ic))->rematiCode;
          sprintf (s, "(%s %c 0x%04x)",
                   OP_SYMBOL (IC_LEFT (ric))->rname,
                   ic->op,
                   (int) operandLitValue (IC_RIGHT (ic)));

          //s += strlen(s);
          //ic = OP_SYMBOL(IC_LEFT(ic))->rematiCode;
          //continue ;
          //fprintf(stderr, "ralloc.c:%d OOPS %s\n",__LINE__,s);
          return buffer;
        }

      /* we reached the end */
      sprintf (s, "%s", OP_SYMBOL (IC_LEFT (ic))->rname);
      break;
    }

  printf ("%s\n", buffer);
  return buffer;
}
#endif

/*-----------------------------------------------------------------*/
/* regTypeNum - computes the type & number of registers required   */
/*-----------------------------------------------------------------*/
static void
regTypeNum ()
{
  symbol *sym;
  int k;
  iCode *ic;

  debugLog ("%s\n", __FUNCTION__);
  /* for each live range do */
  for (sym = hTabFirstItem (liveRanges, &k); sym; sym = hTabNextItem (liveRanges, &k)) {

    debugLog ("  %d - %s\n", __LINE__, sym->rname);
    //fprintf(stderr,"  %d - %s\n", __LINE__, sym->rname);

    /* if used zero times then no registers needed */
    if ((sym->liveTo - sym->liveFrom) == 0)
      continue;

    /* if the live range is a temporary */
    if (sym->isitmp) {

      debugLog ("  %d - itemp register\n", __LINE__);

      /* if the type is marked as a conditional */
      if (sym->regType == REG_CND)
        continue;

      /* if used in return only then we don't
         need registers */
      if (sym->ruonly || sym->accuse) {
        if (IS_AGGREGATE (sym->type) || sym->isptr)
          sym->type = aggrToPtr (sym->type, FALSE);
        debugLog ("  %d - no reg needed - used as a return\n", __LINE__);
        continue;
      }

      /* if the symbol has only one definition &
         that definition is a get_pointer and the
         pointer we are getting is rematerializable and
         in "data" space */

      if (bitVectnBitsOn (sym->defs) == 1 &&
          (ic = hTabItemWithKey (iCodehTab,
                                 bitVectFirstBit (sym->defs))) &&
          POINTER_GET (ic) &&
          !IS_BITVAR (sym->etype) &&
          (aggrToPtrDclType (operandType (IC_LEFT (ic)), FALSE) == POINTER)) {

//        continue;       /* FIXME -- VR */
        if (ptrPseudoSymSafe (sym, ic)) {

          symbol *psym;

          debugLog ("  %d - \n", __LINE__);

          /* create a psuedo symbol & force a spil */
          //X symbol *psym = newSymbol (rematStr (OP_SYMBOL (IC_LEFT (ic))), 1);
          psym = rematStr (OP_SYMBOL (IC_LEFT (ic)));
          psym->type = sym->type;
          psym->etype = sym->etype;
          psym->psbase = ptrBaseRematSym (OP_SYMBOL (IC_LEFT (ic)));
          strcpy (psym->rname, psym->name);
          sym->isspilt = 1;
          SYM_SPIL_LOC (sym) = psym;
          continue;
        }

        /* if in data space or idata space then try to
           allocate pointer register */

      }

      /* if not then we require registers */
      sym->nRegs = ((IS_AGGREGATE (sym->type) || sym->isptr) ?
                    getSize (sym->type = aggrToPtr (sym->type, FALSE)) :
                    getSize (sym->type));


#if 0
    if(IS_PTR_CONST (sym->type)) {
#else
    if(IS_CODEPTR (sym->type)) {
#endif
      // what IS this ???? (HJD)
      debugLog ("  %d const pointer type requires %d registers, changing to 3\n",__LINE__,sym->nRegs); // patch 14
      sym->nRegs = 3; // patch 14
    }

      if (sym->nRegs > 4) {
        fprintf (stderr, "allocated more than 4 or 0 registers for type ");
        printTypeChain (sym->type, stderr);
        fprintf (stderr, "\n");
      }

      /* determine the type of register required */
      if (sym->nRegs == 1 &&
          IS_PTR (sym->type) &&
          sym->uptr)
        sym->regType = REG_PTR;
      else
        sym->regType = REG_GPR;


      debugLog ("  reg name %s,  reg type %s\n", sym->rname, debugLogRegType (sym->regType));

    }
    else
      /* for the first run we don't provide */
      /* registers for true symbols we will */
      /* see how things go                  */
      sym->nRegs = 0;

  }

}
static DEFSETFUNC (markRegFree)
{
  ((reg_info *)item)->isFree = 1;
//  ((regs *)item)->wasUsed = 0;

  return 0;
}

DEFSETFUNC (pic16_deallocReg)
{
  fprintf(stderr,"deallocting register %s\n",((reg_info *)item)->name);
  ((reg_info *)item)->isFree = 1;
  ((reg_info *)item)->wasUsed = 0;

  return 0;
}
/*-----------------------------------------------------------------*/
/* freeAllRegs - mark all registers as free                        */
/*-----------------------------------------------------------------*/
void
pic16_freeAllRegs ()
{
  debugLog ("%s\n", __FUNCTION__);

  applyToSet(pic16_dynAllocRegs,markRegFree);
  applyToSet(pic16_dynStackRegs,markRegFree);
}

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
void
pic16_deallocateAllRegs ()
{
  debugLog ("%s\n", __FUNCTION__);

  applyToSet(pic16_dynAllocRegs,pic16_deallocReg);
}


/*-----------------------------------------------------------------*/
/* deallocStackSpil - this will set the stack pointer back         */
/*-----------------------------------------------------------------*/
static
DEFSETFUNC (deallocStackSpil)
{
  symbol *sym = item;

  debugLog ("%s\n", __FUNCTION__);
  deallocLocal (sym);
  return 0;
}

/*-----------------------------------------------------------------*/
/* farSpacePackable - returns the packable icode for far variables */
/*-----------------------------------------------------------------*/
static iCode *
farSpacePackable (iCode * ic)
{
  iCode *dic;

  debugLog ("%s\n", __FUNCTION__);
  /* go thru till we find a definition for the
     symbol on the right */
  for (dic = ic->prev; dic; dic = dic->prev)
    {

      /* if the definition is a call then no */
      if ((dic->op == CALL || dic->op == PCALL) &&
          IC_RESULT (dic)->key == IC_RIGHT (ic)->key)
        {
          return NULL;
        }

      /* if shift by unknown amount then not */
      if ((dic->op == LEFT_OP || dic->op == RIGHT_OP) &&
          IC_RESULT (dic)->key == IC_RIGHT (ic)->key)
        return NULL;

      /* if pointer get and size > 1 */
      if (POINTER_GET (dic) &&
          getSize (aggrToPtr (operandType (IC_LEFT (dic)), FALSE)) > 1)
        return NULL;

      if (POINTER_SET (dic) &&
          getSize (aggrToPtr (operandType (IC_RESULT (dic)), FALSE)) > 1)
        return NULL;

      /* if any three is a true symbol in far space */
      if (IC_RESULT (dic) &&
          IS_TRUE_SYMOP (IC_RESULT (dic)) &&
          isOperandInFarSpace (IC_RESULT (dic)))
        return NULL;

      if (IC_RIGHT (dic) &&
          IS_TRUE_SYMOP (IC_RIGHT (dic)) &&
          isOperandInFarSpace (IC_RIGHT (dic)) &&
          !isOperandEqual (IC_RIGHT (dic), IC_RESULT (ic)))
        return NULL;

      if (IC_LEFT (dic) &&
          IS_TRUE_SYMOP (IC_LEFT (dic)) &&
          isOperandInFarSpace (IC_LEFT (dic)) &&
          !isOperandEqual (IC_LEFT (dic), IC_RESULT (ic)))
        return NULL;

      if (isOperandEqual (IC_RIGHT (ic), IC_RESULT (dic)))
        {
          if ((dic->op == LEFT_OP ||
               dic->op == RIGHT_OP ||
               dic->op == '-') &&
              IS_OP_LITERAL (IC_RIGHT (dic)))
            return NULL;
          else
            return dic;
        }
    }

  return NULL;
}

#if 0
static int packRegsForPointerGet(iCode *ic, eBBlock *ebp)
{
  iCode *dic, *sic;

    debugLog ("%d\t%s\n", __LINE__, __FUNCTION__);
    debugLog ("ic->op = %s\n", pic16_decodeOp( ic->op ) );
    debugAopGet ("  result:", IC_RESULT (ic));
    debugAopGet ("  left:", IC_LEFT (ic));
    debugAopGet ("  right:", IC_RIGHT (ic));

    dic = ic->prev;
    if((dic->op == '=')
      && (
}
#endif


void replaceOperandWithOperand(eBBlock *ebp, iCode *ic, operand *src, iCode *dic, operand *dst);

/*-----------------------------------------------------------------*/
/* packRegsForAssign - register reduction for assignment           */
/*-----------------------------------------------------------------*/
static int
packRegsForAssign (iCode * ic, eBBlock * ebp)
{
  iCode *dic, *sic;

  debugLog ("%d\t%s\n", __LINE__, __FUNCTION__);
  debugLog ("ic->op = %s\n", pic16_decodeOp( ic->op ) );
  debugAopGet ("  result:", IC_RESULT (ic));
  debugAopGet ("  left:", IC_LEFT (ic));
  debugAopGet ("  right:", IC_RIGHT (ic));

//      fprintf(stderr, "%s:%d symbol = %s\n", __FILE__, __LINE__, OP_SYMBOL( IC_RESULT(ic))->name);

        debugLog(" %d - actuall processing\n", __LINE__ );

  if (!IS_ITEMP (IC_RESULT (ic))) {
    pic16_allocDirReg(IC_RESULT (ic));
    debugLog ("  %d - result is not temp\n", __LINE__);
  }

//  if(IS_VALOP(IC_RIGHT(ic)))return 0;

/* See BUGLOG0001 - VR */
#if 1
  if (!IS_ITEMP (IC_RIGHT (ic)) /*&& (!IS_PARM(IC_RESULT(ic)))*/) {
    debugLog ("  %d - not packing - right is not temp\n", __LINE__);
    pic16_allocDirReg(IC_RIGHT (ic));
    return 0;
  }
#endif

  if (OP_SYMBOL (IC_RIGHT (ic))->isind ||
      OP_LIVETO (IC_RIGHT (ic)) > ic->seq)
    {
      debugLog ("  %d - not packing - right side fails \n", __LINE__);
      return 0;
    }

  /* if the true symbol is defined in far space or on stack
     then we should not since this will increase register pressure */
  if (isOperandInFarSpace (IC_RESULT (ic)))
    {
      if ((dic = farSpacePackable (ic)))
        goto pack;
      else
        return 0;

    }

  /* find the definition of iTempNN scanning backwards if we find a
     a use of the true symbol before we find the definition then
     we cannot pack */
  for (dic = ic->prev; dic; dic = dic->prev)
    {

      /* if there is a function call and this is
         a parameter & not my parameter then don't pack it */
      if ((dic->op == CALL || dic->op == PCALL) &&
          (OP_SYMBOL (IC_RESULT (ic))->_isparm &&
           !OP_SYMBOL (IC_RESULT (ic))->ismyparm))
        {
          debugLog ("  %d - \n", __LINE__);
          dic = NULL;
          break;
        }


      if (SKIP_IC2 (dic))
        continue;

        debugLog("%d\tSearching for iTempNN\n", __LINE__);

      if (IS_TRUE_SYMOP (IC_RESULT (dic)) &&
          IS_OP_VOLATILE (IC_RESULT (dic)))
        {
          debugLog ("  %d - dic is VOLATILE \n", __LINE__);
          dic = NULL;
          break;
        }

#if 1
      if( IS_SYMOP( IC_RESULT(dic)) &&
        IS_BITFIELD( OP_SYMBOL(IC_RESULT(dic))->etype ) ) {

          debugLog (" %d - result is bitfield\n", __LINE__);
          dic = NULL;
          break;
        }
#endif

      if (IS_SYMOP (IC_RESULT (dic)) &&
          IC_RESULT (dic)->key == IC_RIGHT (ic)->key)
        {
          /* A previous result was assigned to the same register - we'll our definition */
          debugLog ("  %d - dic result key == ic right key -- pointer set=%c\n",
                    __LINE__, ((POINTER_SET (dic)) ? 'Y' : 'N'));
          if (POINTER_SET (dic))
            dic = NULL;

          break;
        }

      if (IS_SYMOP (IC_RIGHT (dic)) &&
          (IC_RIGHT (dic)->key == IC_RESULT (ic)->key ||
           IC_RIGHT (dic)->key == IC_RIGHT (ic)->key))
        {
          debugLog ("  %d - dic right key == ic rightor result key\n", __LINE__);
          dic = NULL;
          break;
        }

      if (IS_SYMOP (IC_LEFT (dic)) &&
          (IC_LEFT (dic)->key == IC_RESULT (ic)->key ||
           IC_LEFT (dic)->key == IC_RIGHT (ic)->key))
        {
          debugLog ("  %d - dic left key == ic rightor result key\n", __LINE__);
          dic = NULL;
          break;
        }

      if (POINTER_SET (dic) &&
          IC_RESULT (dic)->key == IC_RESULT (ic)->key)
        {
          debugLog ("  %d - dic result key == ic result key -- pointer set=Y\n",
                    __LINE__);
          dic = NULL;
          break;
        }
    }

  if (!dic)
    return 0;                   /* did not find */

#if 1
        /* This code is taken from the hc08 port. Do not know
         * if it fits for pic16, but I leave it here just in case */

        /* if assignment then check that right is not a bit */
        if (ASSIGNMENT (ic) && !POINTER_SET (ic)) {
          sym_link *etype = operandType (IC_RESULT (dic));

                if (IS_BITFIELD (etype)) {
                        /* if result is a bit too then it's ok */
                        etype = operandType (IC_RESULT (ic));
                        if (!IS_BITFIELD (etype)) {
                                debugLog(" %d bitfields\n");
                          return 0;
                        }
                }
        }
#endif

  /* if the result is on stack or iaccess then it must be
     the same atleast one of the operands */
  if (OP_SYMBOL (IC_RESULT (ic))->onStack ||
      OP_SYMBOL (IC_RESULT (ic))->iaccess)
    {
      /* the operation has only one symbol
         operator then we can pack */
      if ((IC_LEFT (dic) && !IS_SYMOP (IC_LEFT (dic))) ||
          (IC_RIGHT (dic) && !IS_SYMOP (IC_RIGHT (dic))))
        goto pack;

      if (!((IC_LEFT (dic) &&
             IC_RESULT (ic)->key == IC_LEFT (dic)->key) ||
            (IC_RIGHT (dic) &&
             IC_RESULT (ic)->key == IC_RIGHT (dic)->key)))
        return 0;
    }
pack:
  debugLog ("  packing. removing %s\n", OP_SYMBOL (IC_RIGHT (ic))->rname);
  debugLog ("  replacing with %s\n", OP_SYMBOL (IC_RESULT (dic))->rname);
  /* found the definition */

  /* delete from liverange table also
     delete from all the points inbetween and the new
     one */
    for (sic = dic; sic != ic; sic = sic->next)
      {
        bitVectUnSetBit (sic->rlive, IC_RESULT (ic)->key);
        if (IS_ITEMP (IC_RESULT (dic)))
          bitVectSetBit (sic->rlive, IC_RESULT (dic)->key);
      }

  /* replace the result with the result of */
  /* this assignment and remove this assignment */


    bitVectUnSetBit(OP_SYMBOL(IC_RESULT(dic))->defs,dic->key);
    IC_RESULT (dic) = IC_RESULT (ic);

    if (IS_ITEMP (IC_RESULT (dic)) && OP_SYMBOL (IC_RESULT (dic))->liveFrom > dic->seq)
      {
        OP_SYMBOL (IC_RESULT (dic))->liveFrom = dic->seq;
      }

    remiCodeFromeBBlock (ebp, ic);
    bitVectUnSetBit(OP_SYMBOL(IC_RESULT(ic))->defs,ic->key);

    debugLog("  %d\n", __LINE__ );
    hTabDeleteItem (&iCodehTab, ic->key, ic, DELETE_ITEM, NULL);
    OP_DEFS (IC_RESULT (dic)) = bitVectSetBit (OP_DEFS (IC_RESULT (dic)), dic->key);
    return 1;
}


#if 1

#define NO_packRegsForAccUse
#define NO_packRegsForSupport
#define NO_packRegsForOneuse
#define NO_cast_peep

#endif


#ifndef NO_packRegsForSupport
/*-----------------------------------------------------------------*/
/* findAssignToSym : scanning backwards looks for first assig found */
/*-----------------------------------------------------------------*/
static iCode *
findAssignToSym (operand * op, iCode * ic)
{
  iCode *dic;

  debugLog ("%s\n", __FUNCTION__);
  for (dic = ic->prev; dic; dic = dic->prev)
    {

      /* if definition by assignment */
      if (dic->op == '=' &&
          !POINTER_SET (dic) &&
          IC_RESULT (dic)->key == op->key
/*          &&  IS_TRUE_SYMOP(IC_RIGHT(dic)) */
        )
        {

          /* we are interested only if defined in far space */
          /* or in stack space in case of + & - */

          /* if assigned to a non-symbol then return
             true */
          if (!IS_SYMOP (IC_RIGHT (dic)))
            break;

          /* if the symbol is in far space then
             we should not */
          if (isOperandInFarSpace (IC_RIGHT (dic)))
            return NULL;

          /* for + & - operations make sure that
             if it is on the stack it is the same
             as one of the three operands */
          if ((ic->op == '+' || ic->op == '-') &&
              OP_SYMBOL (IC_RIGHT (dic))->onStack)
            {
              if (IC_RESULT (ic)->key != IC_RIGHT (dic)->key &&
                  IC_LEFT (ic)->key != IC_RIGHT (dic)->key &&
                  IC_RIGHT (ic)->key != IC_RIGHT (dic)->key)
                return NULL;
            }

          break;

        }

      /* if we find an usage then we cannot delete it */
      if (IC_LEFT (dic) && IC_LEFT (dic)->key == op->key)
        return NULL;

      if (IC_RIGHT (dic) && IC_RIGHT (dic)->key == op->key)
        return NULL;

      if (POINTER_SET (dic) && IC_RESULT (dic)->key == op->key)
        return NULL;
    }

  /* now make sure that the right side of dic
     is not defined between ic & dic */
  if (dic)
    {
      iCode *sic = dic->next;

      for (; sic != ic; sic = sic->next)
        if (IC_RESULT (sic) &&
            IC_RESULT (sic)->key == IC_RIGHT (dic)->key)
          return NULL;
    }

  return dic;


}
#endif


#ifndef NO_packRegsForSupport
/*-----------------------------------------------------------------*/
/* packRegsForSupport :- reduce some registers for support calls   */
/*-----------------------------------------------------------------*/
static int
packRegsForSupport (iCode * ic, eBBlock * ebp)
{
  int change = 0;

  debugLog ("%s\n", __FUNCTION__);
  /* for the left & right operand :- look to see if the
     left was assigned a true symbol in far space in that
     case replace them */
  if (IS_ITEMP (IC_LEFT (ic)) &&
      OP_SYMBOL (IC_LEFT (ic))->liveTo <= ic->seq)
    {
      iCode *dic = findAssignToSym (IC_LEFT (ic), ic);
      iCode *sic;

      if (!dic)
        goto right;

      debugAopGet ("removing left:", IC_LEFT (ic));

      /* found it we need to remove it from the
         block */
      for (sic = dic; sic != ic; sic = sic->next)
        bitVectUnSetBit (sic->rlive, IC_LEFT (ic)->key);

      IC_LEFT (ic)->operand.symOperand =
        IC_RIGHT (dic)->operand.symOperand;
      IC_LEFT (ic)->key = IC_RIGHT (dic)->operand.symOperand->key;
      remiCodeFromeBBlock (ebp, dic);
      bitVectUnSetBit(OP_SYMBOL(IC_RESULT(dic))->defs,dic->key);
      hTabDeleteItem (&iCodehTab, dic->key, dic, DELETE_ITEM, NULL);
      change++;
    }

  /* do the same for the right operand */
right:
  if (!change &&
      IS_ITEMP (IC_RIGHT (ic)) &&
      OP_SYMBOL (IC_RIGHT (ic))->liveTo <= ic->seq)
    {
      iCode *dic = findAssignToSym (IC_RIGHT (ic), ic);
      iCode *sic;

      if (!dic)
        return change;

      /* if this is a subtraction & the result
         is a true symbol in far space then don't pack */
      if (ic->op == '-' && IS_TRUE_SYMOP (IC_RESULT (dic)))
        {
          sym_link *etype = getSpec (operandType (IC_RESULT (dic)));
          if (IN_FARSPACE (SPEC_OCLS (etype)))
            return change;
        }

      debugAopGet ("removing right:", IC_RIGHT (ic));

      /* found it we need to remove it from the
         block */
      for (sic = dic; sic != ic; sic = sic->next)
        bitVectUnSetBit (sic->rlive, IC_RIGHT (ic)->key);

      IC_RIGHT (ic)->operand.symOperand =
        IC_RIGHT (dic)->operand.symOperand;
      IC_RIGHT (ic)->key = IC_RIGHT (dic)->operand.symOperand->key;

      remiCodeFromeBBlock (ebp, dic);
      bitVectUnSetBit(OP_SYMBOL(IC_RESULT(dic))->defs,dic->key);
      hTabDeleteItem (&iCodehTab, dic->key, dic, DELETE_ITEM, NULL);
      change++;
    }

  return change;
}
#endif


#ifndef NO_packRegsForOneuse
/*-----------------------------------------------------------------*/
/* packRegsForOneuse : - will reduce some registers for single Use */
/*-----------------------------------------------------------------*/
static iCode *
packRegsForOneuse (iCode * ic, operand * op, eBBlock * ebp)
{
  bitVect *uses;
  iCode *dic, *sic;

  return NULL;

  debugLog ("%s\n", __FUNCTION__);
  /* if returning a literal then do nothing */
  if (!IS_SYMOP (op))
    return NULL;

  if(OP_SYMBOL(op)->remat || OP_SYMBOL(op)->ruonly)
    return NULL;

  /* only upto 2 bytes since we cannot predict
     the usage of b, & acc */
  if (getSize (operandType (op)) > (pic16_fReturnSizePic - 1)
      && ic->op != RETURN
      && ic->op != SEND
      && !POINTER_SET(ic)
      && !POINTER_GET(ic)
      )
    return NULL;

  /* this routine will mark the a symbol as used in one
     instruction use only && if the definition is local
     (ie. within the basic block) && has only one definition &&
     that definition is either a return value from a
     function or does not contain any variables in
     far space */

#if 0
  uses = bitVectCopy (OP_USES (op));
  bitVectUnSetBit (uses, ic->key);      /* take away this iCode */
  if (!bitVectIsZero (uses))    /* has other uses */
    return NULL;
#endif

#if 1
  if (bitVectnBitsOn (OP_USES (op)) > 1)
    return NULL;
#endif

  /* if it has only one defintion */
  if (bitVectnBitsOn (OP_DEFS (op)) > 1)
    return NULL;                /* has more than one definition */

  /* get that definition */
  if (!(dic =
        hTabItemWithKey (iCodehTab,
                         bitVectFirstBit (OP_DEFS (op)))))
    return NULL;

  /* found the definition now check if it is local */
  if (dic->seq < ebp->fSeq ||
      dic->seq > ebp->lSeq)
    return NULL;                /* non-local */

  /* now check if it is the return from
     a function call */
  if (dic->op == CALL || dic->op == PCALL)
    {
      if (ic->op != SEND && ic->op != RETURN &&
          !POINTER_SET(ic) && !POINTER_GET(ic))
        {
          OP_SYMBOL (op)->ruonly = 1;
          return dic;
        }
      dic = dic->next;
    }
  else
    {


  /* otherwise check that the definition does
     not contain any symbols in far space */
  if (isOperandInFarSpace (IC_LEFT (dic)) ||
      isOperandInFarSpace (IC_RIGHT (dic)) ||
      IS_OP_RUONLY (IC_LEFT (ic)) ||
      IS_OP_RUONLY (IC_RIGHT (ic)))
    {
      return NULL;
    }

  /* if pointer set then make sure the pointer
     is one byte */
  if (POINTER_SET (dic) &&
      !IS_DATA_PTR (aggrToPtr (operandType (IC_RESULT (dic)), FALSE)))
    return NULL;

  if (POINTER_GET (dic) &&
      !IS_DATA_PTR (aggrToPtr (operandType (IC_LEFT (dic)), FALSE)))
    return NULL;
    }

  sic = dic;

  /* also make sure the intervenening instructions
     don't have any thing in far space */
  for (dic = dic->next; dic && dic != ic; dic = dic->next)
    {

      /* if there is an intervening function call then no */
      if (dic->op == CALL || dic->op == PCALL)
        return NULL;
      /* if pointer set then make sure the pointer
         is one byte */
      if (POINTER_SET (dic) &&
          !IS_DATA_PTR (aggrToPtr (operandType (IC_RESULT (dic)), FALSE)))
        return NULL;

      if (POINTER_GET (dic) &&
          !IS_DATA_PTR (aggrToPtr (operandType (IC_LEFT (dic)), FALSE)))
        return NULL;

      /* if address of & the result is remat then okay */
      if (dic->op == ADDRESS_OF &&
          OP_SYMBOL (IC_RESULT (dic))->remat)
        continue;

      /* if operand has size of three or more & this
         operation is a '*','/' or '%' then 'b' may
         cause a problem */
      if ((dic->op == '%' || dic->op == '/' || dic->op == '*') &&
          getSize (operandType (op)) >= 2)
        return NULL;

      /* if left or right or result is in far space */
      if (isOperandInFarSpace (IC_LEFT (dic)) ||
          isOperandInFarSpace (IC_RIGHT (dic)) ||
          isOperandInFarSpace (IC_RESULT (dic)) ||
          IS_OP_RUONLY (IC_LEFT (dic)) ||
          IS_OP_RUONLY (IC_RIGHT (dic)) ||
          IS_OP_RUONLY (IC_RESULT (dic)))
        {
          return NULL;
        }
    }

  OP_SYMBOL (op)->ruonly = 1;
  return sic;

}
#endif


/*-----------------------------------------------------------------*/
/* isBitwiseOptimizable - requirements of JEAN LOUIS VERN          */
/*-----------------------------------------------------------------*/
static bool
isBitwiseOptimizable (iCode * ic)
{
  sym_link *ltype = getSpec (operandType (IC_LEFT (ic)));
  sym_link *rtype = getSpec (operandType (IC_RIGHT (ic)));

  debugLog ("%s\n", __FUNCTION__);
  /* bitwise operations are considered optimizable
     under the following conditions (Jean-Louis VERN)

     x & lit
     bit & bit
     bit & x
     bit ^ bit
     bit ^ x
     x   ^ lit
     x   | lit
     bit | bit
     bit | x
   */
  if (IS_LITERAL (rtype) ||
      (IS_BITVAR (ltype) && IN_BITSPACE (SPEC_OCLS (ltype))))
    return TRUE;
  else
    return FALSE;
}


#ifndef NO_packRegsForAccUse

/*-----------------------------------------------------------------*/
/* packRegsForAccUse - pack registers for acc use                  */
/*-----------------------------------------------------------------*/
static void
packRegsForAccUse (iCode * ic)
{
  iCode *uic;

  debugLog ("%s\n", __FUNCTION__);

  /* if this is an aggregate, e.g. a one byte char array */
  if (IS_AGGREGATE(operandType(IC_RESULT(ic)))) {
    return;
  }
  debugLog ("  %s:%d\n", __FUNCTION__,__LINE__);

  /* if + or - then it has to be one byte result */
  if ((ic->op == '+' || ic->op == '-')
      && getSize (operandType (IC_RESULT (ic))) > 1)
    return;

  debugLog ("  %s:%d\n", __FUNCTION__,__LINE__);
  /* if shift operation make sure right side is not a literal */
  if (ic->op == RIGHT_OP &&
      (isOperandLiteral (IC_RIGHT (ic)) ||
       getSize (operandType (IC_RESULT (ic))) > 1))
    return;

  debugLog ("  %s:%d\n", __FUNCTION__,__LINE__);
  if (ic->op == LEFT_OP &&
      (isOperandLiteral (IC_RIGHT (ic)) ||
       getSize (operandType (IC_RESULT (ic))) > 1))
    return;

  debugLog ("  %s:%d\n", __FUNCTION__,__LINE__);
  if (IS_BITWISE_OP (ic) &&
      getSize (operandType (IC_RESULT (ic))) > 1)
    return;


  debugLog ("  %s:%d\n", __FUNCTION__,__LINE__);
  /* has only one definition */
  if (bitVectnBitsOn (OP_DEFS (IC_RESULT (ic))) > 1)
    return;

  debugLog ("  %s:%d\n", __FUNCTION__,__LINE__);
  /* has only one use */
  if (bitVectnBitsOn (OP_USES (IC_RESULT (ic))) > 1)
    return;

  debugLog ("  %s:%d\n", __FUNCTION__,__LINE__);
  /* and the usage immediately follows this iCode */
  if (!(uic = hTabItemWithKey (iCodehTab,
                               bitVectFirstBit (OP_USES (IC_RESULT (ic))))))
    return;

  debugLog ("  %s:%d\n", __FUNCTION__,__LINE__);
  if (ic->next != uic)
    return;

  /* if it is a conditional branch then we definitely can */
  if (uic->op == IFX)
    goto accuse;

  if (uic->op == JUMPTABLE)
    return;

  /* if the usage is not is an assignment
     or an arithmetic / bitwise / shift operation then not */
  if (POINTER_SET (uic) &&
      getSize (aggrToPtr (operandType (IC_RESULT (uic)), FALSE)) > 1)
    return;

  debugLog ("  %s:%d\n", __FUNCTION__,__LINE__);
  if (uic->op != '=' &&
      !IS_ARITHMETIC_OP (uic) &&
      !IS_BITWISE_OP (uic) &&
      uic->op != LEFT_OP &&
      uic->op != RIGHT_OP)
    return;

  debugLog ("  %s:%d\n", __FUNCTION__,__LINE__);
  /* if used in ^ operation then make sure right is not a
     literl */
  if (uic->op == '^' && isOperandLiteral (IC_RIGHT (uic)))
    return;

  /* if shift operation make sure right side is not a literal */
  if (uic->op == RIGHT_OP &&
      (isOperandLiteral (IC_RIGHT (uic)) ||
       getSize (operandType (IC_RESULT (uic))) > 1))
    return;

  if (uic->op == LEFT_OP &&
      (isOperandLiteral (IC_RIGHT (uic)) ||
       getSize (operandType (IC_RESULT (uic))) > 1))
    return;

  /* make sure that the result of this icode is not on the
     stack, since acc is used to compute stack offset */
  if (IS_TRUE_SYMOP (IC_RESULT (uic)) &&
      OP_SYMBOL (IC_RESULT (uic))->onStack)
    return;

  /* if either one of them in far space then we cannot */
  if ((IS_TRUE_SYMOP (IC_LEFT (uic)) &&
       isOperandInFarSpace (IC_LEFT (uic))) ||
      (IS_TRUE_SYMOP (IC_RIGHT (uic)) &&
       isOperandInFarSpace (IC_RIGHT (uic))))
    return;

  /* if the usage has only one operand then we can */
  if (IC_LEFT (uic) == NULL ||
      IC_RIGHT (uic) == NULL)
    goto accuse;

  /* make sure this is on the left side if not
     a '+' since '+' is commutative */
  if (ic->op != '+' &&
      IC_LEFT (uic)->key != IC_RESULT (ic)->key)
    return;

#if 1
  debugLog ("  %s:%d\n", __FUNCTION__,__LINE__);
  /* if one of them is a literal then we can */
  if ( ((IC_LEFT (uic) && IS_OP_LITERAL (IC_LEFT (uic))) ||
        (IC_RIGHT (uic) && IS_OP_LITERAL (IC_RIGHT (uic))))  &&
       (getSize (operandType (IC_RESULT (uic))) <= 1))
    {
      OP_SYMBOL (IC_RESULT (ic))->accuse = 1;
      return;
    }
#endif

  debugLog ("  %s:%d\n", __FUNCTION__,__LINE__);
  /* if the other one is not on stack then we can */
  if (IC_LEFT (uic)->key == IC_RESULT (ic)->key &&
      (IS_ITEMP (IC_RIGHT (uic)) ||
       (IS_TRUE_SYMOP (IC_RIGHT (uic)) &&
        !OP_SYMBOL (IC_RIGHT (uic))->onStack)))
    goto accuse;

  if (IC_RIGHT (uic)->key == IC_RESULT (ic)->key &&
      (IS_ITEMP (IC_LEFT (uic)) ||
       (IS_TRUE_SYMOP (IC_LEFT (uic)) &&
        !OP_SYMBOL (IC_LEFT (uic))->onStack)))
    goto accuse;

  return;

accuse:
  debugLog ("%s - Yes we are using the accumulator\n", __FUNCTION__);
  OP_SYMBOL (IC_RESULT (ic))->accuse = 1;


}
#endif


/*-----------------------------------------------------------------*/
/* packForPush - hueristics to reduce iCode for pushing            */
/*-----------------------------------------------------------------*/
static void
packForReceive (iCode * ic, eBBlock * ebp)
{
  iCode *dic;

  debugLog ("%s\n", __FUNCTION__);
  debugAopGet ("  result:", IC_RESULT (ic));
  debugAopGet ("  left:", IC_LEFT (ic));
  debugAopGet ("  right:", IC_RIGHT (ic));

  if (!ic->next)
    return;

  for (dic = ic->next; dic; dic = dic->next)
    {
      if (IC_LEFT (dic) && (IC_RESULT (ic)->key == IC_LEFT (dic)->key))
        debugLog ("    used on left\n");
      if (IC_RIGHT (dic) && IC_RESULT (ic)->key == IC_RIGHT (dic)->key)
        debugLog ("    used on right\n");
      if (IC_RESULT (dic) && IC_RESULT (ic)->key == IC_RESULT (dic)->key)
        debugLog ("    used on result\n");

      if ((IC_LEFT (dic) && (IC_RESULT (ic)->key == IC_LEFT (dic)->key)) ||
        (IC_RESULT (dic) && IC_RESULT (ic)->key == IC_RESULT (dic)->key))
        return;
    }

  debugLog ("  hey we can remove this unnecessary assign\n");
}
/*-----------------------------------------------------------------*/
/* packForPush - hueristics to reduce iCode for pushing            */
/*-----------------------------------------------------------------*/
static void
packForPush (iCode * ic, eBBlock * ebp)
{
  iCode *dic, *lic;
  const char *iLine;
  bitVect *dbv;
  int disallowHiddenAssignment = 0;

  debugLog ("%s\n", __FUNCTION__);
  if (ic->op != IPUSH || !IS_ITEMP (IC_LEFT (ic)))
    return;

#if 0
  {
    int n1, n2;

    n1 = bitVectnBitsOn( OP_DEFS(IC_LEFT(ic)));
    n2 = bitVectnBitsOn( OP_USES(IC_LEFT(ic)));
    iLine = printILine(ic);
    debugf3("defs: %d\tuses: %d\t%s\n", n1, n2, printILine(ic));
    dbuf_free(iLine);
    debugf2("IC_LEFT(ic): from %d to %d\n", OP_LIVEFROM(IC_LEFT(ic)), OP_LIVETO(IC_LEFT(ic)));
  }
#endif

  /* must have only definition & one usage */
  if (bitVectnBitsOn (OP_DEFS (IC_LEFT (ic))) != 1 ||
      bitVectnBitsOn (OP_USES (IC_LEFT (ic))) != 1)
    return;

  /* find the definition */
  if (!(dic = hTabItemWithKey (iCodehTab,
                               bitVectFirstBit (OP_DEFS (IC_LEFT (ic))))))
    return;

  /* if definition is not assignment,
   * or is not pointer (because pointer might have changed) */
  if (dic->op != '=' || POINTER_SET (dic))
    return;

  /* we must ensure that we can use the delete the assignment,
   * because the source might have been modified in between.
   * Until I know how to fix this, I'll use the adhoc fix
   * to check the liveranges */
  if((OP_LIVEFROM(IC_RIGHT(dic))==0) || (OP_LIVETO(IC_RIGHT(dic))==0))
    return;
//  debugf2("IC_RIGHT(dic): from %d to %d\n", OP_LIVEFROM(IC_RIGHT(dic)), OP_LIVETO(IC_RIGHT(dic)));
    
  /* If the defining iCode is outside of this block, we need to recompute */
  /* ebp (see the mcs51 version of packForPush), but we weren't passed    */
  /* enough data to do that. Just bail out instead if that happens. */
  if (dic->seq < ebp->fSeq)
    return;

  if (IS_SYMOP (IC_RIGHT (dic)))
    {
      if (IC_RIGHT (dic)->isvolatile)
        return;

      if (OP_SYMBOL (IC_RIGHT (dic))->addrtaken || isOperandGlobal (IC_RIGHT (dic)))
        disallowHiddenAssignment = 1;

      /* make sure the right side does not have any definitions
         inbetween */
      dbv = OP_DEFS (IC_RIGHT (dic));
      for (lic = ic; lic && lic != dic; lic = lic->prev)
        {
          if (bitVectBitValue (dbv, lic->key))
            return;
          if (disallowHiddenAssignment && (lic->op == CALL || lic->op == PCALL || POINTER_SET (lic)))
            return;
        }
      /* make sure they have the same type */
      if (IS_SPEC (operandType (IC_LEFT (ic))))
        {
          sym_link *itype = operandType (IC_LEFT (ic));
          sym_link *ditype = operandType (IC_RIGHT (dic));

          if (SPEC_USIGN (itype) != SPEC_USIGN (ditype) || SPEC_LONG (itype) != SPEC_LONG (ditype))
            return;
        }
    }


  /*
   * The following code causes segfaults, e.g.,
   *   #2496919 Internal error with pic16 sdcc
   * and is thus disabled for now.
   */
  if (0)
    {
      /* we now we know that it has one & only one def & use
         and the that the definition is an assignment */

      /* extend the live range of replaced operand if needed */
      if (IS_SYMOP (IC_RIGHT (dic)) && OP_SYMBOL (IC_RIGHT (dic))->liveTo < ic->seq)
        {
          OP_SYMBOL (IC_RIGHT (dic))->liveTo = ic->seq;
        }
      bitVectUnSetBit (OP_SYMBOL (IC_RESULT (dic))->defs, dic->key);
      if (IS_ITEMP (IC_RIGHT (dic)))
        OP_USES (IC_RIGHT (dic)) = bitVectSetBit (OP_USES (IC_RIGHT (dic)), ic->key);

      IC_LEFT (ic) = IC_RIGHT (dic);

      iLine = printILine(dic);
      debugf("remiCodeFromeBBlock: %s\n", iLine);
      dbuf_free(iLine);

      remiCodeFromeBBlock (ebp, dic);
      bitVectUnSetBit(OP_SYMBOL(IC_RESULT(dic))->defs,dic->key);
      hTabDeleteItem (&iCodehTab, dic->key, dic, DELETE_ITEM, NULL);
    } // if
}

static void printSymType(char * str, sym_link *sl)
{
        if(!pic16_ralloc_debug)return;

        debugLog ("    %s Symbol type: ",str);
        printTypeChain (sl, debugF);
        debugLog ("\n");
}

/*-----------------------------------------------------------------*/
/* some debug code to print the symbol S_TYPE. Note that
 * the function checkSClass in src/SDCCsymt.c dinks with
 * the S_TYPE in ways the PIC port doesn't fully like...*/
/*-----------------------------------------------------------------*/
static void isData(sym_link *sl)
{
  FILE *of = stderr;

    if(!pic16_ralloc_debug)return;

    if(!sl)return;

    if(debugF)
      of = debugF;

    for ( ; sl; sl=sl->next) {
      if(!IS_DECL(sl) ) {
        switch (SPEC_SCLS(sl)) {
          case S_DATA: fprintf (of, "data "); break;
          case S_XDATA: fprintf (of, "xdata "); break;
          case S_SFR: fprintf (of, "sfr "); break;
          case S_SBIT: fprintf (of, "sbit "); break;
          case S_CODE: fprintf (of, "code "); break;
          case S_IDATA: fprintf (of, "idata "); break;
          case S_PDATA: fprintf (of, "pdata "); break;
          case S_LITERAL: fprintf (of, "literal "); break;
          case S_STACK: fprintf (of, "stack "); break;
          case S_XSTACK: fprintf (of, "xstack "); break;
          case S_BIT: fprintf (of, "bit "); break;
          case S_EEPROM: fprintf (of, "eeprom "); break;
          default: break;
        }
      }
    }
}


/*--------------------------------------------------------------------*/
/* pic16_packRegisters - does some transformations to reduce          */
/*                   register pressure                                */
/*                                                                    */
/*--------------------------------------------------------------------*/
static void
pic16_packRegisters (eBBlock * ebp)
{
  iCode *ic;
  int change = 0;

  debugLog ("%s\n", __FUNCTION__);

  while (1) {

    change = 0;

    /* look for assignments of the form */
    /* iTempNN = TRueSym (someoperation) SomeOperand */
    /*       ....                       */
    /* TrueSym := iTempNN:1             */
    for (ic = ebp->sch; ic; ic = ic->next)
      {
//              debugLog("%d\n", __LINE__);
        /* find assignment of the form TrueSym := iTempNN:1 */
        if ( (ic->op == '=') && !POINTER_SET (ic) ) // patch 11
          change += packRegsForAssign (ic, ebp);
        /* debug stuff */
        if (ic->op == '=')
          {
            if (POINTER_SET (ic))
              debugLog ("pointer is set\n");
            debugAopGet ("  result:", IC_RESULT (ic));
            debugAopGet ("  left:", IC_LEFT (ic));
            debugAopGet ("  right:", IC_RIGHT (ic));
          }

      }

    if (!change)
      break;
  }

  for (ic = ebp->sch; ic; ic = ic->next) {

    if(IS_SYMOP ( IC_LEFT(ic))) {
      sym_link *etype = getSpec (operandType (IC_LEFT (ic)));

      debugAopGet ("x  left:", IC_LEFT (ic));
#if 0
      if(IS_PTR_CONST(OP_SYMBOL(IC_LEFT(ic))->type))
#else
      if(IS_CODEPTR(OP_SYMBOL(IC_LEFT(ic))->type))
#endif
        debugLog ("    is a pointer\n");

      if(IS_PTR(OP_SYMBOL(IC_LEFT(ic))->type))
        debugLog ("    is a ptr\n");

      if(IS_OP_VOLATILE(IC_LEFT(ic)))
        debugLog ("    is volatile\n");

      isData(etype);

        if(IS_OP_VOLATILE(IC_LEFT(ic))) {
            debugLog ("  %d - left is not temp, allocating\n", __LINE__);
            pic16_allocDirReg(IC_LEFT (ic));
        }

      printSymType("c  ", OP_SYMBOL(IC_LEFT(ic))->type);
    }

    if(IS_SYMOP ( IC_RIGHT(ic))) {
      debugAopGet ("  right:", IC_RIGHT (ic));
      printSymType("    ", OP_SYMBOL(IC_RIGHT(ic))->type);
    }

    if(IS_SYMOP ( IC_RESULT(ic))) {
      debugAopGet ("  result:", IC_RESULT (ic));
      printSymType("     ", OP_SYMBOL(IC_RESULT(ic))->type);
    }

    if(IS_TRUE_SYMOP ( IC_RIGHT(ic))) {
      debugAopGet ("  right:", IC_RIGHT (ic));
      printSymType("    ", OP_SYMBOL(IC_RIGHT(ic))->type);
//      pic16_allocDirReg(IC_RIGHT(ic));
    }

    if(IS_TRUE_SYMOP ( IC_RESULT(ic))) {
      debugAopGet ("  result:", IC_RESULT (ic));
      printSymType("     ", OP_SYMBOL(IC_RESULT(ic))->type);
//      pic16_allocDirReg(IC_RESULT(ic));
    }


    if (POINTER_SET (ic))
      debugLog ("  %d - Pointer set\n", __LINE__);

      /* Look for two subsequent iCodes with */
      /*   iTemp := _c;         */
      /*   _c = iTemp & op;     */
      /* and replace them by    */
      /*   iTemp := _c;         */
      /*   _c = _c & op;        */
      if ((ic->op == BITWISEAND || ic->op == '|' || ic->op == '^')
        && ic->prev
        && ic->prev->op == '='
        && IS_ITEMP (IC_LEFT (ic))
        && IC_LEFT (ic) == IC_RESULT (ic->prev)
        && isOperandEqual (IC_RESULT(ic), IC_RIGHT(ic->prev)))
        {
          iCode* ic_prev = ic->prev;
          symbol* prev_result_sym = OP_SYMBOL (IC_RESULT (ic_prev));

          ReplaceOpWithCheaperOp (&IC_LEFT (ic), IC_RESULT (ic));
          if (IC_RESULT (ic_prev) != IC_RIGHT (ic)) {
            bitVectUnSetBit (OP_USES (IC_RESULT (ic_prev)), ic->key);
            if (/*IS_ITEMP (IC_RESULT (ic_prev)) && */
              prev_result_sym->liveTo == ic->seq)
            {
              prev_result_sym->liveTo = ic_prev->seq;
            }
          }
          bitVectSetBit (OP_USES (IC_RESULT (ic)), ic->key);

          bitVectSetBit (ic->rlive, IC_RESULT (ic)->key);

          if (bitVectIsZero (OP_USES (IC_RESULT (ic_prev)))) {
            bitVectUnSetBit (ic->rlive, IC_RESULT (ic)->key);
            bitVectUnSetBit (OP_DEFS (IC_RESULT (ic_prev)), ic_prev->key);
            remiCodeFromeBBlock (ebp, ic_prev);
            hTabDeleteItem (&iCodehTab, ic_prev->key, ic_prev, DELETE_ITEM, NULL);
          }
        }

    /* if this is an itemp & result of a address of a true sym
       then mark this as rematerialisable   */
    if (ic->op == ADDRESS_OF &&
        IS_ITEMP (IC_RESULT (ic)) &&
        IS_TRUE_SYMOP (IC_LEFT (ic)) &&
        bitVectnBitsOn (OP_DEFS (IC_RESULT (ic))) == 1 &&
        !OP_SYMBOL (IC_LEFT (ic))->onStack)
      {

        debugLog ("  %d - %s. result is rematerializable\n", __LINE__,__FUNCTION__);

        OP_SYMBOL (IC_RESULT (ic))->remat = 1;
        OP_SYMBOL (IC_RESULT (ic))->rematiCode = ic;
        SPIL_LOC (IC_RESULT (ic)) = NULL;

      }

    /* if straight assignment then carry remat flag if
       this is the only definition */
    if (ic->op == '=' &&
        !POINTER_SET (ic) &&
        IS_SYMOP (IC_RIGHT (ic)) &&
        OP_SYMBOL (IC_RIGHT (ic))->remat &&
        bitVectnBitsOn (OP_SYMBOL (IC_RESULT (ic))->defs) <= 1 &&
        !isOperandGlobal (IC_RESULT (ic)) && 
        !OP_SYMBOL (IC_RESULT (ic))->addrtaken)
      {
        debugLog ("  %d - %s. straight rematerializable\n", __LINE__,__FUNCTION__);

        OP_SYMBOL (IC_RESULT (ic))->remat =
          OP_SYMBOL (IC_RIGHT (ic))->remat;
        OP_SYMBOL (IC_RESULT (ic))->rematiCode =
          OP_SYMBOL (IC_RIGHT (ic))->rematiCode;
      }

    /* if this is a +/- operation with a rematerizable
       then mark this as rematerializable as well */
    if ((ic->op == '+' || ic->op == '-') &&
        (IS_SYMOP (IC_LEFT (ic)) &&
         IS_ITEMP (IC_RESULT (ic)) &&
         OP_SYMBOL (IC_LEFT (ic))->remat &&
         bitVectnBitsOn (OP_DEFS (IC_RESULT (ic))) == 1 &&
         IS_OP_LITERAL (IC_RIGHT (ic))))
      {
        debugLog ("  %d - %s. rematerializable because op is +/-\n", __LINE__,__FUNCTION__);
        //int i =
        operandLitValue (IC_RIGHT (ic));
        OP_SYMBOL (IC_RESULT (ic))->remat = 1;
        OP_SYMBOL (IC_RESULT (ic))->rematiCode = ic;
        SPIL_LOC (IC_RESULT (ic)) = NULL;
      }


#if 0
    /* try to optimize FSR0 usage when reading data memory pointers */

    if(getenv("OPTIMIZE_NEAR_POINTER_GET")) {
          static int fsr0usage=0;
          static iCode *usic;

                if(POINTER_GET(ic)                              /* this is a memory read */
                        && ic->loop                                     /* this is in a loop */
                ) {
                        fprintf(stderr, "might optimize FSR0 usage\n");
                }
    }
#endif

    /* mark the pointer usages */
    if (POINTER_SET (ic) && IS_SYMOP (IC_RESULT (ic)))
      {
        OP_SYMBOL (IC_RESULT (ic))->uptr = 1;
        debugLog ("  marking as a pointer (set) =>");
        debugAopGet ("  result:", IC_RESULT (ic));

      }

    if (POINTER_GET (ic))
      {
        if(IS_SYMOP(IC_LEFT(ic))) {
          OP_SYMBOL (IC_LEFT (ic))->uptr = 1;
          debugLog ("  marking as a pointer (get) =>");
          debugAopGet ("  left:", IC_LEFT (ic));
        }

        if(getenv("OPTIMIZE_BITFIELD_POINTER_GET")) {
          if(IS_ITEMP(IC_LEFT(ic)) && IS_BITFIELD(OP_SYM_ETYPE(IC_LEFT(ic)))) {
            iCode *dic = ic->prev;

            fprintf(stderr, "%s:%d might give opt POINTER_GET && IS_BITFIELD(IC_LEFT)\n", __FILE__, __LINE__);

            if(dic && dic->op == '='
              && isOperandEqual(IC_RESULT(dic), IC_LEFT(ic))) {

                fprintf(stderr, "%s:%d && prev is '=' && prev->result == ic->left\n", __FILE__, __LINE__);


                /* replace prev->left with ic->left */
                IC_LEFT(ic) = IC_RIGHT(dic);
                IC_RIGHT(ic->prev) = NULL;

                /* remove ic->prev iCode (assignment) */
                remiCodeFromeBBlock (ebp, dic);
                bitVectUnSetBit(OP_SYMBOL(IC_RESULT(dic))->defs,ic->key);


                hTabDeleteItem (&iCodehTab, dic->key, dic, DELETE_ITEM, NULL);
            }
          }
        }
      }

        //debugLog("  %d   %s\n", __LINE__, __FUNCTION__);

    if (!SKIP_IC2 (ic))
      {
        //debugLog("  %d   %s\n", __LINE__, __FUNCTION__ );
        /* if we are using a symbol on the stack
           then we should say pic16_ptrRegReq */
        if (ic->op == IFX && IS_SYMOP (IC_COND (ic)))
          pic16_ptrRegReq += ((OP_SYMBOL (IC_COND (ic))->onStack ||
                               OP_SYMBOL (IC_COND (ic))->iaccess) ? 1 : 0);
        else if (ic->op == JUMPTABLE && IS_SYMOP (IC_JTCOND (ic)))
          pic16_ptrRegReq += ((OP_SYMBOL (IC_JTCOND (ic))->onStack ||
                               OP_SYMBOL (IC_JTCOND (ic))->iaccess) ? 1 : 0);
        else
          {

                //debugLog("   %d   %s\n", __LINE__, __FUNCTION__ );
            if (IS_SYMOP (IC_LEFT (ic)))
              pic16_ptrRegReq += ((OP_SYMBOL (IC_LEFT (ic))->onStack ||
                                   OP_SYMBOL (IC_LEFT (ic))->iaccess) ? 1 : 0);
            if (IS_SYMOP (IC_RIGHT (ic)))
              pic16_ptrRegReq += ((OP_SYMBOL (IC_RIGHT (ic))->onStack ||
                                   OP_SYMBOL (IC_RIGHT (ic))->iaccess) ? 1 : 0);
            if (IS_SYMOP (IC_RESULT (ic)))
              pic16_ptrRegReq += ((OP_SYMBOL (IC_RESULT (ic))->onStack ||
                                   OP_SYMBOL (IC_RESULT (ic))->iaccess) ? 1 : 0);
          }

        debugLog ("  %d - pointer reg req = %d\n", __LINE__,pic16_ptrRegReq);

      }

    /* if the condition of an if instruction
       is defined in the previous instruction then
       mark the itemp as a conditional */
    if ((IS_CONDITIONAL (ic) ||
         ((ic->op == BITWISEAND ||
           ic->op == '|' ||
           ic->op == '^') &&
          isBitwiseOptimizable (ic))) &&
        ic->next && ic->next->op == IFX &&
        isOperandEqual (IC_RESULT (ic), IC_COND (ic->next)) &&
        OP_SYMBOL (IC_RESULT (ic))->liveTo <= ic->next->seq)
      {

        debugLog ("  %d\n", __LINE__);
        OP_SYMBOL (IC_RESULT (ic))->regType = REG_CND;
        continue;
      }

        debugLog(" %d\n", __LINE__);

#ifndef NO_packRegsForSupport
    /* reduce for support function calls */
    if (ic->supportRtn || ic->op == '+' || ic->op == '-')
      packRegsForSupport (ic, ebp);
#endif

    /* if a parameter is passed, it's in W, so we may not
       need to place a copy in a register */
    if (ic->op == RECEIVE)
      packForReceive (ic, ebp);

#ifndef NO_packRegsForOneuse
    /* some cases the redundant moves can
       can be eliminated for return statements */
    if ((ic->op == RETURN || ic->op == SEND) &&
        !isOperandInFarSpace (IC_LEFT (ic)) &&
        !options.model)
      packRegsForOneuse (ic, IC_LEFT (ic), ebp);
#endif

#ifndef NO_packRegsForOneuse
    /* if pointer set & left has a size more than
       one and right is not in far space */
    if (POINTER_SET (ic) &&
        !isOperandInFarSpace (IC_RIGHT (ic)) &&
        !OP_SYMBOL (IC_RESULT (ic))->remat &&
        !IS_OP_RUONLY (IC_RIGHT (ic)) &&
        getSize (aggrToPtr (operandType (IC_RESULT (ic)), FALSE)) > 1)

      packRegsForOneuse (ic, IC_RESULT (ic), ebp);
#endif

#ifndef NO_packRegsForOneuse
    /* if pointer get */
    if (POINTER_GET (ic) &&
        !isOperandInFarSpace (IC_RESULT (ic)) &&
        !OP_SYMBOL (IC_LEFT (ic))->remat &&
        !IS_OP_RUONLY (IC_RESULT (ic)) &&
        getSize (aggrToPtr (operandType (IC_LEFT (ic)), FALSE)) > 1)

      packRegsForOneuse (ic, IC_LEFT (ic), ebp);
      debugLog("%d - return from packRegsForOneuse\n", __LINE__);
#endif

#ifndef NO_cast_peep
    /* if this is cast for intergral promotion then
       check if only use of  the definition of the
       operand being casted/ if yes then replace
       the result of that arithmetic operation with
       this result and get rid of the cast */
    if (ic->op == CAST) {

      sym_link *fromType = operandType (IC_RIGHT (ic));
      sym_link *toType = operandType (IC_LEFT (ic));

      debugLog ("  %d - casting\n", __LINE__);

      if (IS_INTEGRAL (fromType) && IS_INTEGRAL (toType) &&
          getSize (fromType) != getSize (toType)) {


        iCode *dic = packRegsForOneuse (ic, IC_RIGHT (ic), ebp);
        if (dic) {

          if (IS_ARITHMETIC_OP (dic)) {
                    debugLog("   %d   %s\n", __LINE__, __FUNCTION__ );

            bitVectUnSetBit(OP_SYMBOL(IC_RESULT(dic))->defs,dic->key);
            IC_RESULT (dic) = IC_RESULT (ic);
            remiCodeFromeBBlock (ebp, ic);
            bitVectUnSetBit(OP_SYMBOL(IC_RESULT(ic))->defs,ic->key);
            hTabDeleteItem (&iCodehTab, ic->key, ic, DELETE_ITEM, NULL);
            OP_DEFS (IC_RESULT (dic)) = bitVectSetBit (OP_DEFS (IC_RESULT (dic)), dic->key);
            ic = ic->prev;
          }  else

            OP_SYMBOL (IC_RIGHT (ic))->ruonly = 0;
        }
      } else {

        /* if the type from and type to are the same
           then if this is the only use then packit */
        if (compareType (operandType (IC_RIGHT (ic)),
                         operandType (IC_LEFT (ic))) == 1) {

          iCode *dic = packRegsForOneuse (ic, IC_RIGHT (ic), ebp);
          if (dic) {

                   debugLog(" %d\n", __LINE__);

            bitVectUnSetBit(OP_SYMBOL(IC_RESULT(dic))->defs,dic->key);
            IC_RESULT (dic) = IC_RESULT (ic);
            bitVectUnSetBit(OP_SYMBOL(IC_RESULT(ic))->defs,ic->key);
            remiCodeFromeBBlock (ebp, ic);
            hTabDeleteItem (&iCodehTab, ic->key, ic, DELETE_ITEM, NULL);
            OP_DEFS (IC_RESULT (dic)) = bitVectSetBit (OP_DEFS (IC_RESULT (dic)), dic->key);
            ic = ic->prev;
          }
        }
      }
    }
#endif

#if 1
    /* there are some problems with packing variables
     * it seems that the live range estimator doesn't
     * estimate correctly the liveranges of some symbols */

    /* pack for PUSH
       iTempNN := (some variable in farspace) V1
       push iTempNN ;
       -------------
       push V1
    */
    if (ic->op == IPUSH)
      {
        packForPush (ic, ebp);
      }
#endif

#ifndef NO_packRegsForAccUse
    /* pack registers for accumulator use, when the
       result of an arithmetic or bit wise operation
       has only one use, that use is immediately following
       the defintion and the using iCode has only one
       operand or has two operands but one is literal &
       the result of that operation is not on stack then
       we can leave the result of this operation in acc:b
       combination */
    if ((IS_ARITHMETIC_OP (ic)

         || IS_BITWISE_OP (ic)

         || ic->op == LEFT_OP || ic->op == RIGHT_OP

         ) &&
        IS_ITEMP (IC_RESULT (ic)) &&
        getSize (operandType (IC_RESULT (ic))) <= 1)

      packRegsForAccUse (ic);
#endif

  }
}

static void
dumpEbbsToDebug (eBBlock ** ebbs, int count)
{
  int i;

  if (!pic16_ralloc_debug || !debugF)
    return;

  for (i = 0; i < count; i++)
    {
      fprintf (debugF, "\n----------------------------------------------------------------\n");
      fprintf (debugF, "Basic Block %s : loop Depth = %d noPath = %d , lastinLoop = %d\n",
               ebbs[i]->entryLabel->name,
               ebbs[i]->depth,
               ebbs[i]->noPath,
               ebbs[i]->isLastInLoop);
      fprintf (debugF, "depth 1st num %d : bbnum = %d 1st iCode = %d , last iCode = %d\n",
               ebbs[i]->dfnum,
               ebbs[i]->bbnum,
               ebbs[i]->fSeq,
               ebbs[i]->lSeq);
      fprintf (debugF, "visited %d : hasFcall = %d\n",
               ebbs[i]->visited,
               ebbs[i]->hasFcall);

      fprintf (debugF, "\ndefines bitVector :");
      bitVectDebugOn (ebbs[i]->defSet, debugF);
      fprintf (debugF, "\nlocal defines bitVector :");
      bitVectDebugOn (ebbs[i]->ldefs, debugF);
      fprintf (debugF, "\npointers Set bitvector :");
      bitVectDebugOn (ebbs[i]->ptrsSet, debugF);
      fprintf (debugF, "\nin pointers Set bitvector :");
      bitVectDebugOn (ebbs[i]->inPtrsSet, debugF);
      fprintf (debugF, "\ninDefs Set bitvector :");
      bitVectDebugOn (ebbs[i]->inDefs, debugF);
      fprintf (debugF, "\noutDefs Set bitvector :");
      bitVectDebugOn (ebbs[i]->outDefs, debugF);
      fprintf (debugF, "\nusesDefs Set bitvector :");
      bitVectDebugOn (ebbs[i]->usesDefs, debugF);
      fprintf (debugF, "\n----------------------------------------------------------------\n");
      printiCChain (ebbs[i]->sch, debugF);
    }
}

void dbg_dumpregusage(void);

/*-----------------------------------------------------------------*/
/* pic16_assignRegisters - assigns registers to each live range as need  */
/*-----------------------------------------------------------------*/
void
pic16_assignRegisters (ebbIndex * ebbi)
{
  eBBlock ** ebbs = ebbi->bbOrder;
  int count = ebbi->count;
  iCode *ic;
  int i;

  debugLog ("<><><><><><><><><><><><><><><><><>\nstarting\t%s:%s", __FILE__, __FUNCTION__);
  debugLog ("\nebbs before optimizing:\n");
  dumpEbbsToDebug (ebbs, count);

  _inRegAllocator = 1;

  pic16_freeAllRegs();
#if 0
        dbg_dumpregusage();
        /* clear whats left over from peephole parser */
        pic16_dynAllocRegs= newSet();   //NULL;
//      pic16_dynStackRegs= newSet();   //NULL;
//      pic16_dynProcessorRegs=newSet();        //NULL;
//      pic16_dynDirectRegs=newSet();           //NULL;
//      pic16_dynDirectBitRegs=newSet();        //NULL;
//      pic16_dynInternalRegs=newSet();         //NULL;
//      pic16_dynAccessRegs=newSet();           //NULL;

//      dynDirectRegNames=NULL;
        dynAllocRegNames=NULL;
//      dynProcRegNames=NULL;
//      dynAccessRegNames=NULL;
#endif

  setToNull ((void *) &_G.funcrUsed);
  pic16_ptrRegReq = _G.stackExtend = _G.dataExtend = 0;


  /* change assignments this will remove some
     live ranges reducing some register pressure */
  for (i = 0; i < count; i++)
    pic16_packRegisters (ebbs[i]);


  if (0)
    {
      reg_info *reg;
      int hkey;

      debugLog("dir registers allocated so far:\n");
      reg = hTabFirstItem(dynDirectRegNames, &hkey);

      while(reg) {
          debugLog("  -- #%d reg = %s  key %d, rIdx = %d, size %d\n",i++,reg->name,hkey, reg->rIdx,reg->size);
          //      fprintf(stderr, "  -- #%d reg = %s  key %d, rIdx = %d, size %d\n",i++,reg->name,hkey, reg->rIdx,reg->size);
          reg = hTabNextItem(dynDirectRegNames, &hkey);
      }
    }

  /* liveranges probably changed by register packing
     so we compute them again */
  recomputeLiveRanges (ebbs, count, FALSE);

  if (options.dump_i_code)
    dumpEbbsToFileExt (DUMP_PACK, ebbi);

  /* first determine for each live range the number of
     registers & the type of registers required for each */
  regTypeNum ();

  /* start counting function temporary registers from zero */
  /* XXX: Resetting dynrIdx breaks register allocation,
   *      see #1489055, #1483693 (?), and #1445850! */
  //dynrIdx = 0;

  /* and serially allocate registers */
  serialRegAssign (ebbs, count);

#if 0
  debugLog ("ebbs after serialRegAssign:\n");
  dumpEbbsToDebug (ebbs, count);
#endif

  //pic16_freeAllRegs();

  /* if stack was extended then tell the user */
  if (_G.stackExtend)
    {
/*      werror(W_TOOMANY_SPILS,"stack", */
/*             _G.stackExtend,currFunc->name,""); */
      _G.stackExtend = 0;
    }

  if (_G.dataExtend)
    {
/*      werror(W_TOOMANY_SPILS,"data space", */
/*             _G.dataExtend,currFunc->name,""); */
      _G.dataExtend = 0;
    }

  /* after that create the register mask
     for each of the instruction */
  createRegMask (ebbs, count);

  /* redo that offsets for stacked automatic variables */
  redoStackOffsets ();

  if (options.dump_i_code)
    dumpEbbsToFileExt (DUMP_RASSGN, ebbi);

//  dumpLR(ebbs, count);

  /* now get back the chain */
  ic = iCodeLabelOptimize (iCodeFromeBBlock (ebbs, count));

  debugLog ("ebbs after optimizing:\n");
  dumpEbbsToDebug (ebbs, count);


  _inRegAllocator = 0;

  genpic16Code (ic);

  /* free up any _G.stackSpil locations allocated */
  applyToSet (_G.stackSpil, deallocStackSpil);
  _G.slocNum = 0;
  setToNull ((void *) &_G.stackSpil);
  setToNull ((void *) &_G.spiltSet);
  /* mark all registers as free */
  pic16_freeAllRegs ();


  debugLog ("leaving\n<><><><><><><><><><><><><><><><><>\n");
  debugLogClose ();
  return;
}
