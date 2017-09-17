/*
 * Simulator of microcontrollers (inst_xd.cc)
 *  dd or fd escaped multi-byte opcodes.
 *
 *   This module gets pulled in and pre-processed to create
 *   two modules.  DD prefixed opcodes reference
 *   IX register, while FD prefixes reference IY register.
 *   See inst_ddcb.cc and inst_fdcb.cc
 *
 * Copyright (C) 1999,2002 Drotos Daniel, Talker Bt.
 * some z80 coding from Karl Bongers karl@turbobit.com
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

int
cl_z80::inst_Xd_ld(t_mem code)
{
  unsigned short tw;

  switch (code) {
    case 0x21: // LD IX,nnnn
      regs_IX_OR_IY = fetch2();
    return(resGO);
    case 0x22: // LD (nnnn),IX
      tw = fetch2();
      store2(tw, regs_IX_OR_IY);
      vc.wr+= 2;
    return(resGO);
    case 0x26: // LD HX,nn
      regs_iX_h = fetch1();
    return(resGO);
    case 0x2A: // LD IX,(nnnn)
      tw = fetch2();
      regs_IX_OR_IY = get2(tw);
      vc.rd+= 2;
    return(resGO);
    case 0x2E: // LD LX,nn
      regs_iX_l = fetch1();
    return(resGO);
    case 0x36: // LD (IX+dd),nn
      tw = add_u16_disp(regs_IX_OR_IY, fetch());
      store1(tw, fetch());
      vc.wr++;
    return(resGO);
    case 0x44: // LD B,HX
      regs.bc.h = regs_iX_h;
    return(resGO);
    case 0x45: // LD B,LX
      regs.bc.h = regs_iX_l;
    return(resGO);
    case 0x46: // LD B,(IX+dd)
      regs.bc.h = get1(add_u16_disp(regs_IX_OR_IY,fetch()));
      vc.rd++;
    return(resGO);
    case 0x4C: // LD C,HX
      regs.bc.l = regs_iX_h;
    return(resGO);
    case 0x4D: // LD C,LX
      regs.bc.l = regs_iX_l;
    return(resGO);
    case 0x4E: // LD C,(IX+dd)
      regs.bc.l = get1(add_u16_disp(regs_IX_OR_IY,fetch()));
      vc.rd++;
    return(resGO);
    case 0x54: // LD D,HX
      regs.de.h = regs_iX_h;
    return(resGO);
    case 0x55: // LD D,LX
      regs.de.h = regs_iX_l;
    return(resGO);
    case 0x56: // LD D,(IX+dd)
      regs.de.h = get1(add_u16_disp(regs_IX_OR_IY,fetch()));
      vc.rd++;
    return(resGO);
    case 0x5C: // LD E,H
      regs.de.l = regs.hl.h;
    return(resGO);
    case 0x5D: // LD E,L
      regs.de.l = regs.hl.l;
    return(resGO);
    case 0x5E: // LD E,(IX+dd)
      regs.de.l = get1(add_u16_disp(regs_IX_OR_IY,fetch()));
      vc.rd++;
    return(resGO);
    case 0x60: // LD HX,B
      regs_iX_h = regs.bc.h;
    return(resGO);
    case 0x61: // LD HX,C
      regs_iX_h = regs.bc.l;
    return(resGO);
    case 0x62: // LD HX,D
      regs_iX_h = regs.de.h;
    return(resGO);
    case 0x63: // LD HX,E
      regs_iX_h = regs.de.l;
    return(resGO);
    case 0x64: // LD HX,HX
    return(resGO);
    case 0x65: // LD HX,LX
      regs_iX_h = regs_iX_l;
    return(resGO);
    case 0x66: // LD H,(IX+dd)
      regs.hl.h = get1(add_u16_disp(regs_IX_OR_IY,fetch()));
      vc.rd++;
    return(resGO);
    case 0x67: // LD HX,A
      regs_iX_h = regs.raf.A;
    return(resGO);
    case 0x68: // LD LX,B
      regs_iX_l = regs.bc.h;
    return(resGO);
    case 0x69: // LD LX,C
      regs_iX_l = regs.bc.l;
    return(resGO);
    case 0x6A: // LD LX,D
      regs_iX_l = regs.de.h;
    return(resGO);
    case 0x6B: // LD LX,E
      regs_iX_l = regs.de.l;
    return(resGO);
    case 0x6C: // LD LX,HX
      regs_iX_l = regs.hl.h;
    return(resGO);
    case 0x6D: // LD LX,LX
    return(resGO);
    case 0x6E: // LD L,(IX+dd)
      regs.hl.l = get1(add_u16_disp(regs_IX_OR_IY,fetch()));
      vc.rd++;
    return(resGO);
    case 0x6F: // LD LX,A
      regs_iX_l = regs.raf.A;
    return(resGO);
    case 0x70: // LD (IX+dd),B
      store1(add_u16_disp(regs_IX_OR_IY,fetch()), regs.bc.h);
      vc.wr++;
    return(resGO);
    case 0x71: // LD (IX+dd),C
      store1(add_u16_disp(regs_IX_OR_IY,fetch()), regs.bc.l);
      vc.wr++;
    return(resGO);
    case 0x72: // LD (IX+dd),D
      store1(add_u16_disp(regs_IX_OR_IY,fetch()), regs.de.h);
      vc.wr++;
    return(resGO);
    case 0x73: // LD (IX+dd),E
      store1(add_u16_disp(regs_IX_OR_IY,fetch()), regs.de.l);
      vc.wr++;
    return(resGO);
    case 0x74: // LD (IX+dd),H
      store1(add_u16_disp(regs_IX_OR_IY,fetch()), regs.hl.h);
      vc.wr++;
    return(resGO);
    case 0x75: // LD (IX+dd),L
      store1(add_u16_disp(regs_IX_OR_IY,fetch()), regs.hl.l);
      vc.wr++;
    return(resGO);
    case 0x77: // LD (IX+dd),A
      store1(add_u16_disp(regs_IX_OR_IY,fetch()), regs.raf.A);
      vc.wr++;
    return(resGO);
    case 0x7C: // LD A,HX
      regs.raf.A = regs_iX_h;
    return(resGO);
    case 0x7D: // LD A,LX
      regs.raf.A = regs_iX_l;
    return(resGO);
    case 0x7E: // LD A,(IX+dd)
      regs.raf.A = get1(add_u16_disp(regs_IX_OR_IY,fetch()));
      vc.rd++;
    return(resGO);
    case 0xF9: // LD SP,IX
      regs.SP = regs_IX_OR_IY;
    return(resGO);
  }
  return(resINV_INST);
}

int
cl_z80::inst_Xd_add(t_mem code)
{
  switch (code) {
    case 0x09: // ADD IX,BC
      add_IX_Word(regs.BC);
      return(resGO);
    case 0x19: // ADD IX,DE
      add_IX_Word(regs.DE);
      return(resGO);
    case 0x29: // ADD IX,IX
      add_IX_Word(regs_IX_OR_IY);
      return(resGO);
    case 0x39: // ADD IX,SP
      add_IX_Word(regs.SP);
    return(resGO);
    case 0x84: // ADD A,HX
      add_A_bytereg(regs_iX_h);
      return(resGO);
    case 0x85: // ADD A,LX
      add_A_bytereg(regs_iX_l);
      return(resGO);
    case 0x86: // ADD A,(IX+dd)
      { unsigned char ourtmp;
        t_addr addr;
        addr = add_u16_disp(regs_IX_OR_IY, fetch());
        ourtmp = get1(addr);
        add_A_bytereg(ourtmp);
	vc.rd++;
      }
      return(resGO);
  }
  return(resINV_INST);
}

int
cl_z80::inst_Xd_push(t_mem code)
{
  switch (code) {
    case 0xe5: // PUSH IX
      push2(regs_IX_OR_IY);
      vc.wr+= 2;
    return(resGO);
  }
  return(resINV_INST);
}

int
cl_z80::inst_Xd_inc(t_mem code)
{
  switch(code) {
    case 0x23: // INC IX
      ++regs_IX_OR_IY;
    break;
    case 0x24: // INC HX
      inc(regs_iX_h);
    break;
    case 0x2C: // INC LX
      inc(regs_iX_l);
    break;
    case 0x34: // INC (IX+dd)
      {
        t_addr addr;
        unsigned char tmp;
        addr = add_u16_disp(regs_IX_OR_IY,fetch());
        tmp = get1(addr);
        inc(tmp);
        store1(addr, tmp);
	vc.rd++;
	vc.wr++;
      }
    break;
    default:
      return(resINV_INST);
    break;
  }
  return(resGO);
}

int
cl_z80::inst_Xd_dec(t_mem code)
{
  switch(code) {
    case 0x25: // DEC HX
      dec(regs_iX_h);
    break;
    case 0x2B: // DEC IX
      --regs_IX_OR_IY;
    break;
    case 0x2D: // DEC LX
      dec(regs_iX_l);
    break;
    case 0x35: // DEC (IX+dd)
      {
        t_addr addr;
        unsigned char tmp;
        addr = add_u16_disp(regs_IX_OR_IY,fetch());
        tmp = get1(addr);
        dec(tmp);
        store1(addr, tmp);
	vc.rd++;
	vc.wr++;
      }
    break;
    default:
      return(resINV_INST);
    break;
  }
  return(resGO);
}


/* need ADC, SUB, SBC, AND, XOR, OR, CP */
int
cl_z80::inst_Xd_misc(t_mem code)
{
  switch(code) {
    case 0x8C: // ADC A,HX
      adc_A_bytereg(regs_iX_h);
    return(resGO);
    case 0x8D: // ADC A,LX
      adc_A_bytereg(regs_iX_l);
    return(resGO);
    case 0x8E: // ADC A,(IX+dd)
      { unsigned char utmp;
        t_addr addr;
        addr = add_u16_disp(regs_IX_OR_IY, fetch());
        utmp = get1(addr);
        adc_A_bytereg(utmp);
	vc.rd++;
      }
    return(resGO);

    case 0x94: // SUB HX
      sub_A_bytereg(regs_iX_h);
    return(resGO);
    case 0x95: // SUB LX
      sub_A_bytereg(regs_iX_l);
    return(resGO);
    case 0x96: // SUB (IX+dd)
      { unsigned char tmp1;
        tmp1 = get1(add_u16_disp(regs_IX_OR_IY, fetch()));
        sub_A_bytereg(tmp1);
	vc.rd++;
      }
    return(resGO);

    case 0x9C: // SBC A,HX
      sbc_A_bytereg(regs_iX_h);
    return(resGO);
    case 0x9D: // SBC A,LX
      sbc_A_bytereg(regs_iX_l);
    return(resGO);
    case 0x9E: // SBC A,(IX+dd)
      { unsigned char utmp;
        utmp = get1(add_u16_disp(regs_IX_OR_IY, fetch()));
        sbc_A_bytereg(utmp);
	vc.rd++;
      }
    return(resGO);

    case 0xA4: // AND HX
      and_A_bytereg(regs_iX_h);
    return(resGO);
    case 0xA5: // AND LX
      and_A_bytereg(regs_iX_l);
    return(resGO);
    case 0xA6: // AND (IX+dd)
      { unsigned char utmp;
        utmp = get1(add_u16_disp(regs_IX_OR_IY, fetch()));
        and_A_bytereg(utmp);
	vc.rd++;
      }
    return(resGO);

    case 0xAC: // XOR HX
      xor_A_bytereg(regs_iX_h);
    return(resGO);
    case 0xAD: // XOR LX
      xor_A_bytereg(regs_iX_l);
    return(resGO);
    case 0xAE: // XOR (IX+dd)
      { unsigned char utmp;
        utmp = get1(add_u16_disp(regs_IX_OR_IY, fetch()));
        xor_A_bytereg(utmp);
	vc.rd++;
      }
    return(resGO);

    case 0xB4: // OR HX
      or_A_bytereg(regs_iX_h);
    return(resGO);
    case 0xB5: // OR LX
      or_A_bytereg(regs_iX_l);
    return(resGO);
    case 0xB6: // OR (IX+dd)
      { unsigned char utmp;
        utmp = get1(add_u16_disp(regs_IX_OR_IY, fetch()));
        or_A_bytereg(utmp);
	vc.rd++;
      }
    return(resGO);

    case 0xBC: // CP HX
      cp_bytereg(regs_iX_h);
    return(resGO);
    case 0xBD: // CP LX
      cp_bytereg(regs_iX_l);
    return(resGO);
    case 0xBE: // CP (IX+dd)
      { unsigned char utmp;
        utmp = get1(add_u16_disp(regs_IX_OR_IY, fetch()));
        cp_bytereg(utmp);
	vc.rd++;
      }
    return(resGO);
  }
  return(resINV_INST);
}

int
cl_z80::inst_Xd(void)
{
  t_mem code;

  if (fetch(&code))
    return(resBREAKPOINT);

  switch (code)
    {
      case 0x21: // LD IX,nnnn
      case 0x22: // LD (nnnn),IX
      case 0x26: // LD HX,nn
      case 0x2A: // LD IX,(nnnn)
      case 0x2E: // LD LX,nn
      case 0x36: // LD (IX+dd),nn
      case 0x44: // LD B,HX
      case 0x45: // LD B,LX
      case 0x46: // LD B,(IX+dd)
      case 0x4C: // LD C,HX
      case 0x4D: // LD C,LX
      case 0x4E: // LD C,(IX+dd)
      case 0x54: // LD D,HX
      case 0x55: // LD D,LX
      case 0x56: // LD D,(IX+dd)
      case 0x5C: // LD E,H
      case 0x5D: // LD E,L
      case 0x5E: // LD E,(IX+dd)
      case 0x60: // LD HX,B
      case 0x61: // LD HX,C
      case 0x62: // LD HX,D
      case 0x63: // LD HX,E
      case 0x64: // LD HX,HX
      case 0x66: // LD H,(IX+dd)
      case 0x67: // LD HX,A
      case 0x68: // LD LX,B
      case 0x69: // LD LX,C
      case 0x6A: // LD LX,D
      case 0x6B: // LD LX,E
      case 0x6C: // LD LX,HX
      case 0x6D: // LD LX,LX
      case 0x6E: // LD L,(IX+dd)
      case 0x6F: // LD LX,A
      case 0x70: // LD (IX+dd),B
      case 0x71: // LD (IX+dd),C
      case 0x72: // LD (IX+dd),D
      case 0x73: // LD (IX+dd),E
      case 0x74: // LD (IX+dd),H
      case 0x75: // LD (IX+dd),L
      case 0x77: // LD (IX+dd),A
      case 0x7C: // LD A,HX
      case 0x7D: // LD A,LX
      case 0x7E: // LD A,(IX+dd)
      case 0xF9: // LD SP,IX
        return(inst_Xd_ld(code));

      case 0x23: // INC IX
      case 0x24: // INC HX
      case 0x2C: // INC LX
      case 0x34: // INC (IX+dd)
        return(inst_Xd_inc(code));

      case 0x09: // ADD IX,BC
      case 0x19: // ADD IX,DE
      case 0x29: // ADD IX,IX
      case 0x39: // ADD IX,SP
      case 0x84: // ADD A,HX
      case 0x85: // ADD A,LX
      case 0x86: // ADD A,(IX)
        return(inst_Xd_add(code));

      case 0x25: // DEC HX
      case 0x2B: // DEC IX
      case 0x2D: // DEC LX
      case 0x35: // DEC (IX+dd)
	return(inst_Xd_dec(code));

      case 0x8C: // ADC A,HX
      case 0x8D: // ADC A,LX
      case 0x8E: // ADC A,(IX)
      case 0x94: // SUB HX
      case 0x95: // SUB LX
      case 0x96: // SUB (IX+dd)
      case 0x9C: // SBC A,HX
      case 0x9D: // SBC A,LX
      case 0x9E: // SBC A,(IX+dd)
      case 0xA4: // AND HX
      case 0xA5: // AND LX
      case 0xA6: // AND (IX+dd)
      case 0xAC: // XOR HX
      case 0xAD: // XOR LX
      case 0xAE: // XOR (IX+dd)
      case 0xB4: // OR HX
      case 0xB5: // OR LX
      case 0xB6: // OR (IX+dd)
      case 0xBC: // CP HX
      case 0xBD: // CP LX
      case 0xBE: // CP (IX+dd)
        return(inst_Xd_misc(code));
      break;

      case 0xCB: // escape, IX prefix to CB commands
        return(inst_Xdcb()); /* see inst_ddcb.cc */
      break;

      case 0xE1: // POP IX
        regs_IX_OR_IY = get2(regs.SP);
        regs.SP+=2;
	vc.rd+= 2;
      return(resGO);

      case 0xE3: // EX (SP),IX
        {
          u16_t tempw;

          tempw = regs_IX_OR_IY;
          regs_IX_OR_IY = get2(regs.SP);
          store2(regs.SP, tempw);
	  vc.rd+= 2;
	  vc.wr+= 2;
        }
      return(resGO);

      case 0xE5: // PUSH IX
        push2(regs_IX_OR_IY);
	vc.wr+= 2;
      return(resGO);

      case 0xE9: // JP (IX)
        PC = regs_IX_OR_IY;
      return(resGO);

      default:
      return(resINV_INST);
    }
  return(resINV_INST);
}

/* End of z80.src/inst_xd.cc */
