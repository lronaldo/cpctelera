/*-------------------------------------------------------------------------
  gen.h - header file for code generation for Padauk

             Written By -  Philipp Krause . pkk@spth.de (2018)

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

#ifndef PDKGEN_H
#define PDKGEN_H 1

typedef enum
{
  AOP_INVALID,
  /* Is a literal */
  AOP_LIT = 1,
  /* Is in a register */
  AOP_REG,
  /* Is partially in a register, partially in direct space */
  AOP_REGDIR,
  /* Is on the stack */
  AOP_STK,
  /* Is a stack location */
  AOP_STL,
  /* Is an immediate value */
  AOP_IMMD,
  /* Is in direct space */
  AOP_DIR,
  /* I/O register */
  AOP_SFR,
  /* Is in code space */
  AOP_CODE,
  /* Read undefined, discard writes */
  AOP_DUMMY,
  /* Implicit condition operand */
  AOP_CND,
}
AOP_TYPE;

/* asmop_byte: A type for the location a single byte
   of an operand can be in */
typedef struct asmop_byte
{
  bool in_reg;
  union
  {
    reg_info *reg;    /* Register this byte is in. */
    int stk;          /* Stack offset for this byte. */
  } byteu;
} asmop_byte;

/* asmop: A homogenised type for all the different
   spaces an operand can be in */
typedef struct asmop
{
  AOP_TYPE type;
  short size;
  union
  {
    value *aop_lit;
    struct
      {
        char *immd;
        int immd_off;
        bool code; /* in code space */
        bool func; /* function address */
      };
    char *aop_dir;
    int stk_off;
    asmop_byte bytes[8];
  } aopu;
}
asmop;

void genPdkCode (iCode *);
void pdk_emitDebuggerSymbol (const char *);

extern bool pdk_assignment_optimal;
void pdk_init_asmops (void);

#endif

