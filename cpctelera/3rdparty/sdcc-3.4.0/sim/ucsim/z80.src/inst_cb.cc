/*
 * Simulator of microcontrollers (inst.cc)
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

int
cl_z80::inst_cb_rlc(t_mem code)
{
  switch(code) {
    case 0x00: // RLC B
      rlc_byte(regs.bc.h);
    break;
    case 0x01: // RLC C
      rlc_byte(regs.bc.l);
    break;
    case 0x02: // RLC D
      rlc_byte(regs.de.h);
    break;
    case 0x03: // RLC E
      rlc_byte(regs.de.l);
    break;
    case 0x04: // RLC H
      rlc_byte(regs.hl.h);
    break;
    case 0x05: // RLC L
      rlc_byte(regs.hl.l);
    break;
    case 0x06: // RLC (HL)
      { unsigned char tmp;
        tmp = get1(regs.HL);
        rlc_byte(tmp);
        store1(regs.HL, tmp);
      }
    break;
    case 0x07: // RLC A
      rlc_byte(regs.A);
    break;
  }
  return(resGO);
}

int
cl_z80::inst_cb_rrc(t_mem code)
{
  switch(code) {
    case 0x08: // RRC B
      rrc_byte(regs.bc.h);
    break;
    case 0x09: // RRC C
      rrc_byte(regs.bc.l);
    break;
    case 0x0A: // RRC D
      rrc_byte(regs.de.h);
    break;
    case 0x0B: // RRC E
      rrc_byte(regs.de.l);
    break;
    case 0x0C: // RRC H
      rrc_byte(regs.hl.h);
    break;
    case 0x0D: // RRC L
      rrc_byte(regs.hl.l);
    break;
    case 0x0E: // RRC (HL)
      { unsigned char tmp;
        tmp = get1(regs.HL);
        rrc_byte(tmp);
        store1(regs.HL, tmp);
      }
    break;
    case 0x0F: // RRC A
      rrc_byte(regs.A);
    break;
  }
  return(resGO);
}

int
cl_z80::inst_cb_rl(t_mem code)
{
  switch(code) {
    case 0x10: // RL B
      rl_byte(regs.bc.h);
    break;
    case 0x11: // RL C
      rl_byte(regs.bc.l);
    break;
    case 0x12: // RL D
      rl_byte(regs.de.h);
    break;
    case 0x13: // RL E
      rl_byte(regs.de.l);
    break;
    case 0x14: // RL H
      rl_byte(regs.hl.h);
    break;
    case 0x15: // RL L
      rl_byte(regs.hl.l);
    break;
    case 0x16: // RL (HL)
      { unsigned char tmp;
        tmp = get1(regs.HL);
        rl_byte(tmp);
        store1(regs.HL, tmp);
      }
    break;
    case 0x17: // RL A
      rl_byte(regs.A);
    break;
  }
  return(resGO);
}

int
cl_z80::inst_cb_rr(t_mem code)
{
  switch(code) {
    case 0x18: // RR B
      rr_byte(regs.bc.h);
    break;
    case 0x19: // RR C
      rr_byte(regs.bc.l);
    break;
    case 0x1A: // RR D
      rr_byte(regs.de.h);
    break;
    case 0x1B: // RR E
      rr_byte(regs.de.l);
    break;
    case 0x1C: // RR H
      rr_byte(regs.hl.h);
    break;
    case 0x1D: // RR L
      rr_byte(regs.hl.l);
    break;
    case 0x1E: // RR (HL)
      { unsigned char tmp;
        tmp = get1(regs.HL);
        rr_byte(tmp);
        store1(regs.HL, tmp);
      }
    break;
    case 0x1F: // RR A
      rr_byte(regs.A);
    break;
  }
  return(resGO);
}

int
cl_z80::inst_cb_sla(t_mem code)
{
  switch(code) {
    case 0x20: // SLA B
      sla_byte(regs.bc.h);
    break;
    case 0x21: // SLA C
      sla_byte(regs.bc.l);
    break;
    case 0x22: // SLA D
      sla_byte(regs.de.h);
    break;
    case 0x23: // SLA E
      sla_byte(regs.de.l);
    break;
    case 0x24: // SLA H
      sla_byte(regs.hl.h);
    break;
    case 0x25: // SLA L
      sla_byte(regs.hl.l);
    break;
    case 0x26: // SLA (HL)
      { unsigned char tmp;
        tmp = get1(regs.HL);
        sla_byte(tmp);
        store1(regs.HL, tmp);
      }
    break;
    case 0x27: // SLA A
      sla_byte(regs.A);
    break;
  }
  return(resGO);
}

int
cl_z80::inst_cb_sra(t_mem code)
{
  switch(code) {
    case 0x28: // SRA B
      sra_byte(regs.bc.h);
    break;
    case 0x29: // SRA C
      sra_byte(regs.bc.l);
    break;
    case 0x2A: // SRA D
      sra_byte(regs.de.h);
    break;
    case 0x2B: // SRA E
      sra_byte(regs.de.l);
    break;
    case 0x2C: // SRA H
      sra_byte(regs.hl.h);
    break;
    case 0x2D: // SRA L
      sra_byte(regs.hl.l);
    break;
    case 0x2E: // SRA (HL)
      { unsigned char tmp;
        tmp = get1(regs.HL);
        sra_byte(tmp);
        store1(regs.HL, tmp);
      }
    break;
    case 0x2F: // SRA A
      sra_byte(regs.A);
    break;
  }
  return(resGO);
}

int
cl_z80::inst_cb_slia(t_mem code)
{
  switch(code) {
    case 0x30: // SLIA B	(Shift Left Inverted Arithmetic)
      slia_byte(regs.bc.h);
    break;
    case 0x31: // SLIA C	like SLA, but shifts in a 1 bit
      slia_byte(regs.bc.l);
    break;
    case 0x32: // SLIA D
      slia_byte(regs.de.h);
    break;
    case 0x33: // SLIA E
      slia_byte(regs.de.l);
    break;
    case 0x34: // SLIA H
      slia_byte(regs.hl.h);
    break;
    case 0x35: // SLIA L
      slia_byte(regs.hl.l);
    break;
    case 0x36: // SLIA (HL)
      { unsigned char tmp;
        tmp = get1(regs.HL);
        slia_byte(tmp);
        store1(regs.HL, tmp);
      }
    break;
    case 0x37: // SLIA A
      slia_byte(regs.A);
    break;
  }
  return(resGO);
}

int
cl_z80::inst_cb_srl(t_mem code)
{
  switch(code) {
    case 0x38: // SRL B
      srl_byte(regs.bc.h);
    break;
    case 0x39: // SRL C
      srl_byte(regs.bc.l);
    break;
    case 0x3A: // SRL D
      srl_byte(regs.de.h);
    break;
    case 0x3B: // SRL E
      srl_byte(regs.de.l);
    break;
    case 0x3C: // SRL H
      srl_byte(regs.hl.h);
    break;
    case 0x3D: // SRL L
      srl_byte(regs.hl.l);
    break;
    case 0x3E: // SRL (HL)
      { unsigned char tmp;
        tmp = get1(regs.HL);
        srl_byte(tmp);
        store1(regs.HL, tmp);
      }
    break;
    case 0x3F: // SRL A
      srl_byte(regs.A);
    break;
  }
  return(resGO);
}


int
cl_z80::inst_cb_bit(t_mem code)
{
#define bit_bitnum ((code >> 3) & 7)

  switch(code & 7) {
    case 0x0: // BIT x,B
      bit_byte(regs.bc.h, bit_bitnum); break;
    case 0x1: // BIT x,C
      bit_byte(regs.bc.l, bit_bitnum); break;
    case 0x2: // BIT x,D
      bit_byte(regs.de.h, bit_bitnum); break;
    case 0x3: // BIT x,E
      bit_byte(regs.de.l, bit_bitnum); break;
    case 0x4: // BIT x,H
      bit_byte(regs.hl.h, bit_bitnum); break;
    case 0x5: // BIT x,L
      bit_byte(regs.hl.l, bit_bitnum); break;
    case 0x6: // BIT x,(HL)
      { unsigned char tmp;
        tmp = get1(regs.HL);
        bit_byte(tmp, bit_bitnum);
        store1(regs.HL, tmp);
      }
    break;
    case 0x7: // BIT x,A
      bit_byte(regs.A, bit_bitnum); break;
    break;
  }
  return(resGO);
}

int
cl_z80::inst_cb_res(t_mem code)
{
#define bit_bitnum ((code >> 3) & 7)

  switch(code & 0x7) {
    case 0x0: // RES x,B
      regs.bc.h &= ~(1 << bit_bitnum); break;
    case 0x1: // RES x,C
      regs.bc.l &= ~(1 << bit_bitnum); break;
    case 0x2: // RES x,D
      regs.de.h &= ~(1 << bit_bitnum); break;
    case 0x3: // RES x,E
      regs.de.l &= ~(1 << bit_bitnum); break;
    case 0x4: // RES x,H
      regs.hl.h &= ~(1 << bit_bitnum); break;
    case 0x5: // RES x,L
      regs.hl.l &= ~(1 << bit_bitnum); break;
    case 0x6: // RES x,(HL)
      { unsigned char tmp;
        tmp = get1(regs.HL);
        tmp &= ~(1 << bit_bitnum);
        store1(regs.HL, tmp);
      }
    break;
    case 0x7: // RES x,A
      regs.A &= ~(1 << bit_bitnum); break;
  }
  return(resGO);
}

int
cl_z80::inst_cb_set(t_mem code)
{
#define bit_bitnum ((code >> 3) & 7)

  switch(code & 0x7) {
    case 0x0: // SET x,B
      regs.bc.h |= (1 << bit_bitnum); break;
    case 0x1: // SET x,C
      regs.bc.l |= (1 << bit_bitnum); break;
    case 0x2: // SET x,D
      regs.de.h |= (1 << bit_bitnum); break;
    case 0x3: // SET x,E
      regs.de.l |= (1 << bit_bitnum); break;
    case 0x4: // SET x,H
      regs.hl.h |= (1 << bit_bitnum); break;
    case 0x5: // SET x,L
      regs.hl.l |= (1 << bit_bitnum); break;
    case 0x6: // SET x,(HL)
      { unsigned char tmp;
        tmp = get1(regs.HL);
        tmp |= (1 << bit_bitnum);
        store1(regs.HL, tmp);
      }
    break;
    case 0x7: // SET x,A
      regs.A |= (1 << bit_bitnum); break;
  }
  return(resGO);
}

/******** start CB codes *****************/
int
cl_z80::inst_cb(void)
{
  t_mem code;

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
      return (inst_cb_rlc(code));
    case 0x08: // RRC B
    case 0x09: // RRC C
    case 0x0A: // RRC D
    case 0x0B: // RRC E
    case 0x0C: // RRC H
    case 0x0D: // RRC L
    case 0x0E: // RRC (HL)
    case 0x0F: // RRC A
      return (inst_cb_rrc(code));
    case 0x10: // RL B
    case 0x11: // RL C
    case 0x12: // RL D
    case 0x13: // RL E
    case 0x14: // RL H
    case 0x15: // RL L
    case 0x16: // RL (HL)
    case 0x17: // RL A
      return (inst_cb_rl(code));
    case 0x18: // RR B
    case 0x19: // RR C
    case 0x1A: // RR D
    case 0x1B: // RR E
    case 0x1C: // RR H
    case 0x1D: // RR L
    case 0x1E: // RR (HL)
    case 0x1F: // RR A
      return (inst_cb_rr(code));
    case 0x20: // SLA B
    case 0x21: // SLA C
    case 0x22: // SLA D
    case 0x23: // SLA E
    case 0x24: // SLA H
    case 0x25: // SLA L
    case 0x26: // SLA (HL)
    case 0x27: // SLA A
      return (inst_cb_sla(code));
    case 0x28: // SRA B
    case 0x29: // SRA C
    case 0x2A: // SRA D
    case 0x2B: // SRA E
    case 0x2C: // SRA H
    case 0x2D: // SRA L
    case 0x2E: // SRA (HL)
    case 0x2F: // SRA A
      return (inst_cb_sra(code));
    case 0x30: // SLIA B	(Shift Left Inverted Arithmetic)
    case 0x31: // SLIA C	like SLA, but shifts in a 1 bit
    case 0x32: // SLIA D
    case 0x33: // SLIA E
    case 0x34: // SLIA H
    case 0x35: // SLIA L
    case 0x36: // SLIA (HL)
    case 0x37: // SLIA A
      return (inst_cb_slia(code));
    case 0x38: // SRL B
    case 0x39: // SRL C
    case 0x3A: // SRL D
    case 0x3B: // SRL E
    case 0x3C: // SRL H
    case 0x3D: // SRL L
    case 0x3E: // SRL (HL)
    case 0x3F: // SRL A
      return (inst_cb_srl(code));
    case 0x40: // BIT 0,B
    case 0x41: // BIT 0,C
    case 0x42: // BIT 0,D
    case 0x43: // BIT 0,E
    case 0x44: // BIT 0,H
    case 0x45: // BIT 0,L
    case 0x46: // BIT 0,(HL)
    case 0x47: // BIT 0,A
    case 0x48: // BIT 1,B
    case 0x49: // BIT 1,C
    case 0x4A: // BIT 1,D
    case 0x4B: // BIT 1,E
    case 0x4C: // BIT 1,H
    case 0x4D: // BIT 1,L
    case 0x4E: // BIT 1,(HL)
    case 0x4F: // BIT 1,A
    case 0x50: // BIT 2,B
    case 0x51: // BIT 2,C
    case 0x52: // BIT 2,D
    case 0x53: // BIT 2,E
    case 0x54: // BIT 2,H
    case 0x55: // BIT 2,L
    case 0x56: // BIT 2,(HL)
    case 0x57: // BIT 2,A
    case 0x58: // BIT 3,B
    case 0x59: // BIT 3,C
    case 0x5A: // BIT 3,D
    case 0x5B: // BIT 3,E
    case 0x5C: // BIT 3,H
    case 0x5D: // BIT 3,L
    case 0x5E: // BIT 3,(HL)
    case 0x5F: // BIT 3,A
    case 0x60: // BIT 4,B
    case 0x61: // BIT 4,C
    case 0x62: // BIT 4,D
    case 0x63: // BIT 4,E
    case 0x64: // BIT 4,H
    case 0x65: // BIT 4,L
    case 0x66: // BIT 4,(HL)
    case 0x67: // BIT 4,A
    case 0x68: // BIT 5,B
    case 0x69: // BIT 5,C
    case 0x6A: // BIT 5,D
    case 0x6B: // BIT 5,E
    case 0x6C: // BIT 5,H
    case 0x6D: // BIT 5,L
    case 0x6E: // BIT 5,(HL)
    case 0x6F: // BIT 5,A
    case 0x70: // BIT 6,B
    case 0x71: // BIT 6,C
    case 0x72: // BIT 6,D
    case 0x73: // BIT 6,E
    case 0x74: // BIT 6,H
    case 0x75: // BIT 6,L
    case 0x76: // BIT 6,(HL)
    case 0x77: // BIT 6,A
    case 0x78: // BIT 7,B
    case 0x79: // BIT 7,C
    case 0x7A: // BIT 7,D
    case 0x7B: // BIT 7,E
    case 0x7C: // BIT 7,H
    case 0x7D: // BIT 7,L
    case 0x7E: // BIT 7,(HL)
    case 0x7F: // BIT 7,A
      return (inst_cb_bit(code));
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
      return (inst_cb_res(code));
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
      return (inst_cb_set(code));
    }
  /*if (PC)
    PC--;
  else
  PC= get_mem_size(MEM_ROM_ID)-1;*/
  PC= rom->inc_address(PC, -1);
  return(resINV_INST);
}

/* End of z80.src/inst_cb.cc */
