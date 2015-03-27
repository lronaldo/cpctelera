/*
 * Simulated instructions specific to the LR35902, the Z-80 derivative used
 * in the gameboy.
 *
 * 2011-12-21  created by Leland Morrison
 *
 *

This file is part of microcontroller simulator: ucsim.

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

#include "lr35902cl.h"

static TYPE_UBYTE  swap_nibbles(TYPE_UBYTE  val) {
  return ((val >> 4) & 0x0f) | ((val << 4) & 0xf0);
}

int cl_lr35902::inst_cb(void) {
  TYPE_UBYTE  result;
  t_mem       code;
  
  if ( (peek1( ) & 0xf8) != 0x30 )
    return cl_z80::inst_cb( );
  
  code = fetch1();
  
  /* perform SWAP instead of slia */
  switch(code) {
  case 0x30: result = regs.bc.h = swap_nibbles(regs.bc.h); break; /* b */
  case 0x31: result = regs.bc.l = swap_nibbles(regs.bc.l); break; /* c */
  case 0x32: result = regs.de.h = swap_nibbles(regs.de.h); break; /* d */
  case 0x33: result = regs.de.l = swap_nibbles(regs.de.l); break; /* e */
  case 0x34: result = regs.hl.l = swap_nibbles(regs.hl.h); break; /* h */
  case 0x35: result = regs.hl.h = swap_nibbles(regs.hl.l); break; /* l */
  case 0x36: /* SWAP (HL) */
    {
      result = swap_nibbles(get1(regs.HL));
      store1(regs.HL, result);
    }
    break;
    
  case 0x37: result = regs.A = swap_nibbles(regs.A); break; /* swap a */
  default: return resINV_INST;
  }
  regs.F = (result)?0:0x80;  // all except zero are simply cleared
  return(resGO);
}

int cl_lr35902::inst_st_sp_abs(t_mem code) {
  if (code == 0x08) {
    TYPE_UWORD addr = fetch2( );
    store2( addr, regs.SP );
    return(resGO);
  }
  
  return resINV_INST;
}

int cl_lr35902::inst_stop0    (t_mem code) {
  // TODO: change to wait for a signal for simulated hardware
  return resHALT;
}

int cl_lr35902::inst_ldi   (t_mem code) {
  if (code == 0x22) {
    store1( regs.HL, regs.A );
    regs.HL ++;
    return resGO;
  } else if (code == 0x2A) {
    regs.A = get1( regs.HL );
    regs.HL ++;
    return resGO;
  }
  
  return resINV_INST;
}

int cl_lr35902::inst_ldd   (t_mem code) {
  if (code == 0x32) {
    store1( regs.HL, regs.A );
    regs.HL --;
    return resGO;
  } else if (code == 0x3A) {
    regs.A = get1( regs.HL );
    regs.HL --;
    return resGO;
  }
  
  return resINV_INST;
}

int cl_lr35902::inst_ldh   (t_mem code) {
  TYPE_UWORD addr = 0xFF00 + fetch1( );
  
  if (code == 0xE0) {
    store1( addr, regs.A );
    return resGO;
  } else if (code == 0xF0) {
    regs.A = get1( addr );
    return resGO;
  }
  
  return resINV_INST;
}
  
int cl_lr35902::inst_reti  (t_mem code) {
  /* enable interrupts */
  cl_z80::inst_ei(0xFB);
  
  /* pop2(PC); */
  PC=get2(regs.SP);
  regs.SP+=2;
  
  return resGO;
}

int cl_lr35902::inst_add_sp_d(t_mem code) {
  TYPE_UWORD  d = fetch( );
  /* sign-extend d from 8-bits to 16-bits */
  d |= (d>>7)*0xFF00;
  
  regs.F &= ~(BIT_ALL);  /* clear these */
  if ((regs.SP & 0x0FFF) + (d & 0x0FFF) > 0x0FFF)
    regs.F |= BIT_A;
  if (regs.SP + (int)(d) > 0xffff)
    regs.F |= BIT_C;
  
  regs.SP = (regs.SP + d) & 0xffff;

  return(resGO);
}

int cl_lr35902::inst_ld16  (t_mem code) {
  TYPE_UWORD addr = fetch2( );
  if (code == 0xEA) {
    store1( addr, regs.A );
    return resGO;
  } else if (code == 0xFA) {
    regs.A = get1( addr );
    return resGO;
  }
  
  return resINV_INST;
}

int cl_lr35902::inst_ldhl_sp (t_mem code) {
  TYPE_UWORD  d = fetch( );
  /* sign-extend d from 8-bits to 16-bits */
  d |= (d>>7)*0xFF00;

  regs.F &= ~(BIT_ALL);  /* clear these */
  if ((regs.SP & 0x0FFF) + (d & 0x0FFF) > 0x0FFF)
    regs.F |= BIT_A;
  if (regs.SP + (int)(d) > 0xffff)
    regs.F |= BIT_C;
  
  regs.HL = (regs.SP + d) & 0xffff;
  return resGO;
}

