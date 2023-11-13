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

// local
#include "r2kcl.h"
#include "z80mac.h"


unsigned   word_parity( u16_t  x ) {
  // bitcount(x) performed by shift-and-add
  u16_t  tmp = (x & 0x5555) + ((x & 0xAAAA) >> 1);
  tmp = (tmp & 0x3333) + ((tmp & 0xCCCC) >> 2);
  tmp = (tmp & 0x0F0F) + ((tmp & 0xF0F0) >> 4);
  tmp = (tmp & 0x000F) + ((tmp & 0x0F00) >> 8);
  
  // parity determined by count being odd or even
  return  0x01 ^ (tmp & 1);
}

/******** rabbit 2000 memory access helper functions *****************/
u32_t  rabbit_mmu::logical_addr_to_phys( u16_t logical_addr ) {
  u32_t  phys_addr = logical_addr;
  unsigned     segnib = logical_addr >> 12;
  
  if (segnib >= 0xe)
  {
    phys_addr += ((u32_t)xpc) << 12;
  }
  else if (segnib >= ((segsize >> 4) & 0xf))
  {
    phys_addr += ((u32_t)stackseg) << 12;    
  }
  else if (segnib >= (segsize & 0xf))
  {
    phys_addr += ((u32_t)dataseg) << 12;    
  }
  return phys_addr;
}

void cl_r2k::store1( u16_t addr, t_mem val ) {
  u32_t  phys_addr;
  
  if (mmu.io_flag == IOI) {
    if ((mmu.mmidr ^ 0x80) & 0x80)
      /* bit 7 = 0 --> use only 8-bits for internal I/O addresses */
      addr = addr & 0x0ff;
    
    if (addr == MMIDR) {
      mmu.mmidr = val;
      return;
    }
    
    if (addr == SADR) {
      /* serial A (console when using the rabbit programming cable) */
      putchar(val);
      fflush(stdout);
    }
    return;
  }
  if (mmu.io_flag == IOE) {
    /* I/O operation for external device (such as an ethernet controller) */
    return;
  }
  
  phys_addr = mmu.logical_addr_to_phys( addr );
  ram->write(phys_addr, val);
}

void cl_r2k::store2( u16_t addr, u16_t val ) {
  u32_t  phys_addr;
  
  if (mmu.io_flag == IOI) {
    /* I/O operation for on-chip device (serial ports, timers, etc) */
    return;
  }
  
  if (mmu.io_flag == IOE) {
    /* I/O operation for external device (such as an ethernet controller) */
    return;
  }
  
  phys_addr = mmu.logical_addr_to_phys( addr );
  
  ram->write(phys_addr,   val & 0xff);
  ram->write(phys_addr+1, (val >> 8) & 0xff);
}

u8_t  cl_r2k::get1( u16_t addr ) {
  u32_t  phys_addr = mmu.logical_addr_to_phys( addr );
  
  if (mmu.io_flag == IOI) {
    /* stub for on-chip device I/O */
    return 0;
  }
  if (mmu.io_flag == IOE) {
    /* stub for external device I/O */
    return 0;
  }
  
  return ram->read(phys_addr);
}

u16_t  cl_r2k::get2( u16_t addr ) {
  u32_t phys_addr = mmu.logical_addr_to_phys( addr );
  u16_t  l, h;
  
  if (mmu.io_flag == IOI) {
    /* stub for on-chip device I/O */
    return 0;
  }
  if (mmu.io_flag == IOE) {
    /* stub for external device I/O */
    return 0;
  }
  
  l = ram->read(phys_addr  );
  h = ram->read(phys_addr+1);
  
  return (h << 8) | l;
}

t_mem       cl_r2k::fetch1( void ) {
  return fetch( );
}

u16_t  cl_r2k::fetch2( void ) {
  u16_t  c1, c2;
  
  c1 = fetch( );
  c2 = fetch( );
  return (c2 << 8) | c1;
}

t_mem cl_r2k::fetch(void) {
  /*
   * Fetch without checking for breakpoint hit
   *
   * Used by bool cl_uc::fetch(t_mem *code) in sim.src/uc.cc
   * which does check for a breakpoint hit
   */
  
  u32_t phys_addr = mmu.logical_addr_to_phys( PC );
  ulong code;
  
  if (!rom)
    return(0);
  
  code= rom->read(phys_addr);
  PC = (PC + 1) & 0xffffUL;
  vc.fetch++;
  return(code);
}

/******** start rabbit 2000 specific codes *****************/
int cl_r2k::inst_add_sp_d(t_mem code) {
  u16_t  d = fetch( );
  /* sign-extend d from 8-bits to 16-bits */
  d |= (d>>7)*0xFF00;
  regs.SP = (regs.SP + d) & 0xffff;
  return(resGO);
}

int cl_r2k::inst_altd(t_mem code) {
  // stub
  return(resGO);
}

int
cl_r2k::inst_r2k_ld(t_mem code)
{
  /* 0xC4  ld hl,(sp+n)
   * 0xD4  ld (sp+n),hl
   * 0xE4  ld hl,(ix+d)
   *   DD E4 = ld hl,(hl+d)   [note: (hl+d) breaks the normal prefix pattern]
   *   FD E4 = ld hl,(iy+d)
   * 0xF4  ld (ix+d),hl
   *   DD F4 = ld (hl+d),hl
   *   FD F4 = ld (iy+d),hl
   */
  switch(code) {
  case 0xC4:  regs.HL = get2( add_u16_nisp(regs.SP, fetch()) ); vc.rd+= 2; break;
  case 0xD4:  store2( add_u16_nisp(regs.SP, fetch()), regs.HL ); vc.wr+= 2; break;
  case 0xE4:  regs.HL = get2( add_u16_disp(regs.IX, fetch()) ); vc.rd+= 2; break;
  case 0xF4:  store2( add_u16_disp(regs.IX, fetch()), regs.HL ); vc.wr+= 2; break;
  default:
    return(resINV_INST);
  }
  
  return(resGO);
}

int cl_r2k::inst_r2k_ex (t_mem code) {
  u16_t tempw;
  
  switch(code) {
  case 0xE3:
    // EX DE', HL  on rabbit processors
    tempw = regs.aDE;
    regs.aDE = regs.HL;
    regs.HL = tempw;
    return(resGO);
    
  default:
    return(resINV_INST);
  }
}

int cl_r2k::inst_ljp(t_mem code) {
  u16_t  mn;
  
  mn = fetch2();  /* don't clobber PC before the fetch for xmem page */
  mmu.xpc = fetch1();
  PC = mn;
  
  return(resGO);
}

int cl_r2k::inst_lcall(t_mem code) {
  u16_t  mn;
  
  push1(mmu.xpc);
  push2(PC+2);
  vc.wr+= 2;
  
  mn = fetch2();  /* don't clobber PC before the fetch for xmem page */
  mmu.xpc = fetch1();
  PC = mn;
  
  return(resGO);
}

int cl_r2k::inst_lret(t_mem code)
{
  u16_t u16;
  u8_t u8;

  pop2(u16);
  pop1(u8);
  mmu.xpc= u8;
  PC= u16;
  
  return resGO;
}

int cl_r2k::inst_bool(t_mem code) {
  regs.raf.F &= ~BIT_ALL;
  if (regs.HL)
    regs.HL = 1;
  else
    regs.raf.F |= BIT_Z;
  return(resGO);
}

int cl_r2k::inst_r2k_and(t_mem code) {  // AND HL,DE
  regs.HL &= regs.DE;
  
  regs.raf.F &= ~BIT_ALL;
  if (regs.DE & 0x8000)
    regs.raf.F |= BIT_S;
  if (regs.DE == 0)
    regs.raf.F |= BIT_Z;
  if (word_parity(regs.DE))
    regs.raf.F |= BIT_P;
  return(resGO);
}

int cl_r2k::inst_r2k_or (t_mem code) {  // OR  HL,DE
  regs.HL |= regs.DE;
  
  regs.raf.F &= ~BIT_ALL;
  if (regs.DE & 0x8000)
    regs.raf.F |= BIT_S;
  if (regs.DE == 0)
    regs.raf.F |= BIT_Z;
  if (word_parity(regs.DE))
    regs.raf.F |= BIT_P;
  return(resGO);
}

int cl_r2k::inst_mul(t_mem code) {
  long m;
  long m1 = (long)(regs.BC & 0x7fff);
  long m2 = (long)(regs.DE & 0x7fff);
  if (regs.BC & 0x8000)
    m1 -= (1 << 15);
  if (regs.DE & 0x8000)
    m2 -= (1 << 15);
  m = m1 * m2;
  regs.BC = ((unsigned long)(m) & 0xffff);
  regs.HL = ((unsigned long)(m) >> 16) & 0xffff;
  return(resGO);
}

int cl_r2k::inst_rl_de(t_mem code) {
  unsigned int oldcarry = (regs.raf.F & BIT_C);
  
  regs.raf.F &= ~BIT_ALL;
  regs.raf.F |= (((regs.DE >> 15) & 1U) << BITPOS_C);
  regs.DE = (regs.DE << 1) | (oldcarry >> BITPOS_C);
  
  if (regs.DE & 0x8000)
    regs.raf.F |= BIT_S;
  if (regs.DE == 0)
    regs.raf.F |= BIT_Z;
  if (word_parity(regs.DE))
    regs.raf.F |= BIT_P;
  return(resGO);
}

int cl_r2k::inst_rr_de(t_mem code) {
  unsigned int oldcarry = (regs.raf.F & BIT_C);

  regs.raf.F &= ~BIT_ALL;
  regs.raf.F |= ((regs.DE & 1) << BITPOS_C);
  regs.DE = (regs.DE >> 1) | (oldcarry << (15 - BITPOS_C));
  
  if (regs.DE & 0x8000)
    regs.raf.F |= BIT_S;
  if (regs.DE == 0)
    regs.raf.F |= BIT_Z;
  if (word_parity(regs.DE))
    regs.raf.F |= BIT_P;
  return(resGO);
}

int cl_r2k::inst_rr_hl(t_mem code)    // RR HL
{
  unsigned int oldcarry = (regs.raf.F & BIT_C);
  
  regs.raf.F &= ~BIT_ALL;
  regs.raf.F |= ((regs.HL & 1) << BITPOS_C);
  regs.HL = (regs.HL >> 1) | (oldcarry << (15 - BITPOS_C));
  
  if (regs.HL & 0x8000)
    regs.raf.F |= BIT_S;
  if (regs.HL == 0)
    regs.raf.F |= BIT_Z;
  if (word_parity(regs.HL))
    regs.raf.F |= BIT_P;
  return(resGO);
}


int
cl_r2k::inst_rst(t_mem code)
{
  switch(code) {
    case 0xC7: // RST 0
      push2(PC+2);
      PC = iir + 0x00 * 2;
      vc.wr+= 2;
    break;
    case 0xCF: // RST 8
      return(resINV_INST);
    
    case 0xD7: // RST 10H
      push2(PC+2);
      PC = iir + 0x10 * 2;
      vc.wr+= 2;
    break;
    case 0xDF: // RST 18H
      push2(PC+2);
      PC = iir + 0x18 * 2;
      vc.wr+= 2;
    break;
    case 0xE7: // RST 20H
      push2(PC+2);
      PC = iir + 0x20 * 2;
      vc.wr+= 2;
    break;
    case 0xEF: // RST 28H
      //PC = 0x28;
      switch (regs.raf.A) {
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
    case 0xF7: // RST 30H
      return(resINV_INST);  // opcode is used for MUL on rabbit 2000+
    break;
    case 0xFF: // RST 38H
      push2(PC+2);
      PC = iir + 0x38 * 2;
      vc.wr+= 2;
    break;
    default:
      return(resINV_INST);
    break;
  }
  return(resGO);
}

int cl_r2k::inst_xd(t_mem prefix)
{
  u16_t  *regs_IX_OR_IY = (prefix==0xdd)?(&regs.IX):(&regs.IY);
  t_mem code;
  
  if (fetch(&code))
    return(resBREAKPOINT);

  switch (code) {

  case 0x64: // LDP (Ix),HL
    {
      u16_t u16= *regs_IX_OR_IY;
      t_addr al= ((regs.raf.A & 0xf) << 16) | u16;
      t_addr ah= ((regs.raf.A & 0xf) << 16) | ((u16+1)&0xffff);
      rom->write(al, regs.hl.l);
      rom->write(ah, regs.hl.h);
      vc.wr+= 2;
      break;
    }
    
  case 0x65: // LDP (mn),IX
    {
      u16_t ix= *regs_IX_OR_IY;
      u16_t u16= fetch2();
      t_addr al= ((regs.raf.A & 0xf) << 16) | u16;
      t_addr ah= ((regs.raf.A & 0xf) << 16) | ((u16+1)&0xffff);
      rom->write(al, ix&0xff);
      rom->write(ah, ix>>8);
      vc.wr+= 2;
      break;
    }
    
  case 0x6c: // LDP HL,(Ix)
    {
      u16_t u16= *regs_IX_OR_IY;
      t_addr al= ((regs.raf.A & 0xf) << 16) | u16;
      t_addr ah= ((regs.raf.A & 0xf) << 16) | ((u16+1)&0xffff);
      regs.hl.l= rom->read(al);
      regs.hl.h= rom->read(ah);
      vc.rd+= 2;
      break;
    }
    
  case 0x6d: // LDP IX,(mn)
    {
      u8_t l,h;
      u16_t u16= fetch2();
      t_addr al= ((regs.raf.A & 0xf) << 16) | u16;
      t_addr ah= ((regs.raf.A & 0xf) << 16) | ((u16+1)&0xffff);
      l= rom->read(al);
      h= rom->read(ah);
      *regs_IX_OR_IY= h*256+l;
      vc.rd+= 2;
      break;
    }
    
    // 0x06 LD A,(IX+A) is r4k+ instruction
  case 0x21: // LD IX,nnnn
  case 0x22: // LD (nnnn),IX
    
  case 0x2A: // LD IX,(nnnn)
  case 0x2E: // LD LX,nn
  case 0x36: // LD (IX+dd),nn
  case 0x46: // LD B,(IX+dd)
  case 0x4E: // LD C,(IX+dd)
  case 0x56: // LD D,(IX+dd)
  case 0x5E: // LD E,(IX+dd)
  case 0x66: // LD H,(IX+dd)
  case 0x6E: // LD L,(IX+dd)
    
  case 0x70: // LD (IX+dd),B
  case 0x71: // LD (IX+dd),C
  case 0x72: // LD (IX+dd),D
  case 0x73: // LD (IX+dd),E
  case 0x74: // LD (IX+dd),H
  case 0x75: // LD (IX+dd),L
  case 0x77: // LD (IX+dd),A
  case 0x7E: // LD A,(IX+dd)
  case 0xF9: // LD SP,IX
    if (prefix == 0xdd)
      return(inst_dd_ld(code));
    else
      return(inst_fd_ld(code));
    
  case 0x7C: // LD HL,IX
    regs.HL = *regs_IX_OR_IY;  // LD HL, IX|IY for rabbit processors
    return(resGO);
  case 0x7D: // LD IX,HL
    *regs_IX_OR_IY = regs.HL;   // LD IX|IY,HL for rabbit processors
    return(resGO);
    
  case 0x23: // INC IX
  case 0x34: // INC (IX+dd)
    if (prefix == 0xdd)
      return(inst_dd_inc(code));
    else
      return(inst_fd_inc(code));
    
  case 0x09: // ADD IX,BC
  case 0x19: // ADD IX,DE
  case 0x29: // ADD IX,IX
  case 0x39: // ADD IX,SP
  case 0x86: // ADD A,(IX)
    if (prefix == 0xdd)
      return(inst_dd_add(code));
    else
      return(inst_fd_add(code));
    
  case 0x2B: // DEC IX
  case 0x35: // DEC (IX+dd)
    if (prefix == 0xdd)
      return(inst_dd_dec(code));
    else
      return(inst_fd_dec(code));
    
    // 0x4C  TEST IX is r4k+

  case 0x8E: // ADC A,(IX)
  case 0x96: // SUB (IX+dd)
  case 0x9E: // SBC A,(IX+dd)
  case 0xA6: // AND (IX+dd)
  case 0xAE: // XOR (IX+dd)
  case 0xB6: // OR (IX+dd)
  case 0xBE: // CP (IX+dd)
    if (prefix == 0xdd)
      return(inst_dd_misc(code));
    else
      return(inst_fd_misc(code));
    
  case 0xC4: // LD IX,(SP+n)
    *regs_IX_OR_IY = get2( add_u16_nisp(regs.SP, fetch()) );
    vc.rd+= 2;
    return(resGO);
    
  case 0xCB: // escape, IX prefix to CB commands
    // fixme: limit the opcodes passed through to those officially
    // documented as present on the rabbit processors
    if (prefix == 0xdd)
      return(inst_ddcb()); /* see inst_ddcb.cc */
    else
      return(inst_fdcb()); /* see inst_fdcb.cc */
    
  case 0xCC: // BOOL IX|IY
    if (*regs_IX_OR_IY)
      *regs_IX_OR_IY = 1;
    
    // update flags
    regs.raf.F &= ~BIT_ALL;
    // bit 15 will never be set, so S<=0
    if (*regs_IX_OR_IY == 0)
      regs.raf.F |= BIT_Z;
    // L/V and C are always cleared
    return(resGO);
    
  case 0xD4: // LD (SP+n),IX|IY
    store2( add_u16_nisp(regs.SP, fetch()), *regs_IX_OR_IY );
    vc.wr+= 2;
    return(resGO);
    
  case 0xE1: // POP IX
    *regs_IX_OR_IY = get2(regs.SP);
    regs.SP+=2;
    vc.rd+= 2;
    return(resGO);
    
  case 0xE3: // EX (SP),IX
  {
    u16_t tempw;
    
    tempw = *regs_IX_OR_IY;
    *regs_IX_OR_IY = get2(regs.SP);
    store2(regs.SP, tempw);
    vc.rd+= 2;
    vc.wr+= 2;
  }
  return(resGO);
  
  case 0xE4:
    if (prefix == 0xDD)
      regs.HL = get2( add_u16_disp(regs.HL, fetch()) );
    else
      regs.HL = get2( add_u16_disp(regs.IY, fetch()) );
    vc.rd+= 2;
    return(resGO);
    
  case 0xE5: // PUSH IX
    push2(*regs_IX_OR_IY);
    vc.wr+= 2;
    return(resGO);
    
  case 0xE9: // JP (IX)
    PC = *regs_IX_OR_IY;
    return(resGO);
    
  case 0xEA:
    push2(PC);
    PC = *regs_IX_OR_IY;
    vc.wr+= 2;
    return(resGO);
    
  case 0xDC: // AND IX|IY,DE  for rabbit processors
  case 0xEC: // OR  IX|IY,DE  for rabbit processors
    if (code == 0xDC)
      *regs_IX_OR_IY &= regs.DE;
    else
      *regs_IX_OR_IY |= regs.DE;
    
    // update flags
    regs.raf.F &= ~BIT_ALL;
    if (*regs_IX_OR_IY & 0x8000)
      regs.raf.F |= BIT_S;
    if (regs_IX_OR_IY == 0)
      regs.raf.F |= BIT_Z;
    if (word_parity(*regs_IX_OR_IY))
      regs.raf.F |= BIT_P;
    return(resGO);
    
  case 0xF4: // LD (HL|IY+d),HL
    if (prefix == 0xDD)
      store2( add_u16_disp(regs.HL, fetch()), regs.HL );
    else
      store2( add_u16_disp(regs.IY, fetch()), regs.HL );
    vc.wr+= 2;
    return(resGO);
    
  case 0xFC: // RR IX|IY
  {
    u16_t  tmp = (regs.raf.F & BIT_C) << (15 - BITPOS_C);
    tmp |= (*regs_IX_OR_IY >> 1);
    
    regs.raf.F = (regs.raf.F & ~BIT_C) | ((*regs_IX_OR_IY & 1) << BITPOS_C);
    
    if (*regs_IX_OR_IY & 0x8000)
      regs.raf.F |= BIT_S;
    if (*regs_IX_OR_IY == 0)
      regs.raf.F |= BIT_Z;
    if (word_parity(*regs_IX_OR_IY))
      regs.raf.F |= BIT_P;
    return(resGO);
  }
  
  default:
    return(resINV_INST);
  }
  
  return(resINV_INST);
}
