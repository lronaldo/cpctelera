/*
 * Simulator of microcontrollers (inst_xxcb.cc)
 *   DD CB or FD CB escaped multi-byte opcodes for Z80.
 *
 *   This module gets pulled in and pre-processed to create
 *   two modules.  DD CB prefixed opcodes reference
 *   IX register, while FD CB prefixes reference IY register.
 *   See inst_ddcb.cc and inst_fdcb.cc
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

static unsigned char n_offset;

int
cl_z80::inst_XXcb_rlc(t_mem code)
{
  unsigned char tmp;
  unsigned short addr;

  addr = add_u16_disp(regs_IX_OR_IY, n_offset);
  tmp = get1(addr);
  rlc_byte(tmp);

  switch(code) {
    case 0x00: // RLC B
      regs.bc.h = tmp;
    break;
    case 0x01: // RLC C
      regs.bc.l = tmp;
    break;
    case 0x02: // RLC D
      regs.de.h = tmp;
    break;
    case 0x03: // RLC E
      regs.de.l = tmp;
    break;
    case 0x04: // RLC H
      regs.hl.h = tmp;
    break;
    case 0x05: // RLC L
      regs.hl.l = tmp;
    break;
    case 0x06: // RLC (HL)
    break;
    case 0x07: // RLC A
      regs.A = tmp;
    break;
    default:
      return(resINV_INST);
    break;
  }
  store1(addr, tmp);

  return(resGO);
}

int
cl_z80::inst_XXcb_rrc(t_mem code)
{
  unsigned char tmp;
  unsigned short addr;
  addr = add_u16_disp(regs_IX_OR_IY, n_offset);
  tmp = get1(addr);
  rrc_byte(tmp);

  switch(code) {
    case 0x08: // RRC B
      regs.bc.h = tmp;
    break;
    case 0x09: // RRC C
      regs.bc.l = tmp;
    break;
    case 0x0A: // RRC D
      regs.de.h = tmp;
    break;
    case 0x0B: // RRC E
      regs.de.l = tmp;
    break;
    case 0x0C: // RRC H
      regs.hl.h = tmp;
    break;
    case 0x0D: // RRC L
      regs.hl.l = tmp;
    break;
    case 0x0E: // RRC (HL)
    break;
    case 0x0F: // RRC A
      regs.A = tmp;
    break;
    default:
      return(resINV_INST);
    break;
  }
  store1(addr, tmp);

  return(resGO);
}

int
cl_z80::inst_XXcb_rl(t_mem code)
{
  unsigned char tmp;
  unsigned short addr;
  addr = add_u16_disp(regs_IX_OR_IY, n_offset);
  tmp = get1(addr);
  rl_byte(tmp);

  switch(code) {
    case 0x10: // RL B
      regs.bc.h = tmp;
    break;
    case 0x11: // RL C
      regs.bc.l = tmp;
    break;
    case 0x12: // RL D
      regs.de.h = tmp;
    break;
    case 0x13: // RL E
      regs.de.l = tmp;
    break;
    case 0x14: // RL H
      regs.hl.h = tmp;
    break;
    case 0x15: // RL L
      regs.hl.l = tmp;
    break;
    case 0x16: // RL (HL)
    break;
    case 0x17: // RL A
      regs.A = tmp;
    break;
    default:
      return(resINV_INST);
    break;
  }
  store1(addr, tmp);

  return(resGO);
}

int
cl_z80::inst_XXcb_rr(t_mem code)
{
  unsigned char tmp;
  unsigned short addr;
  addr = add_u16_disp(regs_IX_OR_IY, n_offset);
  tmp = get1(addr);
  rr_byte(tmp);

  switch(code) {
    case 0x18: // RR B
      regs.bc.h = tmp;
    break;
    case 0x19: // RR C
      regs.bc.l = tmp;
    break;
    case 0x1A: // RR D
      regs.de.h = tmp;
    break;
    case 0x1B: // RR E
      regs.de.l = tmp;
    break;
    case 0x1C: // RR H
      regs.hl.h = tmp;
    break;
    case 0x1D: // RR L
      regs.hl.l = tmp;
    break;
    case 0x1E: // RR (HL)
    break;
    case 0x1F: // RR A
      regs.A = tmp;
    break;
    default:
      return(resINV_INST);
    break;
  }
  store1(addr, tmp);

  return(resGO);
}

int
cl_z80::inst_XXcb_sla(t_mem code)
{
  unsigned char tmp;
  unsigned short addr;
  addr = add_u16_disp(regs_IX_OR_IY, n_offset);
  tmp = get1(addr);
  sla_byte(tmp);

  switch(code) {
    case 0x20: // SLA B
      regs.bc.h = tmp;
    break;
    case 0x21: // SLA C
      regs.bc.l = tmp;
    break;
    case 0x22: // SLA D
      regs.de.h = tmp;
    break;
    case 0x23: // SLA E
      regs.de.l = tmp;
    break;
    case 0x24: // SLA H
      regs.hl.h = tmp;
    break;
    case 0x25: // SLA L
      regs.hl.l = tmp;
    break;
    case 0x26: // SLA (HL)
    break;
    case 0x27: // SLA A
      regs.A = tmp;
    break;
    default:
      return(resINV_INST);
    break;
  }
  store1(addr, tmp);
  return(resGO);
}

int
cl_z80::inst_XXcb_sra(t_mem code)
{
  unsigned char tmp;
  unsigned short addr;
  addr = add_u16_disp(regs_IX_OR_IY, n_offset);
  tmp = get1(addr);
  sra_byte(tmp);

  switch(code) {
    case 0x28: // SRA B
      regs.bc.h = tmp;
    break;
    case 0x29: // SRA C
      regs.bc.l = tmp;
    break;
    case 0x2A: // SRA D
      regs.de.h = tmp;
    break;
    case 0x2B: // SRA E
      regs.de.l = tmp;
    break;
    case 0x2C: // SRA H
      regs.hl.h = tmp;
    break;
    case 0x2D: // SRA L
      regs.hl.l = tmp;
    break;
    case 0x2E: // SRA (HL)
    break;
    case 0x2F: // SRA A
      regs.A = tmp;
    break;
    default:
      return(resINV_INST);
    break;
  }
  store1(addr, tmp);
  return(resGO);
}

int
cl_z80::inst_XXcb_slia(t_mem code)
{
  unsigned char tmp;
  unsigned short addr;
  addr = add_u16_disp(regs_IX_OR_IY, n_offset);
  tmp = get1(addr);
  slia_byte(tmp);

  switch(code) {
    case 0x30: // SLIA B	(Shift Left Inverted Arithmetic)
      regs.bc.h = tmp;
    break;
    case 0x31: // SLIA C	like SLA, but shifts in a 1 bit
      regs.bc.l = tmp;
    break;
    case 0x32: // SLIA D
      regs.de.h = tmp;
    break;
    case 0x33: // SLIA E
      regs.de.l = tmp;
    break;
    case 0x34: // SLIA H
      regs.hl.h = tmp;
    break;
    case 0x35: // SLIA L
      regs.hl.l = tmp;
    break;
    case 0x36: // SLIA (HL)
    break;
    case 0x37: // SLIA A
      regs.A = tmp;
    break;
    default:
      return(resINV_INST);
    break;
  }
  store1(addr, tmp);
  return(resGO);
}

int
cl_z80::inst_XXcb_srl(t_mem code)
{
  unsigned char tmp;
  unsigned short addr;
  addr = add_u16_disp(regs_IX_OR_IY, n_offset);
  tmp = get1(addr);
  srl_byte(tmp);

  switch(code) {
    case 0x38: // SRL B
      regs.bc.h = tmp;
    break;
    case 0x39: // SRL C
      regs.bc.l = tmp;
    break;
    case 0x3A: // SRL D
      regs.de.h = tmp;
    break;
    case 0x3B: // SRL E
      regs.de.l = tmp;
    break;
    case 0x3C: // SRL H
      regs.hl.h = tmp;
    break;
    case 0x3D: // SRL L
      regs.hl.l = tmp;
    break;
    case 0x3E: // SRL (HL)
    break;
    case 0x3F: // SRL A
      regs.A = tmp;
    break;
    default:
      return(resINV_INST);
    break;
  }
  store1(addr, tmp);
  return(resGO);
}


int
cl_z80::inst_XXcb_bit(t_mem code)
{
  unsigned char tmp;
  unsigned short addr;

#define bit_bitnum ((code >> 3) & 7)

  addr = add_u16_disp(regs_IX_OR_IY, n_offset);
  tmp = get1(addr);
  bit_byte(tmp, bit_bitnum);

  store1(addr, tmp);

  return(resGO);
}

int
cl_z80::inst_XXcb_res(t_mem code)
{
  unsigned char tmp;
  unsigned short addr;

#define bit_bitnum ((code >> 3) & 7)

  addr = add_u16_disp(regs_IX_OR_IY, n_offset);
  tmp = get1(addr);
  tmp &= ~(1 << bit_bitnum);

  switch(code & 0x7) {
    case 0x0: // RES x,B
      regs.bc.h = tmp; break;
    case 0x1: // RES x,C
      regs.bc.l = tmp; break;
    case 0x2: // RES x,D
      regs.de.h = tmp; break;
    case 0x3: // RES x,E
      regs.de.l = tmp; break;
    case 0x4: // RES x,H
      regs.hl.h = tmp; break;
    case 0x5: // RES x,L
      regs.hl.l = tmp; break;
    case 0x6: // RES x,(HL)
      break;
    case 0x7: // RES x,A
      regs.A = tmp; break;
    default:
      return(resINV_INST);
  }
  store1(addr, tmp);
  return(resGO);
}

int
cl_z80::inst_XXcb_set(t_mem code)
{
  unsigned char tmp;
  unsigned short addr;

#define bit_bitnum ((code >> 3) & 7)

  addr = add_u16_disp(regs_IX_OR_IY, n_offset);
  tmp = get1(addr);
  tmp |= (1 << bit_bitnum);

  switch(code & 0x7) {
    case 0x0: // SET x,B
      regs.bc.h = tmp; break;
    case 0x1: // SET x,C
      regs.bc.l = tmp; break;
    case 0x2: // SET x,D
      regs.de.h = tmp; break;
    case 0x3: // SET x,E
      regs.de.l = tmp; break;
    case 0x4: // SET x,H
      regs.hl.h = tmp; break;
    case 0x5: // SET x,L
      regs.hl.h = tmp; break;
    case 0x6: // SET x,(HL)
      break;
    case 0x7: // SET x,A
      regs.A = tmp; break;
    default:
      return(resINV_INST);
  }
  store1(addr, tmp);
  return(resGO);
}

/******** start CB codes *****************/
int
cl_z80::inst_XXcb(void)
{
  t_mem code;

  // all DD CB escaped opcodes have a 3rd byte which is a displacement,
  // 4th byte is opcode extension.
  n_offset = fetch();

  if (fetch(&code))
    return(resBREAKPOINT);
  tick(1);
  switch (code)
    {
    case 0x00: // RLC B
    case 0x01: // RLC C
    case 0x02: // RLC D
    case 0x03: // RLC E
    case 0x04: // RLC H
    case 0x05: // RLC L
    case 0x06: // RLC (HL)
    case 0x07: // RLC A
      return (inst_XXcb_rlc(code));
    case 0x08: // RRC B
    case 0x09: // RRC C
    case 0x0A: // RRC D
    case 0x0B: // RRC E
    case 0x0C: // RRC H
    case 0x0D: // RRC L
    case 0x0E: // RRC (HL)
    case 0x0F: // RRC A
      return (inst_XXcb_rrc(code));
    case 0x10: // RL B
    case 0x11: // RL C
    case 0x12: // RL D
    case 0x13: // RL E
    case 0x14: // RL H
    case 0x15: // RL L
    case 0x16: // RL (HL)
    case 0x17: // RL A
      return (inst_XXcb_rl(code));
    case 0x18: // RR B
    case 0x19: // RR C
    case 0x1A: // RR D
    case 0x1B: // RR E
    case 0x1C: // RR H
    case 0x1D: // RR L
    case 0x1E: // RR (HL)
    case 0x1F: // RR A
      return (inst_XXcb_rr(code));
    case 0x20: // SLA B
    case 0x21: // SLA C
    case 0x22: // SLA D
    case 0x23: // SLA E
    case 0x24: // SLA H
    case 0x25: // SLA L
    case 0x26: // SLA (HL)
    case 0x27: // SLA A
      return (inst_XXcb_sla(code));
    case 0x28: // SRA B
    case 0x29: // SRA C
    case 0x2A: // SRA D
    case 0x2B: // SRA E
    case 0x2C: // SRA H
    case 0x2D: // SRA L
    case 0x2E: // SRA (HL)
    case 0x2F: // SRA A
      return (inst_XXcb_sra(code));
    case 0x30: // SLIA B	(Shift Left Inverted Arithmetic)
    case 0x31: // SLIA C	like SLA, but shifts in a 1 bit
    case 0x32: // SLIA D
    case 0x33: // SLIA E
    case 0x34: // SLIA H
    case 0x35: // SLIA L
    case 0x36: // SLIA (HL)
    case 0x37: // SLIA A
      return (inst_XXcb_slia(code));
    case 0x38: // SRL B
    case 0x39: // SRL C
    case 0x3A: // SRL D
    case 0x3B: // SRL E
    case 0x3C: // SRL H
    case 0x3D: // SRL L
    case 0x3E: // SRL (HL)
    case 0x3F: // SRL A
      return (inst_XXcb_srl(code));
    case 0x46: // BIT 0,(HL)
    case 0x4E: // BIT 1,(HL)
    case 0x56: // BIT 2,(HL)
    case 0x5E: // BIT 3,(HL)
    case 0x66: // BIT 4,(HL)
    case 0x6E: // BIT 5,(HL)
    case 0x76: // BIT 6,(HL)
    case 0x7E: // BIT 7,(HL)
      return (inst_XXcb_bit(code));
    case 0x80: // RES 0,B
    case 0x81: // RES 0,C
    case 0x82: // RES 0,D
    case 0x83: // RES 0,E
    case 0x84: // RES 0,H
    case 0x85: // RES 0,L
    case 0x86: // RES 0,(HL)
    case 0x87: // RES 0,A
    case 0x88: // RES 1,B
    case 0x89: // RES 1,C
    case 0x8A: // RES 1,D
    case 0x8B: // RES 1,E
    case 0x8C: // RES 1,H
    case 0x8D: // RES 1,L
    case 0x8E: // RES 1,(HL)
    case 0x8F: // RES 1,A
    case 0x90: // RES 2,B
    case 0x91: // RES 2,C
    case 0x92: // RES 2,D
    case 0x93: // RES 2,E
    case 0x94: // RES 2,H
    case 0x95: // RES 2,L
    case 0x96: // RES 2,(HL)
    case 0x97: // RES 2,A
    case 0x98: // RES 3,B
    case 0x99: // RES 3,C
    case 0x9A: // RES 3,D
    case 0x9B: // RES 3,E
    case 0x9C: // RES 3,H
    case 0x9D: // RES 3,L
    case 0x9E: // RES 3,(HL)
    case 0x9F: // RES 3,A
    case 0xA0: // RES 4,B
    case 0xA1: // RES 4,C
    case 0xA2: // RES 4,D
    case 0xA3: // RES 4,E
    case 0xA4: // RES 4,H
    case 0xA5: // RES 4,L
    case 0xA6: // RES 4,(HL)
    case 0xA7: // RES 4,A
    case 0xA8: // RES 5,B
    case 0xA9: // RES 5,C
    case 0xAA: // RES 5,D
    case 0xAB: // RES 5,E
    case 0xAC: // RES 5,H
    case 0xAD: // RES 5,L
    case 0xAE: // RES 5,(HL)
    case 0xAF: // RES 5,A
    case 0xB0: // RES 6,B
    case 0xB1: // RES 6,C
    case 0xB2: // RES 6,D
    case 0xB3: // RES 6,E
    case 0xB4: // RES 6,H
    case 0xB5: // RES 6,L
    case 0xB6: // RES 6,(HL)
    case 0xB7: // RES 6,A
    case 0xB8: // RES 7,B
    case 0xB9: // RES 7,C
    case 0xBA: // RES 7,D
    case 0xBB: // RES 7,E
    case 0xBC: // RES 7,H
    case 0xBD: // RES 7,L
    case 0xBE: // RES 7,(HL)
    case 0xBF: // RES 7,A
      return (inst_XXcb_res(code));
    case 0xC0: // SET 0,B
    case 0xC1: // SET 0,C
    case 0xC2: // SET 0,D
    case 0xC3: // SET 0,E
    case 0xC4: // SET 0,H
    case 0xC5: // SET 0,L
    case 0xC6: // SET 0,(HL)
    case 0xC7: // SET 0,A
    case 0xC8: // SET 1,B
    case 0xC9: // SET 1,C
    case 0xCA: // SET 1,D
    case 0xCB: // SET 1,E
    case 0xCC: // SET 1,H
    case 0xCD: // SET 1,L
    case 0xCE: // SET 1,(HL)
    case 0xCF: // SET 1,A
    case 0xD0: // SET 2,B
    case 0xD1: // SET 2,C
    case 0xD2: // SET 2,D
    case 0xD3: // SET 2,E
    case 0xD4: // SET 2,H
    case 0xD5: // SET 2,L
    case 0xD6: // SET 2,(HL)
    case 0xD7: // SET 2,A
    case 0xD8: // SET 3,B
    case 0xD9: // SET 3,C
    case 0xDA: // SET 3,D
    case 0xDB: // SET 3,E
    case 0xDC: // SET 3,H
    case 0xDD: // SET 3,L
    case 0xDE: // SET 3,(HL)
    case 0xDF: // SET 3,A
    case 0xE0: // SET 4,B
    case 0xE1: // SET 4,C
    case 0xE2: // SET 4,D
    case 0xE3: // SET 4,E
    case 0xE4: // SET 4,H
    case 0xE5: // SET 4,L
    case 0xE6: // SET 4,(HL)
    case 0xE7: // SET 4,A
    case 0xE8: // SET 5,B
    case 0xE9: // SET 5,C
    case 0xEA: // SET 5,D
    case 0xEB: // SET 5,E
    case 0xEC: // SET 5,H
    case 0xED: // SET 5,L
    case 0xEE: // SET 5,(HL)
    case 0xEF: // SET 5,A
    case 0xF0: // SET 6,B
    case 0xF1: // SET 6,C
    case 0xF2: // SET 6,D
    case 0xF3: // SET 6,E
    case 0xF4: // SET 6,H
    case 0xF5: // SET 6,L
    case 0xF6: // SET 6,(HL)
    case 0xF7: // SET 6,A
    case 0xF8: // SET 7,B
    case 0xF9: // SET 7,C
    case 0xFA: // SET 7,D
    case 0xFB: // SET 7,E
    case 0xFC: // SET 7,H
    case 0xFD: // SET 7,L
    case 0xFE: // SET 7,(HL)
    case 0xFF: // SET 7,A
      return (inst_XXcb_set(code));
    }
  /*if (PC)
    PC--;
  else
  PC= get_mem_size(MEM_ROM_ID)-1;*/
  PC= rom->inc_address(PC, -1);
  return(resINV_INST);
}

/* End of z80.src/inst_xxcb.cc */
