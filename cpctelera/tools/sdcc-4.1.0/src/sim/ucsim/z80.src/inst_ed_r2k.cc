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
#include "r2kcl.h"
#include "z80mac.h"


int  cl_r2k::inst_ed_(t_mem code)
{
  unsigned short tw;
  
  switch(code)
    {
  case 0x41:
    regs.aBC = regs.DE;
    break;
    
  case 0x42: // SBC HL,BC
    sbc_HL_wordreg(regs.BC);
    break;
    
  case 0x43: // LD (nnnn),BC
    tw = fetch2();
    store2(tw, regs.BC);
    vc.wr+= 2;
    break;
    
  case 0x44: // NEG
    regs.raf.F &= ~(BIT_ALL);  /* clear these */
    if (regs.raf.A != 0)    regs.raf.F |= BIT_C;
    if (regs.raf.A == 0x80) regs.raf.F |= BIT_P;
    if ((regs.raf.A & 0x0F) != 0) regs.raf.F |= BIT_A;
    regs.raf.A = 0 - regs.raf.A;
    regs.raf.F |= BIT_N; /* not addition */
    if (regs.raf.A == 0)    regs.raf.F |= BIT_Z;
    if (regs.raf.A & 0x80)  regs.raf.F |= BIT_S;
    break;

    case 0x45: return inst_lret(code);
      
  case 0x46: // ipset0
  case 0x56: // ipset1
  case 0x4E: // ipset2
  case 0x5E: // ipset3
    ip = ((ip << 2) & 0xFC);
    ip |= (code >> 3) & 0x03;
    break;
    
  case 0x47: // LD EIR,A
    eir = regs.raf.A;
    break;
  case 0x49:
    regs.aBC = regs.BC;
    break;
    
  case 0x4A: // ADC HL,BC
    adc_HL_wordreg(regs.BC);
    break;

  case 0x4B: // LD BC,(nnnn)
    tw = fetch2();
    regs.BC = get2(tw);
    vc.rd+= 2;
    break;
    
  case 0x4D: // RETI
    ip=get1(regs.SP); regs.SP+=1;
    pop2(PC);
    vc.rd+= 2;
    // TODO: chained-atomic, so set some marker
    break;

    // 0x4E: see 0x46
  case 0x4F:
    iir = regs.raf.A;
    break;
    
  case 0x51:
    regs.aDE = regs.DE;
    break;

  case 0x52: // SBC HL,DE
    sbc_HL_wordreg(regs.DE);
    break;
  case 0x53: // LD (nnnn),DE
    tw = fetch2();
    store2(tw, regs.DE);
    vc.rd+= 2;
    break;
    
  case 0x54: // EX (SP),HL
    tw = get2(regs.SP);
    store2( regs.SP, regs.HL );
    regs.HL = tw;
    vc.rd+= 2;
    vc.wr+= 2;
    break;
    
    // 0x56: see 0x46
  case 0x57:
    regs.raf.A = eir;
    break;
    
  case 0x59: // LD DE', BC
    regs.aDE = regs.BC;
    break;
    
  case 0x5A: // ADC HL,DE
    adc_HL_wordreg(regs.DE);
    break;
    
  case 0x5B: // LD DE,(nnnn)
    tw = fetch2();
    regs.DE = get2(tw);
    vc.rd+= 2;
    break;
    
  case 0x5D: // ipres
    ip = ((ip >> 2) & 0x3F) | ((ip & 0x03) << 6);
    break;
    
    // 0x5E: see 0x46

  case 0x5F: // LD A,IIR
    regs.raf.A = iir;
    break;
    
  case 0x61: // LD HL',DE
    regs.aHL = regs.DE;
    break;
    
  case 0x62: // SBC HL,HL
    sbc_HL_wordreg(regs.HL);
    break;
    
  case 0x63: // LD (nnnn),HL opcode 22 does the same faster
    tw = fetch2();
    store2(tw, regs.HL);
    vc.wr+= 2;
    break;

    case 0x64: // LDP (HL),HL
      {
	u16_t u16= regs.HL;
	t_addr al= ((regs.raf.A & 0xf) << 16) | u16;
	t_addr ah= ((regs.raf.A & 0xf) << 16) | ((u16+1)&0xffff);
	rom->write(al, regs.hl.l);
	rom->write(ah, regs.hl.h);
	vc.wr+= 2;
	break;
      }
      
    case 0x65: // LDP (mn),HL
      {
	u16_t u16= fetch2();
	t_addr al= ((regs.raf.A & 0xf) << 16) | u16;
	t_addr ah= ((regs.raf.A & 0xf) << 16) | ((u16+1)&0xffff);
	rom->write(al, regs.hl.l);
	rom->write(ah, regs.hl.h);
	vc.wr+= 2;
	break;
      }
      
  case 0x67: // LD XPC,A
    mmu.xpc = regs.raf.A;
    break;
    
  case 0x69: // LD HL',BC
    regs.aHL = regs.BC;
    break;
    
  case 0x6A: // ADC HL,HL
    adc_HL_wordreg(regs.HL);
    break;
    
  case 0x6B: // LD HL,(nnnn) opcode 2A does the same faster
    tw = fetch2();
    regs.HL = get2(tw);
    vc.rd+= 2;
    break;

    case 0x6c: // LDP HL,(HL)
      {
	u16_t u16= regs.HL;
	t_addr al= ((regs.raf.A & 0xf) << 16) | u16;
	t_addr ah= ((regs.raf.A & 0xf) << 16) | ((u16+1)&0xffff);
	regs.hl.l= rom->read(al);
	regs.hl.h= rom->read(ah);
	vc.rd+= 2;
	break;
      }
      
    case 0x6d: // LDP HL,(mn)
      {
	u16_t u16= fetch2();
	t_addr al= ((regs.raf.A & 0xf) << 16) | u16;
	t_addr ah= ((regs.raf.A & 0xf) << 16) | ((u16+1)&0xffff);
	regs.hl.l= rom->read(al);
	regs.hl.h= rom->read(ah);
	vc.rd+= 2;
	break;
      }
      
  case 0x72: // SBC HL,SP
    sbc_HL_wordreg(regs.SP);
    break;
  case 0x73: // LD (nnnn),SP
    tw = fetch2();
    store2(tw, regs.SP);
    vc.wr+= 2;
    break;
    
  case 0x76: // PUSH IP
    push1(ip);
    vc.wr+= 2;
    break;
    
  case 0x77: // LD A,XPC
    regs.raf.A = mmu.xpc;
    break;
    
  case 0x7A: // ADC HL,SP
    adc_HL_wordreg(regs.SP);
    break;
  case 0x7B: // LD SP,(nnnn)
    tw = fetch2();
    regs.SP = get2(tw);
    vc.rd+= 2;
    break;
    
  case 0x7D: // LD IY, HL
    regs.IY = regs.HL;
    break;

  case 0x7E:
    ip=get1(regs.SP); regs.SP+=1;
    vc.rd++;
    break;
    
  case 0xA0: // LDI
    // BC - count, sourc=HL, dest=DE.  *DE++ = *HL++, --BC until zero
    regs.raf.F &= ~(BIT_P | BIT_N | BIT_A);  /* clear these */
    store1(regs.DE, get1(regs.HL));
    ++regs.HL;
    ++regs.DE;
    --regs.BC;
    vc.rd++;
    vc.wr++;
    if (regs.BC != 0) regs.raf.F |= BIT_P;
    return(resGO);
    
  case 0xA8: // LDD
    // BC - count, source=HL, dest=DE.  *DE-- = *HL--, --BC until zero
    regs.raf.F &= ~(BIT_P | BIT_N | BIT_A);  /* clear these */
    store1(regs.DE, get1(regs.HL));
    --regs.HL;
    --regs.DE;
    --regs.BC;
    vc.rd++;
    vc.wr++;
    if (regs.BC != 0) regs.raf.F |= BIT_P;
    return(resGO);
    
  case 0xB0: // LDIR
    // BC - count, sourc=HL, dest=DE.  *DE++ = *HL++, --BC until zero
    regs.raf.F &= ~(BIT_P | BIT_N | BIT_A);  /* clear these */
    
    tw = get1(regs.HL);
    store1(regs.DE, tw);
    ++regs.HL;
    ++regs.DE;
    --regs.BC;
    vc.rd++;
    vc.wr++;
    if (regs.BC != 0)
      PC = ins_start;
    return(resGO);
    
  case 0xB8: // LDDR
    // BC - count, source=HL, dest=DE.  *DE-- = *HL--, --BC until zero
    regs.raf.F &= ~(BIT_P | BIT_N | BIT_A);  /* clear these */
    
    tw = get1(regs.HL);
    store1(regs.DE, tw);
    --regs.HL;
    --regs.DE;
    --regs.BC;
    vc.rd++;
    vc.wr++;
    
    if (regs.BC != 0)
      PC = ins_start;
    return(resGO);
    
  case 0xEA: // CALL (HL)
    push2(PC);
    PC = regs.HL;
    vc.wr+= 2;
    return(resGO);
    
  
  default:
    return(resINV_INST);
    }
  
  return(resGO);
}

int  cl_r3ka::inst_ed_(t_mem code)
{
  u8_t  tb;
  
  switch(code)
    {
    case  0x66:  // PUSH SU
      push1(SU);
      vc.wr++;
      return(resGO);
      
    case  0x6E:  // POP  SU
      SU = get1(regs.SP);
      regs.SP++;
      vc.rd++;
      return(resGO);
      
    case  0x6F:  // SETUSR
      SU = ((SU << 2) & 0xFC) | 0x01;
      return(resGO);
      
    case  0x7D:  // SURES
      SU = ((SU >> 2) & 0x3F) | ((SU << 6) & 0xC0);
      return(resGO);
      
    case  0x7F:  // RDMODE
      regs.raf.F &= ~(BIT_C);
      if (SU & 0x01)
        regs.raf.F |= BIT_C;
      return(resGO);
      
    case  0x90:  // LDISR
      // repeat (cnt=BC) { (DE) <= (HL++) }  /* normally has io prefix */
      /* TODO: fix IOI/IOE behavior */
      tb = get1(regs.HL);
      store1( regs.DE, tb );
      regs.HL++;
      regs.BC--;
      vc.rd++;
      vc.wr++;
      if (regs.BC)
        PC = ins_start;
      return(resGO);
      
    case  0x98:  // LDDSR
      /* TODO: fix IOI/IOE behavior */
      // repeat (cnt=BC) { (DE) <= (HL--) }  /* normally has io prefix */
      tb = get1(regs.HL);
      store1( regs.DE, tb );
      regs.HL--;
      regs.BC--;
      vc.rd++;
      vc.wr++;
      if (regs.BC)
        PC = ins_start;
      return(resGO);

    case  0xC0:  // UMA
      // repeat while BC != 0:
      // {CF:DE':(HL)} <= (IX) + [(IY)*DE + DE' + CF];
      // BC--; IX++; IY++; HL++;
      {
        u32_t  tmp;
        
        /* scale a sum for operand pointed to by IY */
        tmp  = get1(regs.IY);
        tmp *= regs.DE;
        tmp += regs.aDE;
        tmp += (regs.raf.F & BIT_C) ? 1 : 0;
        
        /* simple add for operand pointed to by IX */
        tmp += get1(regs.IX);
        
        /* store the result(s) */
        store1( regs.HL, tmp & 0xFF );
        regs.aDE = ((tmp >> 8) & 0xFFFF);
        regs.raf.F &= ~(BIT_C);
        regs.raf.F |= (tmp >> 24) ? BIT_C : 0;
	vc.rd+= 2;
	vc.wr++;
      }
      
      regs.IX++;
      regs.IY++;
      regs.HL++;
      regs.BC--;
      if (regs.BC)
        PC = ins_start;
      return(resGO);
      
    case  0xC8:  // UMS
      // repeat while BC != 0:
      // {CF:DE':(HL)} <= (IX) - [(IY)*DE + DE' + CF];
      // BC--; IX++; IY++; HL++;
      {
        u32_t  tmp;
        
        /* scale a sum for operand pointed to by IY */
        tmp  = get1(regs.IY);
        tmp *= regs.DE;
        tmp += regs.aDE;
        tmp += (regs.raf.F & BIT_C) ? 1 : 0;
        
        /* subtract above from operand pointed to by IX */
        tmp = get1(regs.IX) - tmp;
        
        /* store the result(s) */
        store1( regs.HL, tmp & 0xFF );
        regs.aDE = ((tmp >> 8) & 0xFFFF);
        regs.raf.F &= ~(BIT_C);
        regs.raf.F |= (tmp >> 24) ? BIT_C : 0;
	vc.rd+= 2;
	vc.wr++;
      }
      
      regs.IX++;
      regs.IY++;
      regs.HL++;
      regs.BC--;
      if (regs.BC)
        PC = ins_start;
      return(resGO);

    case  0xD0:  // LSIDR
      /* TODO: fix IOI/IOE behavior */
      // repeat (cnt=BC) { (DE++) <= (HL) }  /* normally has io prefix */
      tb = get1( regs.HL );
      store1( regs.DE, tb );
      regs.DE++;
      regs.BC--;
      vc.rd++;
      vc.wr++;
      if (regs.BC)
        PC = ins_start;
      return(resGO);
      
    case 0xD8:   // LSDDR
      /* TODO: fix IOI/IOE behavior */
      // repeat (cnt=BC) { (DE--) <= (HL) }  /* normally has io prefix */
      tb = get1( regs.HL );
      store1( regs.DE, tb );
      regs.DE--;
      regs.BC--;
      vc.rd++;
      vc.wr++;
      if (regs.BC)
        PC = ins_start;
      return(resGO);
      
    case  0xF0:  // LSIR
      /* TODO: fix IOI/IOE behavior */
      //  repeat (cnt=BC) { (DE++) <= (HL++) }
      tb = get1( regs.HL );
      store1( regs.DE, tb );
      regs.DE++;
      regs.HL++;
      regs.BC--;
      vc.rd++;
      vc.wr++;
      if (regs.BC)
        PC = ins_start;
      return(resGO);
      
    case  0xF8:  // LSDR
      /* TODO: fix IOI/IOE behavior */
      //  repeat (cnt=BC) { (DE--) <= (HL--) }
      tb = get1( regs.HL );
      store1( regs.DE, tb );
      regs.DE--;
      regs.HL--;
      regs.BC--;
      vc.rd++;
      vc.wr++;
      if (regs.BC)
        PC = ins_start;
      return(resGO);
      
    default:
      return cl_r2k::inst_ed_(code);
    }
}

//IDET        system mode violation interrupt if in user mode

