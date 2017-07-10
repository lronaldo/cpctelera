/*
 * Simulator of microcontrollers (inst.cc)
 *
 * some z80 code base from Karl Bongers karl@turbobit.com
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

#include "ddconfig.h"

#include <stdlib.h>

// local
#include "z80cl.h"
#include "regsz80.h"
#include "z80mac.h"


/*
 * No Instruction
 * NOP
 * 0000 0000 0000 0000
 *----------------------------------------------------------------------------
 */

int
cl_z80::inst_nop(t_mem code)
{
  return(resGO);
}

/*
 * Load Instruction
 * LD
 *
 *----------------------------------------------------------------------------
 */

int
cl_z80::inst_ld(t_mem code)
{
  switch(code)
    {
    case 1:  // LD BC,nnnn
      regs.BC = fetch2();
      break;
    case 2: // LD (BC),A
      store1(regs.BC, regs.raf.A);
      vc.wr++;
      break;
    case 6: // LD B,nn
      regs.bc.h = fetch();
      break;
    case 0xa: // LD A,(BC)
      regs.raf.A = get1(regs.BC);
      vc.rd++;
      break;
    case 0x0e: // LD C,nn
      regs.bc.l = fetch();
      break;
    case 0x11: // LD DE,nnnn
      regs.DE = fetch2();
      break;
    case 0x12: // LD (DE),A
      store1(regs.DE, regs.raf.A);
      vc.wr++;
      break;
    case 0x16: // LD D,nn
      regs.de.h = fetch();
      break;
    case 0x1A: // LD A,(DE)
      regs.raf.A = get1(regs.DE);
      vc.rd++;
      break;
    case 0x1E: // LD E,nn
      regs.de.l = fetch();
      break;
    case 0x21: // LD HL,nnnn
      regs.HL = fetch2();
      break;
    case 0x22: // LD (nnnn),HL
      {
	unsigned short tw;
	tw = fetch2();
	store2(tw, regs.HL);
	vc.wr+= 2;
	break;
      }
    case 0x26: // LD H,nn
      regs.hl.h = fetch();
      break;
    case 0x2A: // LD HL,(nnnn)
      {
	unsigned short tw;
	tw = fetch2();
	regs.HL = get2(tw);
	vc.rd+= 2;
	break;
      }
    case 0x2E: // LD L,nn
      regs.hl.l = fetch();
      break;
    case 0x31: // LD SP,nnnn
      regs.SP = fetch2();
      break;
    case 0x32: // LD (nnnn),A
      {
	unsigned short tw;
	tw = fetch2();
	store1(tw, regs.raf.A);
	vc.wr++;
	break;
      }
    case 0x36: // LD (HL),nn
      store1(regs.HL, fetch());
      vc.wr++;
      break;
    case 0x3A: // LD A,(nnnn)
      regs.raf.A = get1(fetch2());
      vc.rd++;
      break;
    case 0x3E: // LD A,nn
      regs.raf.A = fetch();
      break;
    case 0x40: // LD B,B
      break;
    case 0x41: // LD B,C
      regs.bc.h = regs.bc.l;
      break;
    case 0x42: // LD B,D
      regs.bc.h = regs.de.h;
      break;
    case 0x43: // LD B,E
      regs.bc.h = regs.de.l;
      break;
    case 0x44: // LD B,H
      regs.bc.h = regs.hl.h;
      break;
    case 0x45: // LD B,L
      regs.bc.h = regs.hl.l;
      break;
    case 0x46: // LD B,(HL)
      regs.bc.h = get1(regs.HL);
      vc.rd++;
      break;
    case 0x47: // LD B,A
      regs.bc.h = regs.raf.A;
      break;
    case 0x48: // LD C,B
      regs.bc.l = regs.bc.h;
      break;
    case 0x49: // LD C,C
      break;
    case 0x4A: // LD C,D
      regs.bc.l = regs.de.h;
      break;
    case 0x4B: // LD C,E
      regs.bc.l = regs.de.l;
      break;
    case 0x4C: // LD C,H
      regs.bc.l = regs.hl.h;
      break;
    case 0x4D: // LD C,L
      regs.bc.l = regs.hl.l;
      break;
    case 0x4E: // LD C,(HL)
      regs.bc.l = get1(regs.HL);
      vc.rd++;
      break;
    case 0x4F: // LD C,A
      regs.bc.l = regs.raf.A;
      break;
    case 0x50: // LD D,B
      regs.de.h = regs.bc.h;
      break;
    case 0x51: // LD D,C
      regs.de.h = regs.bc.l;
      break;
    case 0x52: // LD D,D
      break;
    case 0x53: // LD D,E
      regs.de.h = regs.de.l;
      break;
    case 0x54: // LD D,H
      regs.de.h = regs.hl.h;
      break;
    case 0x55: // LD D,L
      regs.de.h = regs.hl.l;
      break;
    case 0x56: // LD D,(HL)
      regs.de.h = get1(regs.HL);
      vc.rd++;
      break;
    case 0x57: // LD D,A
      regs.de.h = regs.raf.A;
      break;
    case 0x58: // LD E,B
      regs.de.l = regs.bc.h;
      break;
    case 0x59: // LD E,C
      regs.de.l = regs.bc.l;
      break;
    case 0x5A: // LD E,D
      regs.de.l = regs.de.h;
      break;
    case 0x5B: // LD E,E
      break;
    case 0x5C: // LD E,H
      regs.de.l = regs.hl.h;
      break;
    case 0x5D: // LD E,L
      regs.de.l = regs.hl.l;
      break;
    case 0x5E: // LD E,(HL)
      regs.de.l = get1(regs.HL);
      vc.rd++;
      break;
    case 0x5F: // LD E,A
      regs.de.l = regs.raf.A;
      break;
    case 0x60: // LD H,B
      regs.hl.h = regs.bc.h;
      break;
    case 0x61: // LD H,C
      regs.hl.h = regs.bc.l;
      break;
    case 0x62: // LD H,D
      regs.hl.h = regs.de.h;
      break;
    case 0x63: // LD H,E
      regs.hl.h = regs.de.l;
      break;
    case 0x64: // LD H,H
      regs.hl.h = regs.hl.h;
      break;
    case 0x65: // LD H,L
      regs.hl.h = regs.hl.l;
      break;
    case 0x66: // LD H,(HL)
      regs.hl.h = get1(regs.HL);
      vc.rd++;
      break;
    case 0x67: // LD H,A
      regs.hl.h = regs.raf.A;
      break;
    case 0x68: // LD L,B
      regs.hl.l = regs.bc.h;
      break;
    case 0x69: // LD L,C
      regs.hl.l = regs.bc.l;
      break;
    case 0x6A: // LD L,D
      regs.hl.l = regs.de.h;
      break;
    case 0x6B: // LD L,E
      regs.hl.l = regs.de.l;
      break;
    case 0x6C: // LD L,H
      regs.hl.l = regs.hl.h;
      break;
    case 0x6D: // LD L,L
      break;
    case 0x6E: // LD L,(HL)
      regs.hl.l = get1(regs.HL);
      vc.rd++;
      break;
    case 0x6F: // LD L,A
      regs.hl.l = regs.raf.A;
      break;
    case 0x70: // LD (HL),B
      store1(regs.HL, regs.bc.h);
      vc.wr++;
      break;
    case 0x71: // LD (HL),C
      store1(regs.HL, regs.bc.l);
      vc.wr++;
      break;
    case 0x72: // LD (HL),D
      store1(regs.HL, regs.de.h);
      vc.wr++;
      break;
    case 0x73: // LD (HL),E
      store1(regs.HL, regs.de.l);
      vc.wr++;
      break;
    case 0x74: // LD (HL),H
      store1(regs.HL, regs.hl.h);
      vc.wr++;
      break;
    case 0x75: // LD (HL),L
      store1(regs.HL, regs.hl.l);
      vc.wr++;
      break;
    case 0x76: // HALT
      return(resHALT);

    case 0x77: // LD (HL),A
      store1(regs.HL, regs.raf.A);
      vc.wr++;
      break;
    case 0x78: // LD A,B
      regs.raf.A = regs.bc.h;
      break;
    case 0x79: // LD A,C
      regs.raf.A = regs.bc.l;
      break;
    case 0x7A: // LD A,D
      regs.raf.A = regs.de.h;
      break;
    case 0x7B: // LD A,E
      regs.raf.A = regs.de.l;
      break;
    case 0x7C: // LD A,H
      regs.raf.A = regs.hl.h;
      break;
    case 0x7D: // LD A,L
      regs.raf.A = regs.hl.l;
      break;
    case 0x7E: // LD A,(HL)
      regs.raf.A = get1(regs.HL);
      vc.rd++;
      break;
    case 0x7F: // LD A,A
      break;
    case 0xF9: // LD SP,HL
      regs.SP = regs.HL;
      break;
    default:
      return(resINV_INST);
      break;
    }
  return(resGO);
}

int
cl_z80::inst_inc(t_mem code)
{
  switch(code)
    {
    case 0x03: // INC BC
      ++regs.BC;
      break;
    case 0x04: // INC B
      inc(regs.bc.h);
      break;
    case 0x0C: // INC C
      inc(regs.bc.l);
      break;
    case 0x13: // INC DE
      ++regs.DE;
      break;
    case 0x14: // INC D
      inc(regs.de.h);
      break;
    case 0x1C: // INC E
      inc(regs.de.l);
      break;
    case 0x23: // INC HL
      ++regs.HL;
      break;
    case 0x24: // INC H
      inc(regs.hl.h);
      break;
    case 0x2C: // INC L
      inc(regs.hl.l);
      break;
    case 0x33: // INC SP
      ++regs.SP;
      break;
    case 0x34: // INC (HL)
      {
	unsigned char t=get1(regs.HL);
         inc(t);
         store1(regs.HL, t);
	 vc.rd++;
	 vc.wr++;
	 break;
      }
    case 0x3C: // INC A
      inc(regs.raf.A);
      break;
    default:
      return(resINV_INST);
      break;
    }
  return(resGO);
}

int
cl_z80::inst_dec(t_mem code)
{
  switch(code)
    {
    case 0x05: // DEC B
      dec(regs.bc.h);
      break;
    case 0x0B: // DEC BC
      --regs.BC;
      break;
    case 0x0D: // DEC C
      dec(regs.bc.l);
      break;
    case 0x15: // DEC D
      dec(regs.de.h);
      break;
    case 0x1B: // DEC DE
      --regs.DE;
      break;
    case 0x1D: // DEC E
      dec(regs.de.l);
      break;
    case 0x25: // DEC H
      dec(regs.hl.h);
      break;
    case 0x2B: // DEC HL
      --regs.HL;
      break;
    case 0x2D: // DEC L
      dec(regs.hl.l);
      break;
    case 0x35: // DEC (HL)
      {
	unsigned char t=get1(regs.HL);
	dec(t);
	store1(regs.HL, t);
	vc.rd++;
	vc.wr++;
	break;
      }
    case 0x3B: // DEC SP
      --regs.SP;
      break;
    case 0x3D: // DEC A
      dec(regs.raf.A);
      break;
    default:
      return(resINV_INST);
      break;
    }
  return(resGO);
}

int
cl_z80::inst_rlca(t_mem code)
{
  regs.raf.F &= ~(BIT_A | BIT_N | BIT_C);
  if (regs.raf.A & 0x80) {
     regs.raf.F |= BIT_C;
     regs.raf.A = (regs.raf.A << 1) | 0x01;
   } else
     regs.raf.A = (regs.raf.A << 1);

  return(resGO);
}

int
cl_z80::inst_rrca(t_mem code)
{
  regs.raf.F &= ~(BIT_A | BIT_N | BIT_C);
  if (regs.raf.A & 0x01) {
     regs.raf.F |= BIT_C;
     regs.raf.A = (regs.raf.A >> 1) | 0x80;
   }
   else
     regs.raf.A = (regs.raf.A >> 1);
  return(resGO);
}

int
cl_z80::inst_ex(t_mem code)
{
  /* 0x08 // EX AF,AF' */
  unsigned char tmp;
  u16_t tempw;

  switch (code) {
    case 0x08: // EX AF,AF'
      tmp = regs.ralt_af.aA;
      regs.ralt_af.aA = regs.raf.A;
      regs.raf.A = tmp;

      tmp = regs.ralt_af.aF;
      regs.ralt_af.aF = regs.raf.F;
      regs.raf.F = tmp;
    break;

    case 0xE3: // EX (SP),HL
      tempw = regs.HL;
      regs.HL = get2(regs.SP);
      store2(regs.SP, tempw);
      vc.rd+= 2;
      vc.wr+= 2;
    break;

    case 0xEB: // EX DE,HL
      tempw = regs.DE;
      regs.DE = regs.HL;
      regs.HL = tempw;
    break;

    default:
      return(resINV_INST);
    break;
  }

  return(resGO);
}

int
cl_z80::inst_add(t_mem code)
{
  switch(code)
    {
    case 0x09: // ADD HL,BC
      add_HL_Word(regs.BC);
      break;
    case 0x19: // ADD HL,DE
      add_HL_Word(regs.DE);
      break;
    case 0x29: // ADD HL,HL
      add_HL_Word(regs.HL);
      break;
    case 0x39: // ADD HL,SP
      add_HL_Word(regs.SP);
      break;
      
    case 0x80: // ADD A,B
      add_A_bytereg(regs.bc.h);
      break;
    case 0x81: // ADD A,C
      add_A_bytereg(regs.bc.l);
      break;
    case 0x82: // ADD A,D
      add_A_bytereg(regs.de.h);
      break;
    case 0x83: // ADD A,E
      add_A_bytereg(regs.de.l);
      break;
    case 0x84: // ADD A,H
      add_A_bytereg(regs.hl.h);
      break;
    case 0x85: // ADD A,L
      add_A_bytereg(regs.hl.l);
      break;
      
    case 0x86: // ADD A,(HL)
      {
	unsigned char utmp;
        utmp = get1(regs.HL);
        add_A_bytereg(utmp);
	vc.rd++;
	break;
      }
      
    case 0x87: // ADD A,A
      add_A_bytereg(regs.raf.A);
      break;
      
    case 0xC6: // ADD A,nn
      {
        unsigned char utmp1;
        utmp1 = fetch();
        add_A_bytereg(utmp1);
	break;
      }
      
    default:
      return(resINV_INST);
      break;
    }

  return(resGO);
}

int
cl_z80::inst_djnz(t_mem code)
{
  signed char j;

  // 0x10: DJNZ dd

  j = fetch1();
  if ((--regs.bc.h != 0)) {
    PC += j;
  } else {
  }
  return(resGO);
}

int
cl_z80::inst_rra(t_mem code)
{
  rr_byte(regs.raf.A);
  return(resGO);
}

int
cl_z80::inst_rla(t_mem code)
{
  rl_byte(regs.raf.A);
  return(resGO);
}

int
cl_z80::inst_jr(t_mem code)
{
  signed char j;

  j = fetch1();
  switch(code) {
    case 0x18: // JR dd
      PC += j;
    break;
    case 0x20: // JR NZ,dd
      if (!(regs.raf.F & BIT_Z)) {
        PC += j;
      }
    break;
    case 0x28: // JR Z,dd
      if ((regs.raf.F & BIT_Z)) {
        PC += j;
      }
    break;
    case 0x30: // JR NC,dd
      if (!(regs.raf.F & BIT_C)) {
        PC += j;
      }
    break;
    case 0x38: // JR C,dd
      if ((regs.raf.F & BIT_C)) {
        PC += j;
      }
    break;
    default:
      return(resINV_INST);
    break;
  }
  return(resGO);
}

int
cl_z80::inst_daa(t_mem code)
{
  /************* from MH's z80ops.c:
      unsigned char incr=0, carry=cy;
      if((f&0x10) || (a&0x0f)>9) incr=6;
      if((f&1) || (a>>4)>9) incr|=0x60;
      if(f&2)suba(incr,0);
      else {
         if(a>0x90 && (a&15)>9)incr|=0x60;
         adda(incr,0);
      }
      f=((f|carry)&0xfb);
   ********/
  /* I have not tried to understand this archaic bit of BCD logic(kpb),
     taking the lazy way out for now and just transcribing MH's code.
   */
  unsigned char incr;
  unsigned char N = regs.raf.F & BIT_N; /* save N */
  unsigned char C = regs.raf.F & BIT_C; /* save C */
  if ((regs.raf.F & BIT_A) || ((regs.raf.A & 0x0f) > 9))
       incr = 6;
  else incr = 0;

  if ((regs.raf.F & BIT_C) || ((regs.raf.A & 0xf0) > 0x90) || (regs.raf.A > 0x99))
    incr |= 0x60;

  if (regs.raf.F & BIT_N) {  /* not addition */
    sub_A_bytereg(incr);
  } else {
    add_A_bytereg(incr);
  }
  regs.raf.F &= ~(BIT_P | BIT_N);
  if (parity(regs.raf.A))
    regs.raf.F |= BIT_P;
  regs.raf.F |= N; /* restore N */
  regs.raf.F |= C; /* or with original C */

  return(resGO);
}

int
cl_z80::inst_cpl(t_mem code)
{
  regs.raf.F |= (BIT_A | BIT_N);
  regs.raf.A = ~regs.raf.A;
  return(resGO);
}

int
cl_z80::inst_scf(t_mem code)
{
  /* Set Carry Flag */
  regs.raf.F &= ~(BIT_A | BIT_N);
  regs.raf.F |= BIT_C;
  return(resGO);
}

int
cl_z80::inst_ccf(t_mem code)
{
  /* Complement Carry Flag */
  regs.raf.F &= ~(BIT_A | BIT_N);
  if (regs.raf.F & BIT_C)
    regs.raf.F |= BIT_A;
  regs.raf.F ^= BIT_C;
  return(resGO);
}

int
cl_z80::inst_halt(t_mem code)
{
  return(resHALT);
}

int
cl_z80::inst_adc(t_mem code)
{
  switch(code)
    {
    case 0x88: // ADC A,B
      adc_A_bytereg(regs.bc.h);
      break;
    case 0x89: // ADC A,C
      adc_A_bytereg(regs.bc.l);
      break;
    case 0x8A: // ADC A,D
      adc_A_bytereg(regs.de.h);
      break;
    case 0x8B: // ADC A,E
      adc_A_bytereg(regs.de.l);
      break;
    case 0x8C: // ADC A,H
      adc_A_bytereg(regs.hl.h);
      break;
    case 0x8D: // ADC A,L
      adc_A_bytereg(regs.hl.l);
      break;
    case 0x8E: // ADC A,(HL)
      {
	unsigned char utmp;
        utmp = get1(regs.HL);
        adc_A_bytereg(utmp);
	vc.rd++;
	break;
      }
    case 0x8F: // ADC A,A
      adc_A_bytereg(regs.raf.A);
      break;
      
    case 0xCE: // ADC A,nn
      {
	unsigned char utmp;
        utmp = fetch();
        adc_A_bytereg(utmp);
      }
      break;
      
    default:
      return(resINV_INST);
      break;
    }
  return(resGO);
}

int
cl_z80::inst_sbc(t_mem code)
{
  switch(code)
    {
    case 0x98: // SBC A,B
      sbc_A_bytereg(regs.bc.h);
      break;
    case 0x99: // SBC A,C
      sbc_A_bytereg(regs.bc.l);
      break;
    case 0x9A: // SBC A,D
      sbc_A_bytereg(regs.de.h);
      break;
    case 0x9B: // SBC A,E
      sbc_A_bytereg(regs.de.l);
      break;
    case 0x9C: // SBC A,H
      sbc_A_bytereg(regs.hl.h);
      break;
    case 0x9D: // SBC A,L
      sbc_A_bytereg(regs.hl.l);
      break;
    case 0x9E: // SBC A,(HL)
      {
	unsigned char utmp;
        utmp = get1(regs.HL);
        sbc_A_bytereg(utmp);
	vc.rd++;
	break;
      }
    case 0x9F: // SBC A,A
      sbc_A_bytereg(regs.raf.A);
      break;
    case 0xDE: // SBC A,nn
      {
	unsigned char utmp;
        utmp = fetch();
        sbc_A_bytereg(utmp);
	break;
      }
    default:
      return(resINV_INST);
      break;
    }
  return(resGO);
}

int
cl_z80::inst_and(t_mem code)
{
  switch(code)
    {
    case 0xA0: // AND B
      and_A_bytereg(regs.bc.h);
      break;
    case 0xA1: // AND C
      and_A_bytereg(regs.bc.l);
      break;
    case 0xA2: // AND D
      and_A_bytereg(regs.de.h);
      break;
    case 0xA3: // AND E
      and_A_bytereg(regs.de.l);
      break;
    case 0xA4: // AND H
      and_A_bytereg(regs.hl.h);
      break;
    case 0xA5: // AND L
      and_A_bytereg(regs.hl.l);
      break;
    case 0xA6: // AND (HL)
      {
	unsigned char utmp;
        utmp = get1(regs.HL);
        and_A_bytereg(utmp);
	vc.rd++;
	break;
      }
    case 0xA7: // AND A
      and_A_bytereg(regs.raf.A);
      break;
    case 0xE6: // AND nn
      and_A_bytereg(fetch());
      break;
      
    default:
      return(resINV_INST);
      break;
    }
  return(resGO);
}

int
cl_z80::inst_xor(t_mem code)
{
  switch(code)
    {
    case 0xA8: // XOR B
      xor_A_bytereg(regs.bc.h);
      break;
    case 0xA9: // XOR C
      xor_A_bytereg(regs.bc.l);
      break;
    case 0xAA: // XOR D
      xor_A_bytereg(regs.de.h);
      break;
    case 0xAB: // XOR E
      xor_A_bytereg(regs.de.l);
      break;
    case 0xAC: // XOR H
      xor_A_bytereg(regs.hl.h);
      break;
    case 0xAD: // XOR L
      xor_A_bytereg(regs.hl.l);
      break;
    case 0xAE: // XOR (HL)
      {
	unsigned char utmp;
        utmp = get1(regs.HL);
        xor_A_bytereg(utmp);
	vc.rd++;
	break;
      }
    case 0xAF: // XOR A
      xor_A_bytereg(regs.raf.A);
      break;
    case 0xEE: // XOR nn
      xor_A_bytereg(fetch());
      break;
      
    default:
      return(resINV_INST);
      break;
    }
  return(resGO);
}

int
cl_z80::inst_or(t_mem code)
{
  switch(code)
    {
    case 0xB0: // OR B
      or_A_bytereg(regs.bc.h);
      break;
    case 0xB1: // OR C
      or_A_bytereg(regs.bc.l);
      break;
    case 0xB2: // OR D
      or_A_bytereg(regs.de.h);
      break;
    case 0xB3: // OR E
      or_A_bytereg(regs.de.l);
      break;
    case 0xB4: // OR H
      or_A_bytereg(regs.hl.h);
      break;
    case 0xB5: // OR L
      or_A_bytereg(regs.hl.l);
      break;
    case 0xB6: // OR (HL)
      {
	unsigned char utmp;
        utmp = get1(regs.HL);
        or_A_bytereg(utmp);
	vc.rd++;
	break;
      }
    case 0xB7: // OR A
      or_A_bytereg(regs.raf.A);
      break;
    case 0xF6: // OR nn
      or_A_bytereg(fetch());
      break;
    default:
      return(resINV_INST);
      break;
    }
  return(resGO);
}

int
cl_z80::inst_cp(t_mem code)
{
  /* Compare with Accumulator - subtract and test, leave A unchanged */
  switch(code)
    {
    case 0xB8: // CP B
      cp_bytereg(regs.bc.h);
      break;
    case 0xB9: // CP C
      cp_bytereg(regs.bc.l);
      break;
    case 0xBA: // CP D
      cp_bytereg(regs.de.h);
      break;
    case 0xBB: // CP E
      cp_bytereg(regs.de.l);
      break;
    case 0xBC: // CP H
      cp_bytereg(regs.hl.h);
      break;
    case 0xBD: // CP L
      cp_bytereg(regs.hl.l);
      break;
    case 0xBE: // CP (HL)
      {
	unsigned char utmp;
        utmp = get1(regs.HL);
        cp_bytereg(utmp);
	vc.rd++;
	break;
      }
    case 0xBF: // CP A
      cp_bytereg(regs.raf.A);
      break;
    case 0xFE: // CP nn
      {
	unsigned char utmp;
        utmp = fetch();
        cp_bytereg(utmp);
	break;
      }
    default:
      return(resINV_INST);
      break;
    }
  return(resGO);
}

int
cl_z80::inst_rst(t_mem code)
{
  switch(code)
    {
    case 0xC7: // RST 0
      push2(PC+2);
      PC = 0x0;
      vc.wr+= 2;
      break;
    case 0xCF: // RST 8
      //PC = 0x08;
      switch (regs.raf.A)
	{
	case 0:
	  return(resBREAKPOINT);
	  //          ::exit(0);
	  break;
	  
	case 1:
	  //printf("PUTCHAR-----> %xH\n", regs.hl.l);
	  putchar(regs.hl.l);
	  fflush(stdout);
	  break;
	}
      break;
    case 0xD7: // RST 10H
      push2(PC+2);
      PC = 0x10;
      vc.wr+= 2;
      break;
    case 0xDF: // RST 18H
      push2(PC+2);
      PC = 0x18;
      vc.wr+= 2;
      break;
    case 0xE7: // RST 20H
      push2(PC+2);
      PC = 0x20;
      vc.wr+= 2;
      break;
    case 0xEF: // RST 28H
      push2(PC+2);
      PC = 0x28;
      vc.wr+= 2;
      break;
    case 0xF7: // RST 30H
      push2(PC+2);
      PC = 0x30;
      vc.wr+= 2;
      break;
    case 0xFF: // RST 38H
      push2(PC+2);
      PC = 0x38;
      vc.wr+= 2;
      break;
    default:
      return(resINV_INST);
      break;
    }
  return(resGO);
}

int
cl_z80::inst_ret(t_mem code)
{
  switch(code)
    {
    case 0xC0: // RET NZ
      if (!(regs.raf.F & BIT_Z)) {
        pop2(PC);
	vc.rd+= 2;
      }
      break;
    case 0xC8: // RET Z
      if ((regs.raf.F & BIT_Z)) {
        pop2(PC);
	vc.rd+= 2;
      }
      break;
    case 0xC9: // RET
      pop2(PC);
      vc.rd+= 2;
      break;
    case 0xD0: // RET NC
      if (!(regs.raf.F & BIT_C)) {
        pop2(PC);
	vc.rd+= 2;
      }
      break;
    case 0xD8: // RET C
      if ((regs.raf.F & BIT_C)) {
        pop2(PC);
	vc.rd+= 2;
      }
      break;
    case 0xE0: // RET PO
      if (!(regs.raf.F & BIT_P)) {
        pop2(PC);
	vc.rd+= 2;
      }
      break;
    case 0xE8: // RET PE
      if ((regs.raf.F & BIT_P)) {
        pop2(PC);
	vc.rd+= 2;
      }
      break;
    case 0xF0: // RET P
      if (!(regs.raf.F & BIT_S)) {
        pop2(PC);
	vc.rd+= 2;
      }
      break;
    case 0xF8: // RET M
      if ((regs.raf.F & BIT_S)) {
        pop2(PC);
	vc.rd+= 2;
      }
      break;
    default:
      return(resINV_INST);
      break;
    }
  return(resGO);
}

int
cl_z80::inst_call(t_mem code)
{
  switch(code)
    {
    case 0xC4: // CALL NZ,nnnn
      if (!(regs.raf.F & BIT_Z)) {
        push2(PC+2);
        PC = fetch2();
	vc.wr+= 2;
      } else {
        fetch2();
      }
      break;
    case 0xCC: // CALL Z,nnnn
      if (regs.raf.F & BIT_Z) {
        push2(PC+2);
        PC = fetch2();
	vc.wr+= 2;
      } else {
        fetch2();
      }
      break;
    case 0xCD: // CALL nnnn
      push2(PC+2);
      PC = fetch2();
      vc.wr+= 2;
      break;
    case 0xD4: // CALL NC,nnnn
      if (!(regs.raf.F & BIT_C)) {
        push2(PC+2);
        PC = fetch2();
	vc.wr+= 2;
      } else {
        fetch2();
      }
      break;
    case 0xDC: // CALL C,nnnn
      if (regs.raf.F & BIT_C) {
        push2(PC+2);
        PC = fetch2();
	vc.wr+= 2;
      } else {
        fetch2();
      }
      break;
    case 0xE4: // CALL PO,nnnn
      if (!(regs.raf.F & BIT_P)) {
        push2(PC+2);
        PC = fetch2();
	vc.wr+= 2;
      } else {
        fetch2();
      }
      break;
    case 0xEC: // CALL PE,nnnn
      if (regs.raf.F & BIT_P) {
        push2(PC+2);
        PC = fetch2();
	vc.wr+= 2;
      } else {
        fetch2();
      }
      break;
    case 0xF4: // CALL P,nnnn
      if (!(regs.raf.F & BIT_S)) {
        push2(PC+2);
        PC = fetch2();
	vc.wr+= 2;
      } else {
        fetch2();
      }
      break;
    case 0xFC: // CALL M,nnnn
      if (regs.raf.F & BIT_S) {
        push2(PC+2);
        PC = fetch2();
	vc.wr+= 2;
      } else {
        fetch2();
      }
      break;
    default:
      return(resINV_INST);
      break;
    }
  
  return(resGO);
}

int
cl_z80::inst_out(t_mem code)
{
  t_addr a= fetch();
  outputs->write(a, regs.raf.A);
  return(resGO);
}

int
cl_z80::inst_push(t_mem code)
{
  switch(code)
    {
    case 0xC5: // PUSH BC
      push2(regs.BC);
      vc.wr+= 2;
      break;
    case 0xD5: // PUSH DE
      push2(regs.DE);
      vc.wr+= 2;
      break;
    case 0xE5: // PUSH HL
      push2(regs.HL);
      vc.wr+= 2;
      break;
    case 0xF5: // PUSH AF
      push1(regs.raf.A);
      push1(regs.raf.F);
      vc.wr+= 2;
      break;
    default:
      return(resINV_INST);
      break;
    }
  return(resGO);
}

int
cl_z80::inst_exx(t_mem code)
{
  /* case 0xD9: // EXX - swap BC,DE,HL with alternates */
  u16_t tempw;

  tempw = regs.aBC;
  regs.BC = regs.aBC;
  regs.aBC = tempw;

  tempw = regs.aDE;
  regs.DE = regs.aDE;
  regs.aDE = tempw;

  tempw = regs.aDE;
  regs.DE = regs.aDE;
  regs.aDE = tempw;

  return(resGO);
}

int
cl_z80::inst_in(t_mem code)
{
  t_addr a= fetch();
  regs.raf.A= inputs->read(a);
  return(resGO);
}

int
cl_z80::inst_sub(t_mem code)
{
  switch(code)
    {
    case 0x90: // SUB B
      sub_A_bytereg(regs.bc.h);
      break;
    case 0x91: // SUB C
      sub_A_bytereg(regs.bc.l);
      break;
    case 0x92: // SUB D
      sub_A_bytereg(regs.de.h);
      break;
    case 0x93: // SUB E
      sub_A_bytereg(regs.de.l);
      break;
    case 0x94: // SUB H
      sub_A_bytereg(regs.hl.h);
      break;
    case 0x95: // SUB L
      sub_A_bytereg(regs.hl.l);
      break;
    case 0x96: // SUB (HL)
      {
	unsigned char tmp1;
        tmp1 = get1(regs.HL);
        sub_A_bytereg(tmp1);
	vc.rd++;
      }
      break;
    case 0x97: // SUB A
      sub_A_bytereg(regs.raf.A);
      break;
    case 0xD6: // SUB nn
      {
	unsigned char tmp1;
        tmp1 = fetch();
        sub_A_bytereg(tmp1);
      }
      break;
    default:
      return(resINV_INST);
      break;
    }
  return(resGO);
}

int
cl_z80::inst_pop(t_mem code)
{
  switch (code)
    {
    case 0xC1: // POP BC
      regs.BC = get2(regs.SP);
      regs.SP+=2;
      vc.rd+= 2;
      break;
    case 0xD1: // POP DE
      regs.DE = get2(regs.SP);
      regs.SP+=2;
      vc.rd+= 2;
      break;
    case 0xE1: // POP HL
      regs.HL = get2(regs.SP);
      regs.SP+=2;
      vc.rd+= 2;
      break;
    case 0xF1: // POP AF
      regs.raf.F = get1(regs.SP++);
      regs.raf.A = get1(regs.SP++);
      vc.rd+= 2;
      break;
    default:
      return(resINV_INST);
      break;
    }
  return(resGO);
}

int
cl_z80::inst_jp(t_mem code)
{
  switch (code)
    {
    case 0xC2: // JP NZ,nnnn
      if (!(regs.raf.F & BIT_Z)) {
        PC = fetch2();
      } else {
        fetch2();
      }
      break;
      
    case 0xC3: // JP nnnn
      PC = fetch2();
      break;
      
    case 0xCA: // JP Z,nnnn
      if (regs.raf.F & BIT_Z) {
        PC = fetch2();
      } else {
        fetch2();
      }
      break;
      
    case 0xD2: // JP NC,nnnn
      if (!(regs.raf.F & BIT_C)) {
        PC = fetch2();
      } else {
        fetch2();
      }
      break;
      
    case 0xDA: // JP C,nnnn
      if (regs.raf.F & BIT_C) {
        PC = fetch2();
      } else {
        fetch2();
      }
      break;
      
    case 0xE2: // JP PO,nnnn
      if (!(regs.raf.F & BIT_P)) {
        PC = fetch2();
      } else {
        fetch2();
      }
      break;
      
    case 0xE9: // JP (HL)
      PC = regs.HL;
      break;
      
    case 0xEA: // JP PE,nnnn
      if (regs.raf.F & BIT_P) {
        PC = fetch2();
      } else {
        fetch2();
      }
      break;
      
    case 0xF2: // JP P,nnnn (positive)
      if (!(regs.raf.F & BIT_S)) {
        PC = fetch2();
      } else {
        fetch2();
      }
      break;
      
    case 0xFA: // JP M,nnnn (sign negative)
      if (regs.raf.F & BIT_S) {
        PC = fetch2();
      } else {
        fetch2();
      }
      break;
    default:
      return(resINV_INST);
      break;
    }
  return(resGO);
}

int
cl_z80::inst_di(t_mem code)
{
  /* disable interrupts */
  return(resGO);
}

int
cl_z80::inst_ei(t_mem code)
{
  /* enable interrupts */
  return(resGO);
}

/* End of z80.src/inst.cc */
