/*
 * Simulator of microcontrollers (pdk13.cc)
 *
 * Copyright (C) 2016 Drotos Daniel
 * 
 * To contact author send email to dr.dkdb@gmail.com
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

#include "glob.h"

#include "t16cl.h"
#include "wdtcl.h"

#include "pdk13cl.h"


cl_fpp13::cl_fpp13(int aid, class cl_pdk *the_puc, class cl_sim *asim):
  cl_fpp(aid, the_puc, asim)
{
  type= new struct cpu_entry;
  type->type= CPU_PDK13;
}

cl_fpp13::cl_fpp13(int aid, class cl_pdk *the_puc, struct cpu_entry *IType, class cl_sim *asim):
  cl_fpp(aid, the_puc, IType, asim)
{
}


struct dis_entry *cl_fpp13::dis_tbl(void)
{
  return disass_pdk_13;
}


int
cl_fpp13::execute(unsigned int code)
{
  int write_result = resGO;
  if (code == 0x0000) {
    // nop
  } else if (CODE_MASK(0x0100, 0xFF)) {
    // ret k
    cA.W(code & 0xFF);
    cSP->W(rSP-2);
    PC = get_mem(rSP) | (get_mem(rSP + 1) << 8);
  } else if (code == 0x003A) {
    // ret
    cSP->W(rSP-2);
    PC = get_mem(rSP) | (get_mem(rSP + 1) << 8);
  } else if (CODE_MASK(0x1700, 0xFF)) {
    // mov a, k
    cA.W(code);
  } else if (CODE_MASK(0x0080, 0x1F)) {
    // mov io, a
    sfr->write(code&0x1f, rA);
  } else if (CODE_MASK(0x00A0, 0x1F)) {
    // mov a, io
    cA.W(get_io(code & 0x1F));
  } else if (CODE_MASK(0x05C0, 0x3F)) {
    // mov m, a
    wr8(code & 0x3F, rA);
  } else if (CODE_MASK(0x07C0, 0x3F)) {
    // mov a, m
    cA.W(get_mem(code & 0x3F));
  } else if (CODE_MASK(0x00C1, 0x1E)) {
    // ldt16
    wr16(code & 0x001e, puc?(puc->t16->cnt):0);
  } else if (CODE_MASK(0x00C0, 0x1E)) {
    // stt16
    if (puc)
      puc->t16->cnt= rd16(code & 0x001e);
  } else if ((CODE_MASK(0x0E1, 0x1E))) {
    // idxm a, m
    cA.W(get_mem(get_mem(code & 0x1E)));
  } else if ((CODE_MASK(0x0E0, 0x1E))) {
    // idxm m, a
    wr8(get_mem(code & 0x1E), rA);
  } else if (CODE_MASK(0x09C0, 0x3F)) {
    // xch m
    int mem = get_mem(code & 0x3F);
    wr8(code & 0x3F, rA);
    cA.W(mem);
  } else if (code == 0x0032) {
    // pushaf
    ram->write(rSP, rA);
    ram->write(rSP + 1, rF);
    cSP->W(rSP + 2);
  } else if (code == 0x0033) {
    // popaf
    cF->W(get_mem(rSP - 1));
    cA.W(get_mem(rSP - 2));
    cSP->W(rSP - 2);
  } else if (CODE_MASK(0x1000, 0xFF)) {
    // add a, k
    cA.W(add_to(rA, code & 0xFF));
  } else if (CODE_MASK(0x0600, 0x3F)) {
    // add a, m
    cA.W(add_to(rA, get_mem(code & 0x3F)));
  } else if (CODE_MASK(0x0400, 0x3F)) {
    // add m, a
    int addr = code & 0x3F;
    wr8(addr, add_to(rA, get_mem(addr)));
  } else if (CODE_MASK(0x1100, 0xFF)) {
    // sub a, k
    cA.W(sub_to(rA, code & 0xFF));
  } else if (CODE_MASK(0x0640, 0x3F)) {
    // sub a, m
    cA.W(sub_to(rA, get_mem(code & 0x3F)));
  } else if (CODE_MASK(0x0440, 0x3F)) {
    // sub m, a
    int addr = code & 0x3F;
    wr8(addr, sub_to(get_mem(addr), rA));
  } else if (CODE_MASK(0x0680, 0x3F)) {
    // addc a, m
    cA.W(add_to(rA, get_mem(code & 0x3F), fC));
  } else if (CODE_MASK(0x0480, 0x3F)) {
    // addc m, a
    int addr = code & 0x3F;
    wr8(addr, add_to(rA, get_mem(addr), fC));
  } else if (code == 0x0010) {
    // addc a
    cA.W(add_to(rA, fC));
  } else if (CODE_MASK(0x0800, 0x3F)) {
    // addc m
    int addr = code & 0x3F;
    wr8(addr, add_to(get_mem(addr), fC));
  } else if (CODE_MASK(0x06C0, 0x3F)) {
    // subc a, m
    cA.W(sub_to(rA, get_mem(code & 0x3F), fC));
  } else if (CODE_MASK(0x04C0, 0x3F)) {
    // subc m, a
    int addr = code & 0x3F;
    wr8(addr, sub_to(get_mem(addr), rA, fC));
  } else if (code == 0x0011) {
    // subc a
    cA.W(sub_to(rA, fC));
  } else if (CODE_MASK(0x0840, 0x3F)) {
    // subc m
    int addr = code & 0x3F;
    wr8(addr, sub_to(get_mem(addr), fC));
  } else if (CODE_MASK(0x0900, 0x3F)) {
    // inc m
    int addr = code & 0x3F;
    wr8(addr, add_to(get_mem(addr), 1));
  } else if (CODE_MASK(0x0940, 0x3F)) {
    // dec m
    int addr = code & 0x3F;
    wr8(addr, sub_to(get_mem(addr), 1));
  } else if (CODE_MASK(0x0980, 0x3F)) {
    // clear m
    wr8(code & 0x3F, 0);
  } else if (code == 0x001A) {
    // sr a
    SETC(rA & 1);
    cA.W(rA >> 1);
  } else if (CODE_MASK(0x0A80, 0x3F)) {
    // sr m
    int value = get_mem(code & 0x3F);
    SETC(value & 1);
    wr8(code & 0x3F, value >> 1);
  } else if (code == 0x001B) {
    // sl a
    SETC(rA & 0x80);
    cA.W(rA << 1);
  } else if (CODE_MASK(0x0AC0, 0x3F)) {
    // sl m
    int value = get_mem(code & 0x3F);
    SETC(value & 0x80);
    wr8(code & 0x3F, value << 1);
  } else if (code == 0x001C) {
    // src a
    int c = rA & 1;
    rA >>= 1;
    cA.W(rA | (fC << 7));
    SETC(c);
  } else if (CODE_MASK(0x0B00, 0x3F)) {
    // src m
    int value = get_mem(code & 0x3F);
    int c = value & 1;
    wr8(code & 0x3F, (value >> 1) | (fC << 7));
    SETC(c);
  } else if (code == 0x001D) {
    // slc a
    int c = (rA & 0x80) >> 7;
    rA <<= 1;
    cA.W(rA | fC);
    SETC(c);
  } else if (CODE_MASK(0x0B40, 0x3F)) {
    // slc m
    int value = get_mem(code & 0x3F);
    int c = (value & 0x80) >> 7;
    wr8(code & 0x3F, (value << 1) | fC);
    SETC(c);
  } else if (CODE_MASK(0x1400, 0xFF)) {
    // and a, k
    cA.W(rA & code);
    SETZ(!rA);
  } else if (CODE_MASK(0x0700, 0x3F)) {
    // and a, m
    cA.W(rA & get_mem(code & 0x3F));
    SETZ(!rA);
  } else if (CODE_MASK(0x0500, 0x3F)) {
    // and m, a
    int store = rA & get_mem(code & 0x3F);
    SETZ(!store);
    wr8(code & 0x3F, store);
  } else if (CODE_MASK(0x1500, 0xFF)) {
    // or a, k
    cA.W(rA | code);
    SETZ(!rA);
  } else if (CODE_MASK(0x0740, 0x3F)) {
    // or a, m
    cA.W(rA | get_mem(code & 0x3F));
    SETZ(!rA);
  } else if (CODE_MASK(0x0540, 0x3F)) {
    // or m, a
    int store = rA | get_mem(code & 0x3F);
    SETZ(!store);
    wr8(code & 0x3F, store);
  } else if (CODE_MASK(0x1600, 0xFF)) {
    // xor a, k
    cA.W(rA ^ code);
    SETZ(!rA);
  } else if (CODE_MASK(0x0780, 0x3F)) {
    // xor a, m
    cA.W(rA ^ get_mem(code & 0x3F));
    SETZ(!rA);
  } else if (CODE_MASK(0x0580, 0x3F)) {
    // xor m, a
    int store = rA ^ get_mem(code & 0x3F);
    SETZ(!store);
    wr8(code & 0x3F, store);
  } else if (CODE_MASK(0x0060, 0x1F)) {
    // xor io, a
    sfr->write(code & 0x1F, rA ^ get_io(code & 0x1F));
  } else if (code == 0x0018) {
    // not a
    cA.W(~rA);
    SETZ(!rA);
  } else if (CODE_MASK(0x0A00, 0x3F)) {
    // not m
    int store = (~get_mem(code & 0x3F) & 0xff);
    SETZ(!store);
    wr8(code & 0x3F, store);
  } else if (code == 0x0019) {
    // neg a
    cA.W(-rA);
    SETZ(!rA);
  } else if (CODE_MASK(0x0A40, 0x3F)) {
    // neg m
    int store = (-get_mem(code & 0x3F) & 0xff);
    SETZ(!store);
    wr8(code & 0x3F, store);
  } else if (CODE_MASK(0x0E00, 0xFF)) {
    // set0 io, k
    const u8_t bit = (code & 0xE0) >> 5;
    const u8_t addr = code & 0x1F;
    sfr->write(addr, get_io(addr) & ~(1 << bit));
  } else if (CODE_MASK(0x0300, 0xEF)) {
    // set0 m, k
    const u8_t bit = (code & 0xE0) >> 5;
    const u8_t addr = code & 0x0F;
    wr8(addr, get_mem(addr) & ~(1 << bit));
  } else if (CODE_MASK(0x0F00, 0xFF)) {
    // set1 io, k
    const u8_t bit = (code & 0xE0) >> 5;
    const u8_t addr = code & 0x1F;
    sfr->write(addr, get_io(addr) | (1 << bit));
  } else if (CODE_MASK(0x0310, 0xEF)) {
    // set1 m, k
    const u8_t bit = (code & 0xE0) >> 5;
    const u8_t addr = code & 0x0F;
    wr8(addr, get_mem(addr) | (1 << bit));
  } else if (CODE_MASK(0x0C00, 0xFF)) {
    // t0sn io, k
    int n = (code & 0xE0) >> 5;
    if (!(get_io(code & 0x1F) & (1 << n)))
      ++PC;
  } else if (CODE_MASK(0x0200, 0xEF)) {
    // t0sn m, k
    int n = (code & 0xE0) >> 5;
    if (!(get_mem(code & 0x0F) & (1 << n)))
      ++PC;
  } else if (CODE_MASK(0x0D00, 0xFF)) {
    // t1sn io, k
    int n = (code & 0xE0) >> 5;
    if (get_io(code & 0x1F) & (1 << n))
      ++PC;
  } else if (CODE_MASK(0x0210, 0xEF)) {
    // t1sn m, k
    int n = (code & 0xE0) >> 5;
    if (get_mem(code & 0x0F) & (1 << n))
      ++PC;
  } else if (CODE_MASK(0x1200, 0xFF)) {
    // ceqsn a, k
    sub_to(rA, code & 0xFF);
    if (rA == (code & 0xFF))
      ++PC;
  } else if (CODE_MASK(0x0B80, 0x3F)) {
    // ceqsn a, m
    int addr = code & 0x3F;
    sub_to(rA, get_mem(addr));
    if (rA == get_mem(addr))
      ++PC;
  } else if (code == 0x0012) {
    // izsn
    rA = add_to(rA, 1);
    if (!rA)
      ++PC;
  } else if (CODE_MASK(0x0880, 0x3F)) {
    // izsn m
    const int addr = code & 0x3F;
    int result = add_to(get_mem(addr), 1);
    wr8(addr, result);
    if (!result)
      ++PC;
  } else if (code == 0x0013) {
    // dzsn
    cA.W(sub_to(rA, 1));
    if (!rA)
      ++PC;
  } else if (CODE_MASK(0x08C0, 0x3F)) {
    // dzsn m
    const int addr = code & 0x3F;
    int result = sub_to(get_mem(addr), 1);
    wr8(addr, result);
    if (!result)
      ++PC;
  } else if (CODE_MASK(0x1C00, 0x3FF)) {
    // call k
    push(PC);
    PC = code & 0x3FF;
    tick(1);
  } else if (CODE_MASK(0x1800, 0x3FF)) {
    // goto k
    PC = code & 0x3FF;
    tick(1);
  } else if (code == 0x001E) {
    // swap
    int high = rA & 0xF;
    cA.W((high << 4) | (rA >> 4));
  } else if (code == 0x0017) {
    // pcadd
    PC += rA - 1;
  } else if (code == 0x0038) {
    // TODO: engint
  } else if (code == 0x0039) {
    // TODO: disint
  }
  else if (code == 0x0036) {
    // stopsys
    if (puc) puc->mode= pm_pd;
  }
  else if (code == 0x0037) {
    // stopexe
    if (puc) puc->mode= pm_ps;
  }
  else if (code == 0x0035) {
    // reset
    reset();
  }
  else if (code == 0x0030) {
    // wdreset
    if (puc) puc->wdt->clear();
  }
  else if (code == 0x0006) {
    // ldsptl
    cA.W(rom->read(rSP) & 0xFF);
  } else if (code == 0x0007) {
    // ldregs[0x02]th
    //rA = (rom->get(rSP) & 0xFF00) >> 8;
    cA.W(rom->read(rSP+1));
  } else if (code == 0x003C) {
    // mul
    unsigned result = rA * get_io(0x08);
    cA.W(result & 0xFF);
    if (puc) puc->rMULRH= result >> 8;
  } else {
    return (resINV_INST);
  }
  return (resGO);
}


/* End of pdk.src/pdk13.cc */
