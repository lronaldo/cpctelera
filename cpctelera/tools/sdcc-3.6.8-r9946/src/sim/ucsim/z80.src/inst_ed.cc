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

#include "ddconfig.h"

// local
#include "z80cl.h"
#include "regsz80.h"
#include "z80mac.h"


#define tst_A_bytereg(br) {                                \
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
  
  if (code < 0x40)
    {
      if (type->type != CPU_Z180)
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

#if 0
    case 0x40: // IN B,(C)
      return(resGO);
    case 0x41: // OUT (C),B
      return(resGO);
#endif
    case 0x42: // SBC HL,BC
      sbc_HL_wordreg(regs.BC);
      return(resGO);
    case 0x43: // LD (nnnn),BC
      tw = fetch2();
      store2(tw, regs.BC);
      vc.wr+= 2;
      return(resGO);
    case 0x44: // NEG
      regs.raf.F &= ~(BIT_ALL);  /* clear these */
      if (regs.raf.A != 0)    regs.raf.F |= BIT_C;
      if (regs.raf.A == 0x80) regs.raf.F |= BIT_P;
      if ((regs.raf.A & 0x0F) != 0) regs.raf.F |= BIT_A;
      regs.raf.A -= regs.raf.A;
      regs.raf.F |= BIT_N; /* not addition */
      if (regs.raf.A == 0)    regs.raf.F |= BIT_Z;
      if (regs.raf.A & 0x80)  regs.raf.F |= BIT_S;
      return(resGO);
    case 0x45: // RETN (return from non-maskable interrupt)
      pop2(PC);
      vc.rd+= 2;
      return(resGO);
#if 0
    case 0x46: // IM 0
      /* interrupt device puts opcode on data bus */
      return(resGO);
#endif
    case 0x47: // LD IV,A
      regs.iv = regs.raf.A;
      return(resGO);
      
    case 0x48: // IN C,(C)
      vc.rd++;
      return(resGO);
    case 0x49: // OUT (C),C
      vc.wr++;
      return(resGO);

    case 0x4A: // ADC HL,BC
      adc_HL_wordreg(regs.BC);
      return(resGO);
    case 0x4B: // LD BC,(nnnn)
      tw = fetch2();
      regs.BC = get2(tw);
      vc.rd+= 2;
      return(resGO);
    case 0x4C: // MLT BC
      if(type->type != CPU_Z180)
        return(resINV_INST);
      regs.BC = (unsigned long)(regs.bc.h) * (unsigned long)(regs.bc.l);
      return(resGO);
    case 0x4D: // RETI (return from interrupt)
      pop2(PC);
      vc.rd+= 2;
      return(resGO);
    case 0x4F: // LD R,A
      /* Load "refresh" register(whats that?) */
      return(resGO);

    case 0x50: // IN D,(C)
      vc.rd++;
      return(resGO);
    case 0x51: // OUT (C),D
      vc.wr++;
      return(resGO);

    case 0x52: // SBC HL,DE
      sbc_HL_wordreg(regs.DE);
      return(resGO);
    case 0x53: // LD (nnnn),DE
      tw = fetch2();
      store2(tw, regs.DE);
      vc.wr+= 2;
      return(resGO);
#if 0
    case 0x56: // IM 1
      return(resGO);
#endif
    case 0x57: // LD A,IV
      regs.raf.A = regs.iv;
      return(resGO);
      
    case 0x58: // IN E,(C)
      vc.rd++;
      return(resGO);
    case 0x59: // OUT (C),E
      vc.wr++;
      return(resGO);

    case 0x5A: // ADC HL,DE
      adc_HL_wordreg(regs.DE);
      return(resGO);
    case 0x5B: // LD DE,(nnnn)
      tw = fetch2();
      regs.DE = get2(tw);
      vc.rd+= 2;
      return(resGO);
    case 0x5C: // MLT DE
      if(type->type != CPU_Z180)
        return(resINV_INST);
      regs.DE = (unsigned long)(regs.de.h) * (unsigned long)(regs.de.l);
      return(resGO);
#if 0
    case 0x5E: // IM 2
      return(resGO);
    case 0x5F: // LD A,R
      return(resGO);
    case 0x60: // IN H,(C)
      vc.rd++;
      return(resGO);
    case 0x61: // OUT (C),H
      vc.wr++;
      return(resGO);
#endif
    case 0x62: // SBC HL,HL
      sbc_HL_wordreg(regs.HL);
      return(resGO);
    case 0x63: // LD (nnnn),HL opcode 22 does the same faster
      tw = fetch2();
      store2(tw, regs.HL);
      vc.wr+= 2;
      return(resGO);
    case 0x64:
      if (type->type != CPU_Z180)
        return(resINV_INST);
      ubtmp = fetch();      // TST A,n
      tst_A_bytereg(ubtmp);
      return(resGO);

#if 0
    case 0x67: // RRD
      return(resGO);
#endif
    case 0x68: // IN L,(C)
      vc.rd++;
      return(resGO);
    case 0x69: // OUT (C),L
      vc.wr++;
      return(resGO);

    case 0x6A: // ADC HL,HL
      adc_HL_wordreg(regs.HL);
      return(resGO);
    case 0x6B: // LD HL,(nnnn) opcode 2A does the same faster
      tw = fetch2();
      regs.HL = get2(tw);
      vc.rd+= 2;
      return(resGO);
    case 0x6C: // MLT HL
      if(type->type != CPU_Z180)
        return(resINV_INST);
      regs.HL = (unsigned long)(regs.hl.h) * (unsigned long)(regs.hl.l);
      return(resGO);
#if 0
    case 0x6F: // RLD
      /* rotate 1 bcd digit left between ACC and memory location */
      return(resGO);
#endif

    case 0x70: // IN (C)  set flags only (TSTI)
      vc.rd++;
      return(resGO);
    case 0x71: //  OUT (C),0
      vc.wr++;
      return(resGO);

    case 0x72: // SBC HL,SP
      sbc_HL_wordreg(regs.SP);
      return(resGO);
    case 0x73: // LD (nnnn),SP
      tw = fetch2();
      store2(tw, regs.SP);
      vc.wr+= 2;
      return(resGO);

    case 0x78: // IN A,(C)
      vc.rd++;
      return(resGO);
    case 0x79: // OUT (C),A
      vc.wr++;
      return(resGO);

    case 0x7A: // ADC HL,SP
      adc_HL_wordreg(regs.SP);
      return(resGO);
    case 0x7B: // LD SP,(nnnn)
      tw = fetch2();
      regs.SP = get2(tw);
      vc.rd+= 2;
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
      }
      return(resGO);

    case 0xA2: // INI
      return(resGO);
    case 0xA3: // OUTI
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
      
      return(resGO);

    case 0xAA: // IND
      return(resGO);
    case 0xAB: // OUTD
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
          regs.raf.F &= ~BIT_Z;
        if((regs.raf.A - get1(regs.HL)) & 0x80)
          regs.raf.F |= BIT_S;
        else
          regs.raf.F &= ~BIT_S;
/* fixme: set BIT_A correctly. */
        ++regs.HL;
        --regs.BC;
	vc.rd++;
      } while (regs.BC != 0 && (regs.raf.F & BIT_Z) == 0);
      if(regs.BC != 0)
        regs.raf.F |= BIT_P;

      return(resGO);
#if 0
    case 0xB2: // INIR
      return(resGO);
    case 0xB3: // OTIR
      return(resGO);
#endif
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
      } while (regs.BC != 0);
      return(resGO);
#if 0
    case 0xBA: // INDR
      return(resGO);
    case 0xBB: // OTDR
      return(resGO);
#endif

    default:
      return(resINV_INST);
    }
  
  return(resGO);
}

/******** start ED codes *****************/
int  cl_z80::inst_ed(void)
{
  t_mem code;

  if (fetch(&code))
    return(resBREAKPOINT);
  
  return inst_ed_(code);
}

/* End of z80.src/inst_ed.cc */
