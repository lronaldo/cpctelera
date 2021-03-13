/*
 * Simulator of microcontrollers (inst.cc)
 *
 * stm8 code base from Vaclav Peroutka vaclavpe@users.sourceforge.net
 * and Valentin Dudouyt valentin.dudouyt@gmail.com
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

#include <stdio.h>
//#include <stdlib.h>

// local
#include "stm8cl.h"
//#include "regsstm8.h"
#include "stm8mac.h"

int
cl_stm8::fetchea(t_mem code, unsigned char prefix)
{
   int ftc;

   switch ((code >> 4) & 0x0f) {
   case 0x1: return (unsigned char)fetch()+regs.SP;  // SP indexed

   case 0xb: return fetch();           // direct short


   case 0xc:
      if ( 0 == prefix) {              // direct long
         return fetch2();
      } else if ( 0x72 == prefix) {    // long pointer
         ftc = fetch2();
         return get2(ftc);
      } else if ( 0x92 == prefix) {    // short pointer
         ftc = fetch();
         return get2(ftc);
      } else {
         printf("************* bad prefix !!!!\n");
         return (resHALT);
      }

   case 0xd:
      if ( 0 == prefix) {              // long offset with X reg
         return fetch2()+regs.X;
      } else if ( 0x72 == prefix) {    // long pointer to offset with X reg
         ftc = fetch2();
         return get2(ftc)+regs.X;
      } else if ( 0x90 == prefix) {    // long offset with Y reg
         return fetch2()+regs.Y;
      } else if ( 0x91 == prefix) {    // short pointer to offset with Y reg
         ftc = fetch();
         return get2(ftc)+regs.Y;
      } else if ( 0x92 == prefix) {    // short pointer to offset with X reg
         ftc = fetch();
         return get2(ftc)+regs.X;
      } else {
         return( resHALT);
      }


   case 0xe:
      if ( 0 == prefix) {               // short offset with X reg
         return fetch()+regs.X;
      } else if ( 0x90 == prefix) {     // short offset with Y reg
         return fetch()+regs.Y;
      } else {
         return( resHALT);
      }

   case 0xf:
      if ( 0 == prefix) {               // X index
         return regs.X;
      } else if ( 0x90 == prefix) {     // Y index
         return regs.Y;
      } else {
         return( resHALT);
      }

   default:
      return(resHALT);
   }
}

int
cl_stm8::get_dest(t_mem code, unsigned char prefix)
{
  int resaddr, ftc;

  switch ((code >> 4) & 0x0f) {
    case 0x0:
	  if ( 0 == prefix) {              // short offset with SP
		 resaddr = fetch()+regs.SP;
	  } else {
	    resaddr = ( resHALT);
	  }
     break;

    case 0x3:
	  if ( 0 == prefix) {              // short direct
		 resaddr = fetch();
     } else if ( 0x72 == prefix) {    // long indirect - pointer
         ftc = fetch2();
       resaddr = get2(ftc);
     } else if ( 0x92 == prefix) {    // short indirect - pointer
         ftc = fetch();
       resaddr = get2(ftc);
	  } else {
	    resaddr = ( resHALT);
	  }
     break;

    case 0x4:
	  if ( 0x72 == prefix) {           // long offset with X
         ftc = fetch2();
         resaddr = ftc + regs.X;
     } else if ( 0x90 == prefix) {    // long offset with Y
         ftc = fetch2();
         resaddr = ftc + regs.Y;
	  } else {
         resaddr = ( resHALT);
	  }
     break;

    case 0x5:
	  if ( 0x72 == prefix) {           // long direct
         resaddr = fetch2();
	  } else {
         resaddr = ( resHALT);
	  }
     break;

    case 0x6:
          if ( 0 == prefix) {              // short offset with X
		resaddr = (fetch()+regs.X);
	  } else if ( 0x72 == prefix) {    // long pointer to offset with X
          ftc = fetch2();
	  resaddr = (get2(ftc)+regs.X);
	  } else if ( 0x90 == prefix) {    // short offset with Y
	    resaddr = (fetch()+regs.Y);
	  } else if ( 0x91 == prefix) {    // short pointer to offset with Y
          ftc = fetch();
	    resaddr = (get2(ftc)+regs.Y);
	  } else if ( 0x92 == prefix) {    // short pointer to offset with X
          ftc = fetch();
	    resaddr = (get2(ftc)+regs.X);
	  } else {
	    resaddr =( resHALT);
	  }
     break;

    case 0x7:
	  if ( 0 == prefix) {              // X index
		resaddr = regs.X;
	  } else if ( 0x90 == prefix) {    // Y index
	    resaddr = regs.Y;
	  } else {
	    resaddr =( resHALT);
	  }
     break;

    default:
      resaddr =(resHALT);
      break;
  }

  return resaddr;
}

void
cl_stm8::flag_cvh(int x, int m, int r, bool byte, bool add, int mask)
{
  bool xt, xs, xh, mt, ms, mh, rt, rs, rh;

  if (byte)
    {
      xt = !!(x & 0x80);
      xs = !!(x & 0x40);
      xh = !!(x & 0x08);
      mt = !!(m & 0x80);
      ms = !!(m & 0x40);
      mh = !!(m & 0x08);
      rt = !!(r & 0x80);
      rs = !!(r & 0x40);
      rh = !!(r & 0x08);
    }
  else
    {
      xt = !!(x & 0x8000);
      xs = !!(x & 0x4000);
      xh = !!(x & 0x0080);
      mt = !!(m & 0x8000);
      ms = !!(m & 0x4000);
      mh = !!(m & 0x0080);
      rt = !!(r & 0x8000);
      rs = !!(r & 0x4000);
      rh = !!(r & 0x0080);
    }

  if (add)
    {
      if (mask & BIT_V)
        FLAG_ASSIGN (BIT_V, ((xt && mt) || (mt && !rt) || (!rt && xt)) ^ ((xs && ms) || (ms && !rs) || (!rs && xs)));
      if (mask & BIT_C)
        FLAG_ASSIGN (BIT_C, (xt && mt) || (mt && !rt) || (!rt && xt));
      if (mask & BIT_H)
        FLAG_ASSIGN (BIT_H, (xh && mh) || (mh && !rh) || (!rh && xh));
    }
  else
    {
      if (mask & BIT_V)
        FLAG_ASSIGN (BIT_V, ((!xt && mt) || (!xt && rt) || (xt && mt && rt)) ^ ((!xs && ms) || (!xs && rs) || (xs && ms && rs)));
      if (mask & BIT_H)
        FLAG_ASSIGN (BIT_H, (!xh && mh) || (!xh && rh) || (xh && mh && rh));
      if (mask & BIT_C)
        FLAG_ASSIGN (BIT_C, (!xt && mt) || (!xt && rt) || (xt && mt && rt));
    }
}

#define FLAG_CVH_BYTE_ADD(x, m, r, mask) flag_cvh(x, m, r, true, true, mask)
#define FLAG_CVH_BYTE_SUB(x, m, r, mask) flag_cvh(x, m, r, true, false, mask)
#define FLAG_CVH_WORD_ADD(x, m, r, mask) flag_cvh(x, m, r, false, true, mask)
#define FLAG_CVH_WORD_SUB(x, m, r, mask) flag_cvh(x, m, r, false, false, mask)

int
cl_stm8::inst_adc(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;
  int carryin = !!(regs.CC & BIT_C);

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  result = operand1 + operand2 + carryin;

  FLAG_NZ (result);
  FLAG_CVH_BYTE_ADD(operand1, operand2, result, BIT_C | BIT_V | BIT_H);

  regs.A = result & 0xff;
  if (prefix && (prefix!=0x90))
    tick(3);
  return(resGO);
}

int
cl_stm8::inst_add(t_mem code, unsigned char prefix)
{
  FLAG_CLEAR(BIT_C);
  return inst_adc(code, prefix);
}

int
cl_stm8::get_1(unsigned int addr)
{
  vc.rd++;
  return ram->read((t_addr) (addr));
}

int
cl_stm8::get2(unsigned int addr)
{
  vc.rd+= 2;
  return((ram->read((t_addr) (addr)) << 8) | ram->read((t_addr) (addr+1)));
}

int
cl_stm8::get3(unsigned int addr)
{
  vc.rd+= 3;
  return((ram->read((t_addr) (addr)) << 16) | (ram->read((t_addr) (addr+1)) << 8) |ram->read((t_addr) (addr+2)));
}

int
cl_stm8::inst_addw(t_mem code, unsigned char prefix)
{
  long int result, operand1, operand2, nibble_high, nibble_low;
  u16_t *dest_ptr;
  bool sub;

  nibble_high = (code >> 4) & 0x0f;
  nibble_low = code & 0x0f;
  dest_ptr = nibble_low == 0x09 || nibble_low == 0x02 ? &regs.Y : &regs.X;
  operand1 = *dest_ptr;

  switch(nibble_high)
  {
    case 0x1:
    case 0xa: operand2 = fetch2(); break; // Immediate
    case 0xb: operand2 = get2(fetch2()); break; // Long
    case 0xf: operand2 = get2(regs.SP + fetch()); break; // sp-indexed
    default: return(resHALT);
  }

  switch(nibble_low)
  {
    case 0x0:
    case 0x2:
    case 0xd: sub = true; break;
    case 0x9:
    case 0xb:
    case 0xc: sub = false; break;
    default: return(resHALT);
  }

  if (sub)
    result = operand1 - operand2;
  else
    result = operand1 + operand2;

  FLAG_ASSIGN (BIT_N, 0x8000 & result);
  FLAG_ASSIGN (BIT_Z, (result & 0xffff) == 0);
  if (sub)
    FLAG_CVH_WORD_SUB(operand1, operand2, result, BIT_C | BIT_H | BIT_V);
  else
    FLAG_CVH_WORD_ADD(operand1, operand2, result, BIT_C | BIT_H | BIT_V);

  *dest_ptr = result & 0xffff;
  tick(1);
  return(resGO);
}

int
cl_stm8::inst_and(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  result = operand1 & operand2;
  FLAG_NZ (result);

  regs.A = result & 0xff;
  if (prefix && (prefix!=0x90))
    tick(3);
  return(resGO);
}

int
cl_stm8::inst_bccmbcpl(t_mem code, unsigned char prefix)
{
   int ea = fetch2();
   unsigned char dbyte;
   dbyte= get1( ea);

   if (code & 0x01)  { // bccm
      char pos = (code - 0x11) >> 1;
      dbyte = dbyte & (~(1<<pos));
      if (regs.CC & BIT_C) {
         dbyte |= (1<<pos);
      }
   } else { // bcpl
      char pos = (code - 0x10) >> 1;
      dbyte = dbyte ^ (1<<pos);
   }

   store1(ea, dbyte);
   return(resGO);
}
int
cl_stm8::inst_bcp(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  result = operand1 & operand2 & 0xff;
  FLAG_NZ (result);

  if (prefix && (prefix!=0x90))
    tick(3);
  return(resGO);
}

int
cl_stm8::inst_bresbset(t_mem code, unsigned char prefix)
{
  int ea = fetch2();
  unsigned char dbyte;
  dbyte= get1( ea);

  if (code & 0x01) { // bres
	char pos = (code - 0x11) >> 1;
	dbyte = dbyte & (~(1<<pos));
  } else { // bset
 	char pos = (code - 0x10) >> 1;
	dbyte = dbyte | (1<<pos);
  }

  store1(ea, dbyte);
  return(resGO);
}
int
cl_stm8::inst_btjfbtjt(t_mem code, unsigned char prefix)
{
  int ea = fetch2();
  unsigned char dbyte;
  dbyte= get1( ea);
  char reljump = fetch();
  char pos;

  if (code & 0x01) { // btjf
	pos = (code - 0x01) >> 1;
	if(!( dbyte & (1<<pos))) {
		PC += reljump;
		tick(1);
	}
  } else { // btjt
 	pos = (code - 0x00) >> 1;
	if ( dbyte & (1<<pos)) {
		PC += reljump;
		tick(1);
	}
  }

  FLAG_ASSIGN (BIT_C, !!(dbyte & (1<<pos)));

  tick(1);
  return(resGO);
}

int
cl_stm8::inst_call(t_mem code, unsigned char prefix)
{
  t_addr newPC = (PC & 0xff0000ul) + fetchea(code, prefix);
  push2(PC);
  PC = newPC;
  tick(3);
  if (prefix && (prefix!=0x90))
    tick(2);
  return(resGO);
}

int
cl_stm8::inst_clr(t_mem code, unsigned char prefix)
{
  unsigned int opaddr = 0;


  FLAG_SET (BIT_Z);
  FLAG_CLEAR (BIT_N);

  switch(((code & 0xf0) | (prefix << 8)) >> 4) {
    /* clr */
    case 0x004: regs.A = 0; return(resGO);
    case 0x003: opaddr = fetch(); break;
    case 0x725: opaddr = fetch2(); break;
    case 0x007: opaddr = regs.X; break;
    case 0x006: opaddr = regs.X + fetch(); break;
    case 0x724: opaddr = regs.X + fetch2(); break;
    case 0x907: opaddr = regs.Y; break;
    case 0x906: opaddr = regs.Y + fetch(); break;
    case 0x904: opaddr = regs.Y + fetch2(); break;
    case 0x000: opaddr = regs.SP + fetch(); break;
  case 0x923: opaddr = get2(fetch()); tick(3); break; // short indirect
  case 0x723: opaddr = get2(fetch2()); tick(3); break; // long indirect
  case 0x926: opaddr = get2(fetch()) + regs.X; tick(3); break; // short x-indexed indirect
  case 0x726: opaddr = get2(fetch2()) + regs.X; tick(3); break; // long x-indexed indirect
  case 0x916: opaddr = get2(fetch()) + regs.Y; tick(3); break; // short y-indexed indirect
    /* clrw */
    case 0x005: regs.X = 0; return(resGO);
    case 0x905: regs.Y = 0; return(resGO);
    default: return(resHALT);
  }

  store1(opaddr, 0);
  return(resGO);
}

int
cl_stm8::inst_cp(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  result = (operand1 - operand2) & 0xff;

  FLAG_NZ (result);
  FLAG_CVH_BYTE_SUB(operand1, operand2, result, BIT_C | BIT_V);

  if (prefix && (prefix!=0x90))
    tick(3);
  return(resGO);
}

int
cl_stm8::inst_cpw(t_mem code, unsigned char prefix)
{
  long int operand1, operand2, result;
  int reversed = 0;

  tick(1);
  operand1 = prefix == 0x90 ? regs.Y : regs.X;
  operand2 = prefix == 0x90 ? regs.X : regs.Y;

  switch((code & 0xf0) >> 4)
  {
    case 0xa: operand2 = fetch2(); break; // Immediate
    case 0xb: operand2 = get2(fetch()); break; // Short
    case 0xc:
      switch (prefix)
      {
        case 0x00:
        case 0x90: operand2 = get2(fetch2()); break; // Long direct
        case 0x92: operand2 = get2(get2(fetch())); break; // short indirect
      case 0x72: operand2 = get2(get2(fetch2())); tick(3); break; // long indirect
      case 0x91: operand2 = get2(get2(fetch())); operand1 = regs.Y; tick(3); break; // short indirect
        default: return(resHALT);
      }
      break;
    case 0x1: operand2 = get2(regs.SP + fetch()); break; // SP-indexed
    case 0xf: operand1 = get2(operand1); reversed = 1; break; // cpw X|Y, (Y|X)
    case 0xe: operand1 = get2(operand1 + fetch()); reversed = 1; break; // short indexed direct
    case 0xd:
      switch (prefix)
      {
        case 0x00:
        case 0x90: operand1 = get2(operand1 + fetch2()); break; // short indexed direct
      case 0x91: operand1 = get2(regs.Y + get2(fetch())); operand2 = regs.X; tick(3); break; // short y-indexed indirect
        case 0x92: operand1 = get2(operand1 + get2(fetch())); break; // short x-indexed indirect
      case 0x72: operand1 = get2(operand1 + get2(fetch2())); tick(3); break; // long x-indexed indirect
        default: return(resHALT);
      }
      reversed = 1;
      break;
    default: return(resHALT);
  }

  if (!reversed)
    result = (operand1 - operand2) & 0xffff;
  else
    result = (operand2 - operand1) & 0xffff;

  FLAG_ASSIGN (BIT_Z, (result & 0xffff) == 0);
  FLAG_ASSIGN (BIT_N, 0x8000 & result);
  if (!reversed)
    FLAG_CVH_WORD_SUB(operand1, operand2, result, BIT_C | BIT_V);
  else
    FLAG_CVH_WORD_SUB(operand2, operand1, result, BIT_C | BIT_V);

  return(resGO);
}

int
cl_stm8::inst_cpl(t_mem code, unsigned char prefix)
{
  long int operand;
  unsigned int opaddr = 0;

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest(code,prefix);
      operand = get1(opaddr);
   }

   operand ^= 0xffff;

   FLAG_SET (BIT_C);

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      regs.A = operand&0xff;
      FLAG_ASSIGN (BIT_Z, (operand & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & operand);
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      regs.X = operand & 0xffff;
      FLAG_ASSIGN (BIT_Z, (operand & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & operand);
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      regs.Y = operand & 0xffff;
      FLAG_ASSIGN (BIT_Z, (operand & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & operand);
   } else {
      store1(opaddr, operand &0xff);
      FLAG_ASSIGN (BIT_Z, (operand & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & operand);
   }

   if (prefix &&
       (
	(code==0x33) ||
	(code==0x63 && prefix!=0x90)
	)
       )
     tick(3);
   if (code==0x53)
     tick(1);
   return(resGO);
}

int
cl_stm8::inst_dec(t_mem code, unsigned char prefix)
{
  long int operand, resval;
  unsigned int opaddr = 0;

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest(code,prefix);
      operand = get1(opaddr);
   }

   resval = operand - 1;

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      regs.A = resval&0xff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_CVH_BYTE_ADD(operand, 0xff, resval, BIT_V);
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      regs.X = resval & 0xffff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_CVH_WORD_ADD(operand, 0xffff, resval, BIT_V);
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      regs.Y = resval & 0xffff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_CVH_WORD_ADD(operand, 0xffff, resval, BIT_V);
   } else {
      store1(opaddr, resval &0xff);
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_CVH_BYTE_ADD(operand, 0xff, resval, BIT_V);
   }

   if (prefix &&
       (code==0x3a || code==0x6a) &&
       (prefix!=0x90))
     tick(3);
   return(resGO);
}

int
cl_stm8::inst_div(t_mem code, unsigned char prefix)
{
   unsigned int quot, remi;

   FLAG_CLEAR(BIT_N);
   FLAG_CLEAR(BIT_H);
   FLAG_CLEAR(BIT_V);

   if( code == 0x65) { // divw
      if (regs.Y == 0x00) {
         FLAG_SET(BIT_C);
      } else {
         FLAG_CLEAR(BIT_C);
         quot = regs.X / regs.Y;
         remi = regs.X % regs.Y;
         regs.X = quot;
         regs.Y = remi;
         FLAG_ASSIGN (BIT_Z, (quot & 0xffff) == 0);
      }
   } else { //div
      if (regs.A == 0x00) {
         FLAG_SET(BIT_C);
      } else {
         FLAG_CLEAR(BIT_C);
         if (prefix == 0x00) {
            quot = regs.X / regs.A;
            remi = regs.X % regs.A;
            regs.X = quot;
            regs.A = remi;
         } else if (prefix == 0x90) {
            quot = regs.Y / regs.A;
            remi = regs.Y % regs.A;
            regs.Y = quot;
            regs.A = remi;
         } else {
            return (resHALT);
         }
         FLAG_ASSIGN (BIT_Z, (quot & 0xffff) == 0);
      }
   }

   tick(10);
   return(resGO);
}

int
cl_stm8::inst_inc(t_mem code, unsigned char prefix)
{
  long int operand, resval;
  unsigned int opaddr = 0;

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest(code,prefix);
      operand = get1(opaddr);
   }

   resval = operand + 1;

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      regs.A = resval&0xff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_CVH_BYTE_ADD(operand, 0x01, resval, BIT_V);
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      regs.X = resval & 0xffff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_CVH_WORD_ADD(operand, 0x0001, resval, BIT_V);
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      regs.Y = resval & 0xffff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_CVH_WORD_ADD(operand, 0x0001, resval, BIT_V);
   } else {
      store1(opaddr, resval &0xff);
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_CVH_BYTE_ADD(operand, 0x01, resval, BIT_V);
   }

   if (prefix &&
       (prefix!=0x90) &&
       (code==0x3c || code==0x6c))
     tick(3);
   return(resGO);
}

int
cl_stm8::inst_jp(t_mem code, unsigned char prefix)
{
  t_addr newPC = (PC & 0xff0000ul) + fetchea(code, prefix);
  PC = newPC;
  if (prefix &&
      (code==0xec || code==0xdc))
    tick(1);
  if (prefix &&
      (code==0xcc || code==0xdc) &&
      (prefix!=0x90))
    tick(4);
  return(resGO);
}

int
cl_stm8::inst_jr(t_mem code, unsigned char prefix)
{
  bool taken;
  signed char ofs;
  unsigned char bz, bn, bv;

  if (prefix ==0x00) {
    switch ((code>>1) & 7) {
      case 0: // JRT - JRA (20) / JRF (21)
        taken = 1;
        break;
      case 1: // JRUGT (22) / JRULE (23)
        taken = !(regs.CC & (BIT_C | BIT_Z));
        break;
      case 2: // JRUGE (24) / JRULT (25)
        taken = !(regs.CC & BIT_C);
        break;
      case 3: // JRNE (26) / JREQ (27)
        taken = !(regs.CC & BIT_Z);
        break;
      case 4: // JRNV (28) / JRV (29)
        taken = !(regs.CC & BIT_V);
        break;
      case 5: // JRPL (2A) / JRMI (2B)
        taken = !(regs.CC & BIT_N);
        break;
      case 6: // JRSGT (2C) - Z or (N xor V) = 0 / JRSLE (2D) - Z or (N xor V) = 1
        bz = !!(regs.CC & BIT_Z);
        bn = !!(regs.CC & BIT_N);
        bv = !!(regs.CC & BIT_V);
        taken = !(bz | (bn ^ bv));
        break;
      case 7: // JRSGE (2E) - N xor V = 0 / / JRSLT(2F) N xor V = 1
        bn = !!(regs.CC & BIT_N);
        bv = !!(regs.CC & BIT_V);
        taken = !(bn ^ bv);
        break;
      default:
        return(resHALT);
    }
  }
  else if (prefix==0x90) {
    switch ((code>>1) & 7) {
      case 4: // JRNH (28) / JRH (29)
         taken = !(regs.CC & BIT_H);
       break;
      case 6: // JRNM (2C) / JRM (2D)
        taken = !(regs.CC & (BIT_I1|BIT_I0));
        break;
      case 7: // JRIL (2E) / JRIH (2F), no means to test this ???
        taken = 0;
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
  if (taken)
    {
      PC += ofs;
      tick(1);
    }
  return(resGO);
}

int
cl_stm8::inst_lda(t_mem code, unsigned char prefix)
{
  int operand;
  operand = OPERAND(code, prefix);
  FLAG_NZ (operand);
  regs.A = operand;
  if (prefix &&
      (prefix!=0x90))
    tick(3);
  return(resGO);
}

int
cl_stm8::operandw(t_mem code, unsigned char prefix)
{
       if(EA_IMM(code)) {
               return(fetch2());
       } else {
               int addr = fetchea(code,prefix);
               int result = get2(addr);
               return(result);
       }
}

int
cl_stm8::inst_ldxy(t_mem code, unsigned char prefix)
{
  unsigned int operand;
  u16_t *dest_ptr;

  tick(1);

  dest_ptr = (prefix == 0x90) ? &regs.Y : &regs.X;
  if((prefix == 0x00 && code == 0x16) || (prefix == 0x91 && code == 0xce) || (prefix == 0x91 && code == 0xde)) dest_ptr = &regs.Y;

  switch((code & 0xf0) >> 4) {
     case 0xa: operand = fetch2(); break; // Immediate
     case 0xb: operand = get2(fetch()); break; // Short
     case 0xc:
       switch (prefix) {
       case 0x90:
       case 0x00:
         operand = get2(fetch2()); // Long direct
         break;
       case 0x92:
       case 0x91:
         operand = get2(get2(fetch())); // short indirect
         break;
       case 0x72:
         operand = get2(get2(fetch2())); // long indirect
         break;
       default:
         return(resHALT);
       }
       break;
     case 0xf: operand = get2(*dest_ptr); break;
     case 0xe: operand = get2(*dest_ptr + fetch()); break;
     case 0xd:
       switch (prefix) {
       case 0x90:
       case 0x00:
         operand = get2(*dest_ptr + fetch2()); // Long x/y-indexed direct
         break;
       case 0x92:
       case 0x91:
         operand = get2(*dest_ptr + get2(fetch())); // short x/y-indexed indirect
         break;
       case 0x72:
         operand = get2(*dest_ptr + get2(fetch2())); // long x-indexed indirect
         break;
       default:
         return(resHALT);
       }
       break;
     case 0x1: operand = get2(regs.SP + fetch()); break;
     default:  return(resHALT);
  }

  if ((code & 0xf0) != 0x90)
    {
      FLAG_ASSIGN (BIT_Z, (operand & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & operand);
    }

  *dest_ptr = operand;

  if (prefix &&
      (code=0xce || code==0xde))
    tick(3);
  return(resGO);
}

int
cl_stm8::inst_lddst(t_mem code, unsigned char prefix)
{
  unsigned int opaddr;

   opaddr = fetchea(code,prefix);

   FLAG_ASSIGN (BIT_Z, (regs.A & 0xff) == 0);
   FLAG_ASSIGN (BIT_N, 0x80 & regs.A);

   store1(opaddr, regs.A);

   return(resGO);
}

int
cl_stm8::inst_ldxydst(t_mem code, unsigned char prefix)
{
  /* ldw dst, REG */
  unsigned int opaddr, operand;

  tick(1);

  switch ((((code & 0xf0) | (prefix << 8)) >> 4) & 0xfff)
    {
      case 0x00b:
      case 0x00c:
      case 0x92c:
      case 0x72c:
      case 0x90f:
      case 0x90e:
      case 0x90d:
      case 0x91d:
        operand = regs.X;
        break;
      case 0x00f:
      case 0x00e:
      case 0x00d:
      case 0x92d:
      case 0x72d:
      case 0x90b:
      case 0x90c:
      case 0x91c:
        operand = regs.Y;
        break;
      case 0x001:
        switch (code)
          {
            case 0x17:
              operand = regs.Y;
              break;
            case 0x1f:
              operand = regs.X;
              break;
            default:
              return resHALT;
          }
        break;
      default:
        return resHALT;
    }

  switch((code & 0xf0) >> 4) {
    case 0x1: opaddr = regs.SP + fetch(); break;
    case 0xb: opaddr = fetch(); break;
    case 0xc:
      switch (prefix) {
      case 0x00:
      case 0x90:
        opaddr = fetch2(); // long direct
        break;
      case 0x91:
      case 0x92:
        opaddr = get2(fetch()); // short indirect
        break;
      case 0x72:
        opaddr = get2(fetch2()); // long indirect
        break;
      default:
        return (resHALT);
      }
      break;
    case 0xf: opaddr = (prefix == 0x90) ? regs.Y : regs.X; break;
    case 0xe: opaddr = ((prefix == 0x90) ? regs.Y : regs.X) + fetch(); break;
    case 0xd:
      switch (prefix) {
      case 0x90:
        opaddr = regs.Y + fetch2(); // long y-indexed direct
        break;
      case 0x00:
        opaddr = regs.X + fetch2(); // long x-indexed direct
        break;
      case 0x92:
        opaddr = regs.X + get2(fetch()); // short x-indexed indirect
        break;
      case 0x91:
        opaddr = regs.Y + get2(fetch()); // short y-indexed indirect
        break;
      case 0x72:
        opaddr = regs.X + get2(fetch2()); // long x-indexed indirect
        break;
      default:
        return (resHALT);
      }
      break;
    default: return(resHALT);
  }

  FLAG_ASSIGN (BIT_Z, (operand & 0xffff) == 0);
  FLAG_ASSIGN (BIT_N, 0x8000 & operand);

  store2(opaddr, operand);

  if (prefix &&
      (code=0xcf || code==0xdf))
    tick(3);
  return(resGO);
}

int
cl_stm8::inst_neg(t_mem code, unsigned char prefix)
{
  long int operand, resval;
  unsigned int opaddr = 0;

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest(code,prefix);
      operand = get1(opaddr);
   }

   resval = 0 - operand;

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      regs.A = resval&0xff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_ASSIGN (BIT_V, (0x80 == operand));
      FLAG_ASSIGN (BIT_C, 0x100 & resval);
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      regs.X = resval&0xffff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_ASSIGN (BIT_V, (0x8000 == operand));
      FLAG_ASSIGN (BIT_C, 0x10000 & resval);
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      regs.Y = resval&0xffff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_ASSIGN (BIT_V, (0x8000 == operand));
      FLAG_ASSIGN (BIT_C, 0x10000 & resval);
   } else {
      store1(opaddr, resval &0xff);
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_ASSIGN (BIT_V, (0x80 == operand));
      FLAG_ASSIGN (BIT_C, 0x100 & resval);
   }

   if (prefix &&
       (prefix!=0x90) &&
       (code=0x30 || code==0x60))
     tick(3);
   if (code==0x50)
     tick(1);
   return(resGO);
}

int
cl_stm8::inst_or(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  result = (operand1 | operand2) & 0xff;
  FLAG_NZ (result);

  regs.A = result & 0xff;

  if (prefix &&
      (prefix!=0x90))
    tick(3);
  return(resGO);
}

int
cl_stm8::inst_rlc(t_mem code, unsigned char prefix)
{
  long int operand, resval;
  unsigned int opaddr = 0;

   if (((code&0xf0)==0x40) && (prefix == 0x00)) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) && (prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) && (prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest (code, prefix);
      operand = get1 (opaddr);
   }

   resval = operand << 0x1;

   if (regs.CC & BIT_C) {
      resval++;
   }

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      regs.A = resval&0xff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_ASSIGN (BIT_C, (resval & 0x100));
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      regs.X = resval & 0xffff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_ASSIGN (BIT_C, (resval & 0x10000));
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      regs.Y = resval & 0xffff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_ASSIGN (BIT_C, (resval & 0x10000));
   } else {
      store1(opaddr, resval &0xff);
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_ASSIGN (BIT_C, (resval & 0x100));
   }

   if (prefix &&
       (code==0x39 || code==0x69) &&
       (prefix!=0x90))
     tick(3);
   if (code==0x59)
     tick(1);
   return (resGO);
}

int
cl_stm8::inst_rrc(t_mem code, unsigned char prefix)
{
  long int operand, resval;
  unsigned int opaddr = 0;

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest(code,prefix);
      operand = get1(opaddr);
   }

   resval = operand >> 1;

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      if (regs.CC & BIT_C) {      resval |= 0x80;   }
      regs.A = resval&0xff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_ASSIGN (BIT_C, (operand & 0x1));
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      if (regs.CC & BIT_C) {      resval |= 0x8000;   }
      regs.X = resval & 0xffff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_ASSIGN (BIT_C, (operand & 0x1));
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      if (regs.CC & BIT_C) {      resval |= 0x8000;   }
      regs.Y = resval & 0xffff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_ASSIGN (BIT_C, (operand & 0x1));
   } else {
      if (regs.CC & BIT_C) {      resval |= 0x80;   }
      store1(opaddr, resval &0xff);
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_ASSIGN (BIT_C, (operand & 0x1));
   }

   if (prefix &&
       (code==0x36 || code==0x66) &&
       (prefix!=0x90))
     tick(3);
   if (code==0x56)
     tick(1);
   return(resGO);
}

int
cl_stm8::inst_sbc(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;
  int carryin = !!(regs.CC & BIT_C);

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  result = (operand1 - operand2 - carryin) & 0xff;

  FLAG_NZ (result);
  FLAG_CVH_BYTE_SUB(operand1, operand2, result, BIT_C | BIT_V);

  regs.A = result;

  if (prefix &&
      (code==0xc2 || code==0xd2) &&
      (prefix!=0x90))
    tick(3);
  return(resGO);
}

int
cl_stm8::inst_sll(t_mem code, unsigned char prefix)
{
  long int operand, resval;
  unsigned int opaddr = 0;

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest(code,prefix);
      operand = get1(opaddr);
   }

   resval = operand << 0x1;

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      regs.A = resval&0xff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_ASSIGN (BIT_C, (resval & 0x100));
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      regs.X = resval & 0xffff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_ASSIGN (BIT_C, (resval & 0x10000));
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      regs.Y = resval & 0xffff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_ASSIGN (BIT_C, (resval & 0x10000));
   } else {
      store1(opaddr, resval &0xff);
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_ASSIGN (BIT_C, (resval & 0x100));
   }

   if (prefix &&
       (code==0x38 || code==0x68) &&
       (prefix!=0x90))
     tick(3);
   if (code==0x58)
     tick(1);
   return(resGO);
}

int
cl_stm8::inst_sra(t_mem code, unsigned char prefix)
{
  long int operand, resval;
  unsigned int opaddr = 0;

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest(code,prefix);
      operand = get1(opaddr);
   }

   resval = operand >> 1;

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      if (operand & 0x80) {      resval |= 0x80;   }
      regs.A = resval&0xff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_ASSIGN (BIT_C, (operand & 0x1));
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      if (operand & 0x8000) {      resval |= 0x8000;   }
      regs.X = resval & 0xffff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_ASSIGN (BIT_C, (operand & 0x1));
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      if (operand & 0x8000) {      resval |= 0x8000;   }
      regs.Y = resval & 0xffff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_ASSIGN (BIT_C, (operand & 0x1));
   } else {
      if (operand & 0x80) {      resval |= 0x80;   }
      store1(opaddr, resval &0xff);
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_ASSIGN (BIT_C, (operand & 0x1));
   }

   if (prefix &&
       (code==0x37 || code==0x67) &&
       (prefix!=0x90))
     tick(3);
   if (code==0x57)
     tick(1);
   return(resGO);
}

int
cl_stm8::inst_srl(t_mem code, unsigned char prefix)
{
  long int operand, resval;
  unsigned int opaddr = 0;

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      operand = regs.A;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
   } else {
      opaddr = get_dest(code,prefix);
      operand = get1(opaddr);
   }

   resval = operand >> 1;

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      regs.A = resval&0xff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_ASSIGN (BIT_C, (operand & 0x1));
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      regs.X = resval & 0xffff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_ASSIGN (BIT_C, (operand & 0x1));
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      regs.Y = resval & 0xffff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
      FLAG_ASSIGN (BIT_C, (operand & 0x1));
   } else {
      store1(opaddr, resval &0xff);
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
      FLAG_ASSIGN (BIT_C, (operand & 0x1));
   }

   if (prefix &&
       (code==0x34 || code==0x64) &&
       (prefix!=0x90))
     tick(3);
   if (code==0x54)
     tick(1);
   return(resGO);
}

int
cl_stm8::inst_sub(t_mem code, unsigned char prefix)
{
  FLAG_CLEAR(BIT_C);
  if (prefix &&
      (code==0xd0 || code==0xc0) &&
      (prefix!=0x90))
    tick(3);
  return inst_sbc(code, prefix);
}

int
cl_stm8::inst_swap(t_mem code, unsigned char prefix)
{
  long int operand, resval;
  unsigned int opaddr = 0;

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      operand = regs.A;
      resval = (operand << 4) | (operand >> 4);
      regs.A = resval&0xff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      operand = regs.X;
      resval = (operand << 8) | (operand >> 8);
      regs.X = resval&0xffff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      operand = regs.Y;
      resval = (operand << 8) | (operand >> 8);
      regs.Y = resval&0xffff;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
   } else {
      opaddr = get_dest(code,prefix);
      operand = get1(opaddr);
      resval = (operand << 4) | (operand >> 4);
      store1(opaddr, resval &0xff);
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
   }

  if (prefix &&
      (code==0x3e || code==0x6e) &&
      (prefix!=0x90))
    tick(3);
  if (code==0x5e)
    tick(1);
   return(resGO);
}

int
cl_stm8::inst_tnz(t_mem code, unsigned char prefix)
{
  unsigned int resval;
  unsigned int opaddr = 0;

   if (((code&0xf0)==0x40) &&(prefix == 0x00)) {
      resval = regs.A;
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
   } else if (((code&0xf0)==0x50) &&(prefix == 0x00)) {
      resval = regs.X;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
   } else if (((code&0xf0)==0x50) &&(prefix == 0x90)) {
      resval = regs.Y;
      FLAG_ASSIGN (BIT_Z, (resval & 0xffff) == 0);
      FLAG_ASSIGN (BIT_N, 0x8000 & resval);
   } else {
      opaddr = get_dest(code,prefix);
      resval = get1(opaddr);
      FLAG_ASSIGN (BIT_Z, (resval & 0xff) == 0);
      FLAG_ASSIGN (BIT_N, 0x80 & resval);
   }
 
  if (prefix &&
      (code==0x3d || code==0x6d) &&
      (prefix!=0x90))
    tick(3);
  if (code==0x5d)
    tick(1);
   return(resGO);
}

int
cl_stm8::inst_xor(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  result = operand1 ^ operand2;
  FLAG_NZ (result);

  regs.A = result & 0xff;

  if (prefix &&
      (code==0xc8 || code==0xd8) &&
      (prefix!=0x90))
    tick(3);
  return(resGO);
}

/* End of stm8.src/inst.cc */
