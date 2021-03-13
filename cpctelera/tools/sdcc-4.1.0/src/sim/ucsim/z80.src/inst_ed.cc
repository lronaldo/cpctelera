/*
 * Simulator of microcontrollers (inst_ed.cc)
 *   ED escaped multi-byte opcodes for Z80.
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

// local
#include "z80cl.h"
//#include "regsz80.h"
#include "z80mac.h"


#define tst_A_bytereg(br) {                                    \
   ubtmp = regs.raf.A & (br);                                  \
   regs.raf.F &= ~(BIT_ALL);  /* clear these */                \
   regs.raf.F |= BIT_A;                                        \
   if (ubtmp == 0)    regs.raf.F |= BIT_Z;                     \
   if (ubtmp & 0x80)  regs.raf.F |= BIT_S;                     \
   if (parity(ubtmp)) regs.raf.F |= BIT_P;                     \
}


int  cl_z80::inst_ed_(t_mem code)
{
  unsigned short tw;
  u8_t     ubtmp;

  if (type->type == CPU_Z80N)
    {
      int ret;
      if (inst_z80n(code, &ret))
        return ret;
    }

  if (code < 0x40)
    {
      if (!(type->type & (CPU_Z180 | CPU_EZ80)))
        return resINV_INST;

      switch ( code & 0x07 )
        {
        case 0:  //  IN0
          ubtmp = fetch1( );
          reg_g_store( (code >> 3) & 0x07, in_byte( ubtmp ) );
          return resGO;

        case 1:  // OUT0
          ubtmp = fetch1( );
          out_byte( ubtmp, reg_g_read( (code >> 3) & 0x07 ) );
          return resGO;

        case 4:  // TST
          tst_A_bytereg(reg_g_read( (code >> 3) & 0x07 ));
          return resGO;

        default:
          return resINV_INST;
        }
    }

  switch(code)
    {
    case 0x40: // IN B,(C)
      regs.bc.h= inputs->read((type->type==CPU_Z80N)?regs.BC:regs.bc.l);
      regs.raf.F &= ~(BIT_N | BIT_P);
      if (parity(regs.bc.h)) regs.raf.F |= BIT_P;
      vc.rd++;
      tick(11);
      return(resGO);
      
    case 0x41: // OUT (C),B
      outputs->write((type->type==CPU_Z80N)?regs.BC:regs.bc.l, regs.bc.h);
      vc.wr++;
      tick(11);
      return(resGO);

    case 0x42: // SBC HL,BC
      sbc_HL_wordreg(regs.BC);
      tick(14);
      return(resGO);
      
    case 0x43: // LD (nnnn),BC
      tw = fetch2();
      store2(tw, regs.BC);
      vc.wr+= 2;
      tick(19);
      return(resGO);
      
    case 0x44: // NEG
      regs.raf.F &= ~(BIT_ALL);  /* clear these */
      if (regs.raf.A != 0)    regs.raf.F |= BIT_C;
      if (regs.raf.A == 0x80) regs.raf.F |= BIT_P;
      if ((regs.raf.A & 0x0F) != 0) regs.raf.F |= BIT_A;
      regs.raf.A = 0 - regs.raf.A;
      regs.raf.F |= BIT_N; /* not addition */
      if (regs.raf.A == 0)    regs.raf.F |= BIT_Z;
      if (regs.raf.A & 0x80)  regs.raf.F |= BIT_S;
      tick(7);
      return(resGO);
      
    case 0x45: // RETN (return from non-maskable interrupt)
      pop2(PC);
      vc.rd+= 2;
      tick(13);
      return(resGO);

    case 0x46: // IM 0
      /* interrupt device puts opcode on data bus */
      tick(7);
      return(resGO);

    case 0x47: // LD IV,A
      regs.iv = regs.raf.A;
      tick(8);
      return(resGO);

    case 0x48: // IN C,(C)
      regs.bc.l= inputs->read((type->type==CPU_Z80N)?regs.BC:regs.bc.l);
      regs.raf.F &= ~(BIT_N | BIT_P);
      if (parity(regs.bc.l)) regs.raf.F |= BIT_P;
      vc.rd++;
      tick(11);
      return(resGO);
      
    case 0x49: // OUT (C),C
      outputs->write((type->type==CPU_Z80N)?regs.BC:regs.bc.l, regs.bc.l);
      vc.wr++;
      tick(11);
      return(resGO);

    case 0x4A: // ADC HL,BC
      adc_HL_wordreg(regs.BC);
      tick(14);
      return(resGO);
      
    case 0x4B: // LD BC,(nnnn)
      tw = fetch2();
      regs.BC = get2(tw);
      vc.rd+= 2;
      tick(14);
      return(resGO);
      
    case 0x4C: // MLT BC
      if (!(type->type & (CPU_Z180 | CPU_EZ80)))
        return(resINV_INST);
      regs.BC = (unsigned long)(regs.bc.h) * (unsigned long)(regs.bc.l);
      return(resGO);
      
    case 0x4D: // RETI (return from interrupt)
      pop2(PC);
      vc.rd+= 2;
      tick(13);
      return(resGO);
      
    case 0x4F: // LD R,A
      regs.R= regs.raf.A;
      tick(8);
      return(resGO);

    case 0x50: // IN D,(C)
      regs.de.h= inputs->read((type->type==CPU_Z80N)?regs.BC:regs.bc.l);
      regs.raf.F &= ~(BIT_N | BIT_P);
      if (parity(regs.de.h)) regs.raf.F |= BIT_P;
      vc.rd++;
      tick(11);
      return(resGO);
      
    case 0x51: // OUT (C),D
      outputs->write((type->type==CPU_Z80N)?regs.BC:regs.bc.l, regs.de.h);
      vc.wr++;
      tick(11);
      return(resGO);

    case 0x52: // SBC HL,DE
      sbc_HL_wordreg(regs.DE);
      tick(14);
      return(resGO);
      
    case 0x53: // LD (nnnn),DE
      tw = fetch2();
      store2(tw, regs.DE);
      vc.wr+= 2;
      tick(19);
      return(resGO);

    case 0x56: // IM 1
      tick(7);
      return(resGO);

    case 0x57: // LD A,IV
      regs.raf.A = regs.iv;
      SET_S(regs.iv);
      SET_Z(regs.iv);
      regs.raf.F &= ~(BIT_A | BIT_N | BIT_P);
      if (IFF2) regs.raf.F |= BIT_P;
      tick(8);
      return(resGO);

    case 0x58: // IN E,(C)
      regs.de.l= inputs->read((type->type==CPU_Z80N)?regs.BC:regs.bc.l);
      regs.raf.F &= ~(BIT_N | BIT_P);
      if (parity(regs.de.l)) regs.raf.F |= BIT_P;
      vc.rd++;
      tick(11);
      return(resGO);
      
    case 0x59: // OUT (C),E
      outputs->write((type->type==CPU_Z80N)?regs.BC:regs.bc.l, regs.de.l);
      vc.wr++;
      tick(11);
      return(resGO);

    case 0x5A: // ADC HL,DE
      adc_HL_wordreg(regs.DE);
      tick(14);
      return(resGO);
      
    case 0x5B: // LD DE,(nnnn)
      tw = fetch2();
      regs.DE = get2(tw);
      vc.rd+= 2;
      tick(19);
      return(resGO);
      
    case 0x5C: // MLT DE
      if (!(type->type & (CPU_Z180 | CPU_EZ80)))
        return(resINV_INST);
      regs.DE = (unsigned long)(regs.de.h) * (unsigned long)(regs.de.l);
      return(resGO);

    case 0x5E: // IM 2
      tick(7);
      return(resGO);

    case 0x5F: // LD A,R
      regs.raf.A= regs.R;
      SET_S(regs.R);
      SET_Z(regs.R);
      regs.raf.F &= ~(BIT_A | BIT_N | BIT_P);
      if (IFF2) regs.raf.F |= BIT_P;
      tick(7);
      return(resGO);

    case 0x60: // IN H,(C)
      regs.hl.h= inputs->read((type->type==CPU_Z80N)?regs.BC:regs.bc.l);
      regs.raf.F &= ~(BIT_N | BIT_P);
      if (parity(regs.hl.h)) regs.raf.F |= BIT_P;
      vc.rd++;
      tick(11);
      return(resGO);
      
    case 0x61: // OUT (C),H
      outputs->write((type->type==CPU_Z80N)?regs.BC:regs.bc.l, regs.hl.h);
      vc.wr++;
      tick(11);
      return(resGO);

    case 0x62: // SBC HL,HL
      sbc_HL_wordreg(regs.HL);
      tick(14);
      return(resGO);
      
    case 0x63: // LD (nnnn),HL opcode 22 does the same faster
      tw = fetch2();
      store2(tw, regs.HL);
      vc.wr+= 2;
      tick(19);
      return(resGO);
      
    case 0x64:
      if (!(type->type & (CPU_Z180 | CPU_EZ80)))
        return(resINV_INST);
      ubtmp = fetch();      // TST A,n
      tst_A_bytereg(ubtmp);
      return(resGO);

    case 0x67: // RRD
      ubtmp = get1(regs.HL);
      store1(regs.HL, (ubtmp >> 4) | (regs.raf.A << 4));
      regs.raf.A = (regs.raf.A & 0xf0) | (ubtmp & 0x0f);
      tick(17);
      return(resGO);

    case 0x68: // IN L,(C)
      regs.hl.l= inputs->read((type->type==CPU_Z80N)?regs.BC:regs.bc.l);
      regs.raf.F &= ~(BIT_N | BIT_P);
      if (parity(regs.hl.l)) regs.raf.F |= BIT_P;
      vc.rd++;
      tick(11);
      return(resGO);
      
    case 0x69: // OUT (C),L
      outputs->write((type->type==CPU_Z80N)?regs.BC:regs.bc.l, regs.hl.l);
      vc.wr++;
      tick(11);
      return(resGO);

    case 0x6A: // ADC HL,HL
      adc_HL_wordreg(regs.HL);
      tick(14);
      return(resGO);
      
    case 0x6B: // LD HL,(nnnn) opcode 2A does the same faster
      tw = fetch2();
      regs.HL = get2(tw);
      vc.rd+= 2;
      tick(19);
      return(resGO);
      
    case 0x6C: // MLT HL
      if (!(type->type & (CPU_Z180 | CPU_EZ80)))
        return(resINV_INST);
      regs.HL = (unsigned long)(regs.hl.h) * (unsigned long)(regs.hl.l);
      return(resGO);

    case 0x6F: // RLD
      ubtmp = get1(regs.HL);
      store1(regs.HL, (ubtmp << 4) | (regs.raf.A & 0x0f));
      regs.raf.A = (regs.raf.A & 0xf0) | (ubtmp >> 4);
      tick(17);
      return(resGO);

    case 0x70: // IN (C)  set flags only (TSTI)
      {
        u8_t x= inputs->read((type->type==CPU_Z80N)?regs.BC:regs.bc.l);
        regs.raf.F &= ~(BIT_N | BIT_P);
        if (parity(x)) regs.raf.F |= BIT_P;
        vc.rd++;
	tick(11);
        return(resGO);
      }
      
    case 0x71: //  OUT (C),0
      outputs->write((type->type==CPU_Z80N)?regs.BC:regs.bc.l, 0);
      vc.wr++;
      tick(11);
      return(resGO);

    case 0x72: // SBC HL,SP
      sbc_HL_wordreg(regs.SP);
      tick(14);
      return(resGO);
      
    case 0x73: // LD (nnnn),SP
      tw = fetch2();
      store2(tw, regs.SP);
      vc.wr+= 2;
      tick(19);
      return(resGO);

    case 0x78: // IN A,(C)
      regs.raf.A= inputs->read((type->type==CPU_Z80N)?regs.BC:regs.bc.l);
      regs.raf.F &= ~(BIT_N | BIT_P);
      if (parity(regs.raf.A)) regs.raf.F |= BIT_P;
      vc.rd++;
      tick(11);
      return(resGO);
      
    case 0x79: // OUT (C),A
      outputs->write((type->type==CPU_Z80N)?regs.BC:regs.bc.l, regs.raf.A);
      vc.wr++;
      tick(11);
      return(resGO);

    case 0x7A: // ADC HL,SP
      adc_HL_wordreg(regs.SP);
      tick(14);
      return(resGO);
      
    case 0x7B: // LD SP,(nnnn)
      tw = fetch2();
      regs.SP = get2(tw);
      vc.rd+= 2;
      tick(19);
      return(resGO);

    case 0x7C: // MLT SP
      //if(type != CPU_Z180)
      return(resINV_INST);
      //regs.SP = (unsigned long)(regs.sp.h) * (unsigned long)(regs.sp.l);
      return(resGO);

    case 0xA0: // LDI
      // BC - count, sourc=HL, dest=DE.  *DE++ = *HL++, --BC until zero
      regs.raf.F &= ~(BIT_P | BIT_N | BIT_A);  /* clear these */
      store1(regs.DE, get1(regs.HL));
      ++regs.HL;
      ++regs.DE;
      --regs.BC;
      if (regs.BC != 0) regs.raf.F |= BIT_P;
      tick(15);
      return(resGO);

    case 0xA1: // CPI
      // compare acc with mem(HL), if ACC=0 set Z flag.  Incr HL, decr BC.
      {
        unsigned char tmp;
        tmp = get1(regs.HL);
        cp_bytereg(tmp);
        ++regs.HL;
        --regs.BC;
        if (regs.BC != 0) regs.raf.F |= BIT_P;
	tick(15);
      }
      return(resGO);

    case 0xA2: // INI
      this->store1(regs.HL, inputs->read((type->type==CPU_Z80N)?regs.BC:regs.bc.l));
      vc.rd++;
      vc.wr++;
      if (type->type==CPU_Z80N)
        regs.raf.F |= BIT_N;
      else
        regs.raf.F &= ~(BIT_N);
      regs.HL++;
      regs.bc.h--;
      SET_Z(regs.bc.h);
      tick(16);
      return(resGO);

    case 0xA3: // OUTI
      regs.bc.h--;
      outputs->write((type->type==CPU_Z80N)?regs.BC:regs.bc.h, this->get1(regs.HL));
      vc.wr++;
      vc.rd++;
      SET_Z(regs.bc.h);
      if (type->type==CPU_Z80N)
        regs.raf.F |= BIT_N;
      else
        regs.raf.F &= ~(BIT_N);
      regs.HL++;
      tick(16);
      return(resGO);

    case 0xA8: // LDD
      // BC - count, source=HL, dest=DE.  *DE-- = *HL--, --BC until zero
      regs.raf.F &= ~(BIT_P | BIT_N | BIT_A);  /* clear these */
      store1(regs.DE, get1(regs.HL));
      --regs.HL;
      --regs.DE;
      --regs.BC;
      if (regs.BC != 0) regs.raf.F |= BIT_P;
      vc.rd++;
      vc.wr++;
      tick(15);
      return(resGO);
      
    case 0xA9: // CPD
/* fixme: checkme, compare to other emul. */

      regs.raf.F &= ~(BIT_ALL);  /* clear these */
      if ((regs.raf.A - get1(regs.HL)) == 0) {
        regs.raf.F |= (BIT_Z | BIT_P);
      }
      ++regs.HL;
      --regs.BC;
      if (regs.BC != 0) regs.raf.F |= BIT_P;
      vc.rd++;
      tick(15);
      return(resGO);

    case 0xAA: // IND
      this->store1(regs.HL, inputs->read((type->type==CPU_Z80N)?regs.BC:regs.bc.l));
      vc.rd++;
      vc.wr++;
      if (type->type==CPU_Z80N)
        regs.raf.F |= BIT_N;
      else
        regs.raf.F &= ~(BIT_N);
      regs.HL--;
      regs.bc.h--;
      SET_Z(regs.bc.h);
      tick(15);
      return(resGO);
    case 0xAB: // OUTD
      regs.bc.h--;
      outputs->write((type->type==CPU_Z80N)?regs.BC:regs.bc.l, this->get1(regs.HL));
      vc.rd++;
      vc.wr++;
      if (type->type==CPU_Z80N)
        regs.raf.F |= BIT_N;
      else
        regs.raf.F &= ~(BIT_N);
      regs.HL--;
      SET_Z(regs.bc.h);
      tick(15);
      return(resGO);

    case 0xB0: // LDIR
      // BC - count, sourc=HL, dest=DE.  *DE++ = *HL++, --BC until zero
      regs.raf.F &= ~(BIT_P | BIT_N | BIT_A);  /* clear these */
      do {
        store1(regs.DE, get1(regs.HL));
        ++regs.HL;
        ++regs.DE;
        --regs.BC;
        vc.rd++;
        vc.wr++;
	tick(15);
	if (regs.BC) tick(5);
      } while (regs.BC != 0);
      return(resGO);

    case 0xB1: // CPIR
      // compare acc with mem(HL), if ACC=0 set Z flag.  Incr HL, decr BC.
      regs.raf.F &= ~(BIT_P | BIT_A | BIT_Z | BIT_S);  /* clear these */
      regs.raf.F |= BIT_N;
      do {
        if((regs.raf.A - get1(regs.HL)) == 0)
          regs.raf.F |= BIT_Z;
        else
          regs.raf.F &= ~(BIT_Z);
        if((regs.raf.A - get1(regs.HL)) & 0x80)
          regs.raf.F |= BIT_S;
        else
          regs.raf.F &= ~(BIT_S);
/* fixme: set BIT_A correctly. */
        ++regs.HL;
        --regs.BC;
        vc.rd++;
	tick(15);
	if (regs.BC) tick(5);
      } while (regs.BC != 0 && (regs.raf.F & BIT_Z) == 0);
      if(regs.BC != 0)
        regs.raf.F |= BIT_P;

      return(resGO);

    case 0xB2: // INIR
      do {
        this->store1(regs.HL, inputs->read((type->type==CPU_Z80N)?regs.BC:regs.bc.l));
        vc.rd++;
        vc.wr++;
        if (type->type==CPU_Z80N)
          regs.raf.F |= BIT_N;
        else
          regs.raf.F &= ~(BIT_N);
        regs.HL++;
        regs.bc.h--;
        SET_Z(regs.bc.h);
	tick(15);
	if (regs.BC) tick(5);
       }
      while (regs.BC);
      return(resGO);
      
    case 0xB3: // OTIR
      do {
        regs.bc.h--;
        outputs->write((type->type==CPU_Z80N)?regs.BC:regs.bc.h, this->get1(regs.HL));
        vc.wr++;
        vc.rd++;
        SET_Z(regs.bc.h);
        if (type->type==CPU_Z80N)
          regs.raf.F |= BIT_N;
        else
          regs.raf.F &= ~(BIT_N);
        regs.HL++;
	tick(15);
	if (regs.BC) tick(5);
      }
      while (regs.BC);
      return(resGO);

    case 0xB8: // LDDR
      // BC - count, source=HL, dest=DE.  *DE-- = *HL--, --BC until zero
      regs.raf.F &= ~(BIT_P | BIT_N | BIT_A);  /* clear these */
      do {
        store1(regs.DE, get1(regs.HL));
        --regs.HL;
        --regs.DE;
        --regs.BC;
        vc.rd++;
        vc.wr++;
	tick(15);
	if (regs.BC) tick(5);
      } while (regs.BC != 0);
      return(resGO);
    case 0xB9: // CPDR
      // compare acc with mem(HL), if ACC=0 set Z flag.  Incr HL, decr BC.
      regs.raf.F &= ~(BIT_ALL);  /* clear these */
      do {
        if ((regs.raf.A - get1(regs.HL)) == 0) {
          regs.raf.F |= (BIT_Z | BIT_P);
          break;
        }
        --regs.HL;
        --regs.BC;
        vc.rd++;
	tick(15);
	if (regs.BC) tick(5);
      } while (regs.BC != 0);
      return(resGO);

    case 0xBA: // INDR
      do {
        this->store1(regs.HL, inputs->read((type->type==CPU_Z80N)?regs.BC:regs.bc.l));
        vc.rd++;
        vc.wr++;
        if (type->type==CPU_Z80N)
          regs.raf.F |= BIT_N;
        else
          regs.raf.F &= ~(BIT_N);
        regs.HL--;
        regs.bc.h--;
        SET_Z(regs.bc.h);
	tick(15);
	if (regs.BC) tick(5);
      }
      while (regs.BC);
      return(resGO);
      
    case 0xBB: // OTDR
      do {
        regs.bc.h--;
        outputs->write((type->type==CPU_Z80N)?regs.BC:regs.bc.l, this->get1(regs.HL));
        vc.rd++;
        vc.wr++;
        if (type->type==CPU_Z80N)
          regs.raf.F |= BIT_N;
        else
          regs.raf.F &= ~(BIT_N);
        regs.HL--;
        SET_Z(regs.bc.h);
	tick(15);
	if (regs.BC) tick(5);
      }
      while (regs.BC);
      return(resGO);

    default:
      return(resINV_INST);
    }

  return(resGO);
}

/******** start ED codes *****************/
int  cl_z80::inst_ed(t_mem prefix)
{
  t_mem code;

  if (fetch(&code))
    return(resBREAKPOINT);

  return inst_ed_(code);
}

/* End of z80.src/inst_ed.cc */
