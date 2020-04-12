/*
 * Simulator of microcontrollers (inst.cc)
 *
 * hc08 code base from Erik Petrich  epetrich@users.sourceforge.net
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
 * 
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
 *
 */

/* This file is part of microcontroller simulator: ucsim.

UCSIM is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

UCSIM is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with UCSIM; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA. */
/*@1@*/

//#include "ddconfig.h"

//#include <stdio.h>
//#include <stdlib.h>

// local
#include "hc08cl.h"
//#include "regshc08.h"
#include "hc08mac.h"


void
cl_hc08::incx(void)
{
  int hx = (regs.H << 8) | (regs.X);
  hx++;
  regs.H = (hx >> 8) & 0xff;
  regs.X = hx & 0xff;
}

int
cl_hc08::fetch2(void)
{
  int result;
  result = fetch() << 8;
  result |= fetch();
  tick(2);
  return result;
}

int
cl_hc08::fetchea(t_mem code, bool prefix)
{
  switch ((code >> 4) & 0x0f) {
    case 0x0: 
    case 0x1:
    case 0x3:
    case 0xb:
      tick(1);
      return fetch(); // Direct
    case 0x7:
    case 0xf:
      tick(1); // extra cycle needed, even without fetch
      return (regs.H << 8) | regs.X;  // IX
    case 0x6:
    case 0xe:
      tick(1);
      if (!prefix)
        return ((unsigned char)fetch())+((regs.H << 8) | regs.X); // IX1
      else
        return ((unsigned char)fetch())+regs.SP; // SP1
    case 0xd:
      if (!prefix)
        return fetch2()+((regs.H << 8) | regs.X); // IX2
      else
        return fetch2()+regs.SP; // SP2
    case 0xc:
      return fetch2();
    default:
      return(resHALT);
  }
}


int
cl_hc08::inst_nop(t_mem code, bool prefix)
{
  return(resGO);
}


int
cl_hc08::inst_transfer(t_mem code, bool prefix)
{
  int hx;
  
  switch (code) {
    case 0x84: // TAP
      regs.P = regs.A | 0x60;
      break;
    case 0x85: // TPA
      regs.A = regs.P | 0x60;
      break;
    case 0x97: // TAX
      regs.X = regs.A;
      break;
    case 0x9f: // TXA
      regs.A = regs.X;
      break;
    case 0x94: // TXS
      hx = (regs.H << 8) | regs.X;
      regs.SP = (hx - 1) & 0xffff;
      break;
    case 0x95: // TSX
      hx = regs.SP +1;
      regs.H = (hx >> 8) & 0xff;
      regs.X = hx & 0xff;
      break;
    default:
      return(resHALT);
  }
  return(resGO);
}


int
cl_hc08::inst_setclearflags(t_mem code, bool prefix)
{
  switch (code) {
    case 0x98:
      regs.P &= ~BIT_C;
      break;
    case 0x99:
      regs.P |= BIT_C;
      break;
    case 0x9a:
      regs.P &= ~BIT_I;
      break;
    case 0x9b:
      regs.P |= BIT_I;
      break;
    default:
      return(resHALT);
  }
  return(resGO);
}


int
cl_hc08::inst_rsp(t_mem code, bool prefix)
{
  regs.SP = 0x00ff;
  return(resGO);
}


int
cl_hc08::inst_nsa(t_mem code, bool prefix)
{
  regs.A = ((regs.A & 0xf0)>>4) | ((regs.A & 0x0f)<<4);
  return(resGO);
}



int
cl_hc08::inst_lda(t_mem code, bool prefix)
{
  regs.A = OPERAND(code, prefix);
  tick(1);
  FLAG_CLEAR(BIT_V);
  FLAG_NZ(regs.A);
  return(resGO);
}

int
cl_hc08::inst_ldx(t_mem code, bool prefix)
{
  regs.X = OPERAND(code, prefix);
  tick(1);
  FLAG_CLEAR(BIT_V);
  FLAG_NZ(regs.X);
  return(resGO);
}

int
cl_hc08::inst_sta(t_mem code, bool prefix)
{
  int ea = fetchea(code, prefix);

  FLAG_CLEAR(BIT_V);
  FLAG_NZ(regs.A);
  store1(ea, regs.A);
  tick(1);
  return(resGO);
}

int
cl_hc08::inst_stx(t_mem code, bool prefix)
{
  int ea = fetchea(code, prefix);

  FLAG_CLEAR(BIT_V);
  FLAG_NZ(regs.X);
  store1(ea, regs.X);
  tick(1);
  return(resGO);
}

int
cl_hc08::inst_add(t_mem code, bool prefix)
{
  int result, operand1, operand2;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  tick(1);
  result = operand1 + operand2;
  FLAG_NZ (result);
  FLAG_ASSIGN (BIT_V, 0x80 & (operand1 ^ operand2 ^ result ^ (result >>1)));
  FLAG_ASSIGN (BIT_C, 0x100 & result);
  FLAG_ASSIGN (BIT_H, 0x10 & (operand1 ^ operand2 ^ result));

  regs.A = result & 0xff;
  return(resGO);
}

int
cl_hc08::inst_adc(t_mem code, bool prefix)
{
  int result, operand1, operand2;
  int carryin = (regs.P & BIT_C)!=0;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  tick(1);
  result = operand1 + operand2 + carryin;
  FLAG_NZ (result);
  FLAG_ASSIGN (BIT_V, 0x80 & (operand1 ^ operand2 ^ result ^ (result >>1)));
  FLAG_ASSIGN (BIT_C, 0x100 & result);
  FLAG_ASSIGN (BIT_H, 0x10 & (operand1 ^ operand2 ^ result));

  regs.A = result & 0xff;
  return(resGO);
}

int
cl_hc08::inst_sub(t_mem code, bool prefix)
{
  int result, operand1, operand2;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  tick(1);
  result = operand1 - operand2;
  FLAG_NZ (result);
  FLAG_ASSIGN (BIT_V, 0x80 & (operand1 ^ operand2 ^ result ^ (result >>1)));
  FLAG_ASSIGN (BIT_C, 0x100 & result);

  regs.A = result & 0xff;
  return(resGO);
}

int
cl_hc08::inst_sbc(t_mem code, bool prefix)
{
  int result, operand1, operand2;
  int carryin = (regs.P & BIT_C)!=0;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  tick(1);
  result = operand1 - operand2 - carryin;
  FLAG_NZ (result);
  FLAG_ASSIGN (BIT_V, 0x80 & (operand1 ^ operand2 ^ result ^ (result >>1)));
  FLAG_ASSIGN (BIT_C, 0x100 & result);

  regs.A = result & 0xff;
  return(resGO);
}

int
cl_hc08::inst_cmp(t_mem code, bool prefix)
{
  int result, operand1, operand2;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  tick(1);
  result = operand1 - operand2;
  FLAG_NZ (result);
  FLAG_ASSIGN (BIT_V, 0x80 & (operand1 ^ operand2 ^ result ^ (result >>1)));
  FLAG_ASSIGN (BIT_C, 0x100 & result);

  return(resGO);
}

int
cl_hc08::inst_cpx(t_mem code, bool prefix)
{
  int result, operand1, operand2;

  operand1 = regs.X;
  operand2 = OPERAND(code, prefix);
  tick(1);
  result = operand1 - operand2;
  FLAG_NZ (result);
  FLAG_ASSIGN (BIT_V, 0x80 & (operand1 ^ operand2 ^ result ^ (result >>1)));
  FLAG_ASSIGN (BIT_C, 0x100 & result);

  return(resGO);
}

int
cl_hc08::inst_jmp(t_mem code, bool prefix)
{
  PC = fetchea(code, prefix);
  tick(1); // extra cycle to reload pipeline
  return(resGO);
}

int
cl_hc08::inst_jsr(t_mem code, bool prefix)
{
  int newPC = fetchea(code, prefix);

  push2(PC);
  tick(2);
  PC = newPC;
  tick(1); // extra cycle to reload pipeline

  return(resGO);
}

int
cl_hc08::inst_bsr(t_mem code, bool prefix)
{
  signed char ofs = fetch();

  push2(PC);
  tick(2);
  PC += ofs;
  tick(1); // extra cycle to reload pipeline

  return(resGO);
}

int
cl_hc08::inst_ais(t_mem code, bool prefix)
{
  regs.SP = regs.SP + (signed char)fetch();
  return(resGO);
}

int
cl_hc08::inst_aix(t_mem code, bool prefix)
{
  int hx = (regs.H << 8) | (regs.X);
  hx += (signed char)fetch();
  tick(1);
  regs.H = (hx >> 8) & 0xff;
  regs.X = hx & 0xff;
  return(resGO);
}

int
cl_hc08::inst_and(t_mem code, bool prefix)
{
  regs.A = regs.A & OPERAND(code, prefix);
  tick(1);
  FLAG_CLEAR(BIT_V);
  FLAG_NZ(regs.A);
  return(resGO);
}

int
cl_hc08::inst_bit(t_mem code, bool prefix)
{
  uchar operand = regs.A & OPERAND(code, prefix);
  tick(1);
  FLAG_CLEAR(BIT_V);
  FLAG_NZ(operand);
  return(resGO);
}

int
cl_hc08::inst_ora(t_mem code, bool prefix)
{
  regs.A = regs.A | OPERAND(code, prefix);
  tick(1);
  FLAG_CLEAR(BIT_V);
  FLAG_NZ(regs.A);
  return(resGO);
}

int
cl_hc08::inst_eor(t_mem code, bool prefix)
{
  regs.A = regs.A ^ OPERAND(code, prefix);
  tick(1);
  FLAG_CLEAR(BIT_V);
  FLAG_NZ(regs.A);
  return(resGO);
}

int
cl_hc08::inst_asr(t_mem code, bool prefix)
{
  int ea = 0xffff;
  uchar operand;
  
  if ((code & 0xf0) == 0x40)
    operand = regs.A;
  else if ((code & 0xf0) == 0x50)
    operand = regs.X;
  else {
    ea = fetchea(code,prefix);
    operand = get1(ea);
    tick(1);
  }

  FLAG_ASSIGN (BIT_C, operand & 1);
  operand = (operand >> 1) | (operand & 0x80);
  FLAG_NZ (operand);
  FLAG_ASSIGN (BIT_V, ((regs.P & BIT_C)!=0) ^ ((regs.P & BIT_N)!=0));

  if ((code & 0xf0) == 0x40)
    regs.A = operand;
  else if ((code & 0xf0) == 0x50)
    regs.X = operand;
  else {
    store1(ea, operand);
    tick(1);
    if ((code & 0x70) != 0x70)
      tick(1);
  }

  return(resGO);
}


int
cl_hc08::inst_lsr(t_mem code, bool prefix)
{
  int ea = 0xffff;
  uchar operand;
  
  if ((code & 0xf0) == 0x40)
    operand = regs.A;
  else if ((code & 0xf0) == 0x50)
    operand = regs.X;
  else {
    ea = fetchea(code,prefix);
    operand = get1(ea);
    tick(1);
  }

  FLAG_ASSIGN (BIT_C, operand & 1);
  operand = (operand >> 1) & 0x7f;
  FLAG_NZ (operand);
  FLAG_ASSIGN (BIT_V, ((regs.P & BIT_C)!=0) ^ ((regs.P & BIT_N)!=0));

  if ((code & 0xf0) == 0x40)
    regs.A = operand;
  else if ((code & 0xf0) == 0x50)
    regs.X = operand;
  else {
    store1(ea, operand);
    tick(1);
    if ((code & 0x70) != 0x70)
      tick(1);
  }
  return(resGO);
}


int
cl_hc08::inst_lsl(t_mem code, bool prefix)
{
  int ea = 0xffff;
  uchar operand;
  
  if ((code & 0xf0) == 0x40)
    operand = regs.A;
  else if ((code & 0xf0) == 0x50)
    operand = regs.X;
  else {
    ea = fetchea(code,prefix);
    operand = get1(ea);
    tick(1);
  }

  FLAG_ASSIGN (BIT_C, operand & 0x80);
  operand = (operand << 1);
  FLAG_NZ (operand);
  FLAG_ASSIGN (BIT_V, ((regs.P & BIT_C)!=0) ^ ((regs.P & BIT_N)!=0));

  if ((code & 0xf0) == 0x40)
    regs.A = operand;
  else if ((code & 0xf0) == 0x50)
    regs.X = operand;
  else {
    store1(ea, operand);
    tick(1);
    if ((code & 0x70) != 0x70)
      tick(1);
  }
  return(resGO);
}


int
cl_hc08::inst_rol(t_mem code, bool prefix)
{
  uchar c = (regs.P & BIT_C)!=0;
  int ea = 0xffff;
  uchar operand;
  
  if ((code & 0xf0) == 0x40)
    operand = regs.A;
  else if ((code & 0xf0) == 0x50)
    operand = regs.X;
  else {
    ea = fetchea(code,prefix);
    operand = get1(ea);
    tick(1);
  }

  FLAG_ASSIGN (BIT_C, operand & 0x80);
  operand = (operand << 1) | c;
  FLAG_NZ (operand);
  FLAG_ASSIGN (BIT_V, ((regs.P & BIT_C)!=0) ^ ((regs.P & BIT_N)!=0));

  if ((code & 0xf0) == 0x40)
    regs.A = operand;
  else if ((code & 0xf0) == 0x50)
    regs.X = operand;
  else {
    store1(ea, operand);
    tick(1);
    if ((code & 0x70) != 0x70)
      tick(1);
  }
  return(resGO);
}


int
cl_hc08::inst_ror(t_mem code, bool prefix)
{
  uchar c = (regs.P & BIT_C)!=0;
  int ea = 0xffff;
  uchar operand;
  
  if ((code & 0xf0) == 0x40)
    operand = regs.A;
  else if ((code & 0xf0) == 0x50)
    operand = regs.X;
  else {
    ea = fetchea(code,prefix);
    operand = get1(ea);
    tick(1);
  }

  FLAG_ASSIGN (BIT_C, operand & 1);
  operand = (operand >> 1) | (c << 7);
  FLAG_NZ (operand);
  FLAG_ASSIGN (BIT_V, ((regs.P & BIT_C)!=0) ^ ((regs.P & BIT_N)!=0));

  if ((code & 0xf0) == 0x40)
    regs.A = operand;
  else if ((code & 0xf0) == 0x50)
    regs.X = operand;
  else {
    store1(ea, operand);
    tick(1);
    if ((code & 0x70) != 0x70)
      tick(1);
  }
  return(resGO);
}


int
cl_hc08::inst_inc(t_mem code, bool prefix)
{
  int ea = 0xffff;
  uchar operand;
  
  if ((code & 0xf0) == 0x40)
    operand = regs.A;
  else if ((code & 0xf0) == 0x50)
    operand = regs.X;
  else {
    ea = fetchea(code,prefix);
    operand = get1(ea);
    tick(1);
  }

  operand++;
  FLAG_NZ (operand);
  FLAG_ASSIGN (BIT_V, operand == 0x80);

  if ((code & 0xf0) == 0x40)
    regs.A = operand;
  else if ((code & 0xf0) == 0x50)
    regs.X = operand;
  else {
    store1(ea, operand);
    tick(1);
    if ((code & 0x70) != 0x70)
      tick(1);
  }
  return(resGO);
}


int
cl_hc08::inst_dec(t_mem code, bool prefix)
{
  int ea = 0xffff;
  uchar operand;
  
  if ((code & 0xf0) == 0x40)
    operand = regs.A;
  else if ((code & 0xf0) == 0x50)
    operand = regs.X;
  else {
    ea = fetchea(code,prefix);
    operand = get1(ea);
    tick(1);
  }

  operand--;
  FLAG_NZ (operand);
  FLAG_ASSIGN (BIT_V, operand == 0x7f);

  if ((code & 0xf0) == 0x40)
    regs.A = operand;
  else if ((code & 0xf0) == 0x50)
    regs.X = operand;
  else {
    store1(ea, operand);
    tick(1);
    if ((code & 0x70) != 0x70)
      tick(1);
  }
  return(resGO);
}

int
cl_hc08::inst_dbnz(t_mem code, bool prefix)
{
  int ea = 0xffff;
  uchar operand;
  signed char ofs;
  
  if ((code & 0xf0) == 0x40)
    operand = regs.A;
  else if ((code & 0xf0) == 0x50)
    operand = regs.X;
  else {
    ea = fetchea(code,prefix);
    operand = get1(ea);
    tick(1);
  }

  operand--;
  FLAG_NZ (operand);
  FLAG_ASSIGN (BIT_V, operand == 0x7f);

  if ((code & 0xf0) == 0x40)
    regs.A = operand;
  else if ((code & 0xf0) == 0x50)
    regs.X = operand;
  else {
    store1(ea, operand);
    tick(1);
    if ((code & 0x70) != 0x70)
      tick(1);
  }

  ofs = fetch();
  tick(1);
  if (operand)
    PC += ofs;
  tick(1); // extra cycle to reload pipeline

  return(resGO);
}


int
cl_hc08::inst_tst(t_mem code, bool prefix)
{
  int ea = 0xffff;
  uchar operand;

  if ((code & 0xf0) == 0x40)
    operand = regs.A;
  else if ((code & 0xf0) == 0x50)
    operand = regs.X;
  else {
    ea = fetchea(code,prefix);
    operand = get1(ea);
    tick(1);
  }

  FLAG_NZ (operand);
  FLAG_CLEAR (BIT_V);
  if ((code & 0xf0) == 0x30 || (code & 0xf0) == 0x60)
    tick(1);

  return(resGO);
}


int
cl_hc08::inst_clr(t_mem code, bool prefix)
{
  int ea = 0xffff;
  uchar operand;

  // clr uses read-modify-write cycles, so simulate the read even if the data isn't used
  if ((code & 0xf0) == 0x40)
    operand = regs.A;
  else if ((code & 0xf0) == 0x50)
    operand = regs.X;
  else {
    ea = fetchea(code,prefix);
    operand = get1(ea);
    tick(1);
  }

  operand = 0;
  FLAG_CLEAR (BIT_V);
  FLAG_CLEAR (BIT_N);
  FLAG_SET (BIT_Z);

  if ((code & 0xf0) == 0x40)
    regs.A = operand;
  else if ((code & 0xf0) == 0x50)
    regs.X = operand;
  else {
    store1(ea, operand);
    if ((code & 0x70) != 0x70)
      tick(1);
  }
  return(resGO);
}


int
cl_hc08::inst_clrh(t_mem code, bool prefix)
{
  FLAG_CLEAR (BIT_V);
  FLAG_CLEAR (BIT_N);
  FLAG_SET (BIT_Z);
  regs.H = 0;
  return(resGO);
}


int
cl_hc08::inst_com(t_mem code, bool prefix)
{
  int ea = 0xffff;
  uchar operand;
  
  if ((code & 0xf0) == 0x40)
    operand = regs.A;
  else if ((code & 0xf0) == 0x50)
    operand = regs.X;
  else {
    ea = fetchea(code,prefix);
    operand = get1(ea);
    tick(1);
  }

  operand = ~operand;
  FLAG_SET (BIT_C);
  FLAG_NZ (operand);
  FLAG_CLEAR (BIT_V);

  if ((code & 0xf0) == 0x40)
    regs.A = operand;
  else if ((code & 0xf0) == 0x50)
    regs.X = operand;
  else {
    store1(ea, operand);
    if ((code & 0x70) != 0x70)
      tick(1);
  }
  return(resGO);
}


int
cl_hc08::inst_neg(t_mem code, bool prefix)
{
  int ea = 0xffff;
  uchar operand;
  
  if ((code & 0xf0) == 0x40)
    operand = regs.A;
  else if ((code & 0xf0) == 0x50)
    operand = regs.X;
  else {
    ea = fetchea(code,prefix);
    operand = get1(ea);
    tick(1);
  }

  FLAG_ASSIGN (BIT_V, operand==0x80);
  FLAG_ASSIGN (BIT_C, operand!=0x00);
  operand = -operand;
  FLAG_NZ (operand);

  if ((code & 0xf0) == 0x40)
    regs.A = operand;
  else if ((code & 0xf0) == 0x50)
    regs.X = operand;
  else {
    store1(ea, operand);
    if ((code & 0x70) != 0x70)
      tick(1);
  }
  return(resGO);
}



int
cl_hc08::inst_pushpull(t_mem code, bool prefix)
{
  switch (code) {
    case 0x86:
      pop1(regs.A);
      tick(2);
      break;
    case 0x87:
      push1(regs.A);
      tick(1);
      break;
    case 0x88:
      pop1(regs.X);
      tick(2);
      break;
    case 0x89:
      push1(regs.X);
      tick(1);
      break;
    case 0x8a:
      pop1(regs.H);
      tick(2);
      break;
    case 0x8b:
      push1(regs.H);
      tick(1);
      break;
    default:
      return(resHALT);
  }
  return(resGO);
}




int
cl_hc08::inst_stop(t_mem code, bool prefix)
{
  FLAG_CLEAR (BIT_I);
  return(resGO);
}


int
cl_hc08::inst_wait(t_mem code, bool prefix)
{
  FLAG_CLEAR (BIT_I);
  return(resGO);
}


int
cl_hc08::inst_daa(t_mem code, bool prefix)
{
  uchar lsn, msn;

  lsn = regs.A & 0xf;
  msn = (regs.A >> 4) & 0xf;
  if (regs.P & BIT_H) {
    lsn += 16;
    msn = (msn-1) & 0xf;
  }
  if (regs.P & BIT_C)
    msn += 16;

  FLAG_CLEAR (BIT_C);
  while (lsn>9) {
    lsn -= 10;
    msn++;
  }
  if (msn>9) {
    msn = msn % 10;
    FLAG_SET (BIT_C);
  }

  return(resGO);
}

int
cl_hc08::inst_mul(t_mem code, bool prefix)
{
  int result = regs.A * regs.X;
  regs.A = result & 0xff;
  regs.X = (result >> 8) & 0xff;
  FLAG_CLEAR (BIT_C);
  FLAG_CLEAR (BIT_H);
  tick(4);
  return(resGO);
}

int
cl_hc08::inst_div(t_mem code, bool prefix)
{
  unsigned int dividend = (regs.H << 8) | regs.A;
  unsigned int quotient;

  if (regs.X) {
    quotient = dividend / (unsigned int)regs.X;
    if (quotient<=0xff) {
      regs.A = quotient;
      regs.H = dividend % regs.X;
      FLAG_CLEAR (BIT_C);
      FLAG_ASSIGN (BIT_Z, quotient==0);
    }
    else
      FLAG_SET (BIT_C);  // overflow
  } else
    FLAG_SET (BIT_C);    // division by zero
  tick(5);

  return(resGO);
}


int
cl_hc08::inst_condbranch(t_mem code, bool prefix)
{
  bool taken;
  signed char ofs;
  unsigned char maskedP;
  
  if ((code & 0xf0)==0x20) {
    switch ((code>>1) & 7) {
      case 0: // BRA
        taken = 1;
        break;
      case 1: // BHI
        taken = !(regs.P & (BIT_C | BIT_Z));
        break;
      case 2: // BCC
        taken = !(regs.P & BIT_C);
        break;
      case 3: // BNE
        taken = !(regs.P & BIT_Z);
        break;
      case 4: // BHCC
        taken = !(regs.P & BIT_H);
        break;
      case 5: // BPL
        taken = !(regs.P & BIT_N);
        break;
      case 6: // BMC
        taken = !(regs.P & BIT_I);
        break;
      case 7: // BIL
        taken = 0; // TODO: should read simulated IRQ# pin
      default:
        return(resHALT);
    } 
  }
  else if ((code & 0xf0)==0x90) {
    switch ((code>>1) & 7) {
      case 0: // BGE
        maskedP = regs.P & (BIT_N | BIT_V);
        taken = !maskedP || (maskedP == (BIT_N | BIT_V));
        break;
      case 1: // BGT
        maskedP = regs.P & (BIT_N | BIT_V | BIT_Z);
        taken = !maskedP || (maskedP == (BIT_N | BIT_V));
        break;
      default:
        return(resHALT);
    }
  }
  else
    return(resHALT);
  
  if (code & 1)
    taken = ! taken;
  
  ofs = fetch();
  tick(1);
  if (taken)
    PC += ofs;
  tick(1); // extra cycle to reload pipeline

  return(resGO);
}

int
cl_hc08::inst_bitsetclear(t_mem code, bool prefix)
{
  uchar bit = (code >> 1) & 7;
  int ea = fetchea(code, prefix);
  uchar operand = get1(ea);
  tick(1);

  if (code & 1)
    operand &= ~(1 << bit);
  else
    operand |= (1 << bit);
  tick(1);
  store1(ea, operand);
  tick(1);
  return(resGO);
}

int
cl_hc08::inst_bittestsetclear(t_mem code, bool prefix)
{
  uchar bit = (code >> 1) & 7;
  int ea = fetchea(code, prefix);
  uchar operand = get1(ea);
  signed char ofs;
  bool taken;

  tick(1);
  if (code & 1)
    taken = !(operand & (1 << bit));  // brclr
  else
    taken = (operand & (1 << bit));   // brset

  ofs = fetch();
  tick(1);
  if (taken)
    PC += ofs;
  tick(1); // extra cycle to reload pipeline

  FLAG_ASSIGN (BIT_C, operand & (1 << bit));
  return(resGO);
}

int
cl_hc08::inst_cbeq(t_mem code, bool prefix)
{
  int ea;
  uchar operand1, operand2;
  signed char ofs;
    
  if ((code & 0xf0) == 0x40) {
    operand1 = regs.A;
    operand2 = fetch();
    tick(1);
  }
  else if ((code & 0xf0) == 0x50) {
    operand1 = regs.X;
    operand2 = fetch();
    tick(1);
  }
  else {
    ea = fetchea(code,prefix);
    operand1 = get1(ea);
    tick(1);
    operand2 = regs.A;
  }

  ofs = fetch();
  tick(1);
  if (operand1==operand2)
    PC += ofs;  

  if (code == 0x71 || (!prefix && code == 0x61))
    incx();
  tick(1); // extra cycle to reload pipeline

  return(resGO);
}

int
cl_hc08::inst_rti(t_mem code, bool prefix)
{
  pop1(regs.P);
  tick(1);
  regs.P |= 0x60;
  pop1(regs.A);
  tick(1);
  pop1(regs.X);
  tick(1);
  pop2(PC);
  tick(2);
  tick(3); // pipeline reload and some extra overhead?

  return(resGO);
}

int
cl_hc08::inst_rts(t_mem code, bool prefix)
{
  pop2(PC);
  tick(2);
  tick(3); // pipeline reload and some extra overhead?

  return(resGO);
}


int
cl_hc08::inst_mov(t_mem code, bool prefix)
{
  int ea;
  uchar operand;
  bool aix;
  int hx = (regs.H << 8) | (regs.X);
  
  switch (code) {
    case 0x4e:	//mov opr8a,opr8a
      operand = get1(fetch());
      tick(2);
      ea = fetch();
      tick(1);
      aix = 0;
      break;
    case 0x5e:	//mov opr8a,x+
      operand = get1(fetch());
      tick(2);
      tick(1);
      ea = hx;
      aix = 1;
      break;
    case 0x6e:	//mov #opr8i,opr8a
      operand = fetch();
      tick(1);
      ea = fetch();
      tick(1);
      aix = 0;
      break;
    case 0x7e:	//mov x+,opr8a
      operand = get1(hx);
      tick(1);
      ea = fetch();
      tick(1);
      tick(1);
      aix = 1;
      break;
    default:
      return(resHALT);
  }

  store1(ea, operand);
  tick(1);
  if (aix)
    incx();

  FLAG_NZ(operand);
  FLAG_CLEAR(BIT_V);

  return(resGO);
}


int
cl_hc08::inst_sthx(t_mem code, bool prefix)
{
  int ea;
  
  if (code == 0x35) {
    ea = fetch();
    tick(1);
  }
  else if ((code == 0x96) && (type->type == CPU_HCS08))
  {
    ea = fetch2();
    tick(2);
  }
  else if (prefix && (code == 0xff) && (type->type == CPU_HCS08))
  {
    ea = regs.SP + fetch();
    tick(1);
  }
  else
    return(resINV_INST);
  
  store1(ea, regs.H);
  tick(1);
  store1((ea+1) & 0xffff, regs.X);
  tick(1);

  FLAG_CLEAR(BIT_V);
  FLAG_ASSIGN(BIT_N, regs.H & 0x80);
  FLAG_ASSIGN(BIT_Z, !regs.X && !regs.H);
  return(resGO);
}

int
cl_hc08::inst_ldhx(t_mem code, bool prefix)
{
  int ea;
  
  if (code == 0x45) {
    regs.H = fetch();
    tick(1);
    regs.X = fetch();
    tick(1);
  }
  else if (code == 0x55) {
    ea = fetch();
    tick(1);
    regs.H = get1(ea);
    tick(1);
    regs.X = get1(ea+1);
    tick(1);
  }
  else if ((code == 0x32) && (type->type == CPU_HCS08)) {
    ea = fetch2();
    tick(2);
    regs.H = get1(ea);
    tick(1);
    regs.X = get1(ea+1);
    tick(1);
  }
  else if (prefix && (code == 0xae) && (type->type == CPU_HCS08)) {
    ea = (regs.H << 8) | regs.X;
    tick(1);
    regs.H = get1(ea);
    tick(1);
    regs.X = get1(ea+1);
    tick(1);
  }
  else if (prefix && (code == 0xbe) && (type->type == CPU_HCS08)) {
    ea = ((regs.H << 8) | regs.X) + fetch2();
    tick(2);
    regs.H = get1(ea);
    tick(1);
    regs.X = get1(ea+1);
    tick(1);
  }
  else if (prefix && (code == 0xce) && (type->type == CPU_HCS08)) {
    ea = ((regs.H << 8) | regs.X) + fetch();
    tick(1);
    regs.H = get1(ea);
    tick(1);
    regs.X = get1(ea+1);
    tick(1);
  }
  else if (prefix && (code == 0xfe) && (type->type == CPU_HCS08)) {
    ea = regs.SP + fetch();
    tick(1);
    regs.H = get1(ea);
    tick(1);
    regs.X = get1(ea+1);
    tick(1);
  }
  else
    return(resINV_INST);
  
  FLAG_CLEAR(BIT_V);
  FLAG_ASSIGN(BIT_N, regs.H & 0x80);
  FLAG_ASSIGN(BIT_Z, !regs.X && !regs.H);
  return(resGO);
}


int
cl_hc08::inst_cphx(t_mem code, bool prefix)
{
  int ea;
  int hx;
  int operand;
  int result;
  
  if (code == 0x65) {
    operand = fetch2();
    tick(2);
  }
  else if (code == 0x75) {
    ea = fetch();
    tick(1);
    operand = (get1(ea) << 8) | get1(ea+1);
    tick(2);
    tick(1);
  }
  else if ((code == 0x3e) && (type->type == CPU_HCS08)) {
    ea = fetch2();
    tick(2);
    operand = (get1(ea) << 8) | get1(ea+1);
    tick(2);
    tick(1);
  }
  else if (prefix && (code == 0xf3) && (type->type == CPU_HCS08)) {
    ea = ((unsigned char)fetch())+regs.SP;
    tick(1);
    operand = (get1(ea) << 8) | get1(ea+1);
    tick(2);
    tick(1);
  }
  else
    return(resINV_INST);

  hx = (regs.H << 8) | regs.X;
  result = hx-operand;

  FLAG_ASSIGN (BIT_V, 0x8000 & (hx ^ operand ^ result ^ (result>>1)));
  FLAG_ASSIGN (BIT_C, 0x10000 & result);
  FLAG_ASSIGN(BIT_N, result & 0x8000);
  FLAG_ASSIGN(BIT_Z, !(result & 0xffff));

  return(resGO);
}

int
cl_hc08::inst_swi(t_mem code, bool prefix)
{
  push2(PC);
  tick(2);
  push1(regs.X);
  tick(1);
  push1(regs.A);
  tick(1);
  push1(regs.P);
  tick(1);
  FLAG_CLEAR(BIT_I);

  PC = get2(0xfffc);
  tick(2);
  tick(3);

  return(resGO);
}


/* End of hc08.src/inst.cc */
