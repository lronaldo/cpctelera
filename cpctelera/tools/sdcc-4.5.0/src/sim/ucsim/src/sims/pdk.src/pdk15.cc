/*
 * Simulator of microcontrollers (pdk15.cc)
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

#include "pdk15cl.h"


cl_fpp15::cl_fpp15(int aid, class cl_pdk *the_puc, class cl_sim *asim):
  cl_fpp14(aid, the_puc, asim)
{
  type= new struct cpu_entry;
  type->type= CPU_PDK15;
}

cl_fpp15::cl_fpp15(int aid, class cl_pdk *the_puc, struct cpu_entry *IType, class cl_sim *asim):
  cl_fpp14(aid, the_puc, IType, asim)
{
}


struct dis_entry *cl_fpp15::dis_tbl(void)
{
  return disass_pdk_15;
}


int
cl_fpp15::execute(unsigned int code)
{
  int write_result = resGO;
  if (code == 0x0000) {
    // nop
  } else if (CODE_MASK(0x0200, 0xFF)) {
    // ret k
    cA.W(code);
    cSP->W(rSP - 2);
    PC = get_mem(rSP) | (get_mem(rSP + 1) << 8);
  } else if (code == 0x007A) {
    // ret
    /*write_result = store_io(0x2, rSP - 2);
    if (write_result == resGO)
    PC = get_mem(rSP) | (get_mem(rSP + 1) << 8);*/
    cSP->W(rSP-2);
    PC= rd16(rSP);
  } else if (CODE_MASK(0x5700, 0xFF)) {
    // mov a, k
    cA.W(code/* & 0xFF*/);
  } else if (CODE_MASK(0x0100, 0x7F)) {
    // mov io, a
    sfr->write(code & 0x7F, rA);
  } else if (CODE_MASK(0x0180, 0x7F)) {
    // mov a, io
    cA.W(get_io(code & 0x7F));
  } else if (CODE_MASK(0x1700, 0xFF)) {
    // mov m, a
    wr8(code & 0xFF, rA);
  } else if (CODE_MASK(0x1F00, 0xFF)) {
    // mov a, m
    cA.W(get_mem(code & 0xFF));
  } else if (CODE_MASK(0x0601, 0xFE)) {
    // ldt16
    wr16(code & 0x00fe, puc?(puc->t16->cnt):0);
  } else if (CODE_MASK(0x0600, 0xFE)) {
    // stt16
    if (puc)
      puc->t16->cnt= rd16(code & 0x00fe);
  } else if ((CODE_MASK(0x701, 0xFE))) {
    // idxm a, m
    cA.W(rd8(rd8(code & 0xFE)));
    tick(1);
  } else if ((CODE_MASK(0x700, 0xFE))) {
    // idxm m, a
    wr8(rd8(code & 0xFE), rA);
    tick(1);
  } else if (CODE_MASK(0x2700, 0xFF)) {
    // xch m
    int mem = rd8(code & 0xFF);
    wr8(code & 0xFF, rA);
    cA.W(mem);
  } else if (code == 0x0072) {
    // pushaf
    /*ram->write(rSP, rA);
    ram->write(rSP + 1, rF);
    //write_result = store_io(0x2, rSP + 2);
    cSP->W(rSP+2);*/
    pushlh(rA, rF);
  } else if (code == 0x0073) {
    // popaf
    /*cF->W(get_mem(rSP - 1));
    cA.W(get_mem(rSP - 2));
    //write_result = store_io(0x2, rSP - 2);
    cSP->W(rSP-2);*/
    cF->W(pop());
    cA.W(pop());
  } else if (CODE_MASK(0x5000, 0xFF)) {
    // add a, k
    cA.W(add_to(rA, code & 0xFF));
  } else if (CODE_MASK(0x1800, 0xFF)) {
    // add a, m
    cA.W(add_to(rA, rd8(code & 0xFF)));
  } else if (CODE_MASK(0x1000, 0xFF)) {
    // add m, a
    int addr = code & 0xFF;
    wr8(addr, add_to(rA, get_mem(addr)));
  } else if (CODE_MASK(0x5100, 0xFF)) {
    // sub a, k
    cA.W(sub_to(rA, code & 0xFF));
  } else if (CODE_MASK(0x1900, 0xFF)) {
    // sub a, m
    cA.W(sub_to(rA, get_mem(code & 0xFF)));
  } else if (CODE_MASK(0x1100, 0xFF)) {
    // sub m, a
    int addr = code & 0xFF;
    wr8(addr, sub_to(get_mem(addr), rA));
  } else if (CODE_MASK(0x1A00, 0xFF)) {
    // addc a, m
    cA.W(add_to(rA, get_mem(code & 0xFF), fC));
  } else if (CODE_MASK(0x1200, 0xFF)) {
    // addc m, a
    int addr = code & 0xFF;
    wr8(addr, add_to(rA, get_mem(addr), fC));
  } else if (code == 0x0060) {
    // addc a
    cA.W(add_to(rA, fC));
  } else if (CODE_MASK(0x2000, 0xFF)) {
    // addc m
    int addr = code & 0xFF;
    wr8(addr, add_to(get_mem(addr), fC));
  } else if (CODE_MASK(0x1B00, 0xFF)) {
    // subc a, m
    cA.W(sub_to(rA, get_mem(code & 0xFF), fC));
  } else if (CODE_MASK(0x1300, 0xFF)) {
    // subc m, a
    int addr = code & 0xFF;
    wr8(addr, sub_to(get_mem(addr), rA, fC));
  } else if (code == 0x0061) {
    // subc a
    cA.W(sub_to(rA, fC));
  } else if (CODE_MASK(0x2100, 0xFF)) {
    // subc m
    int addr = code & 0xFF;
    wr8(addr, sub_to(get_mem(addr), fC));
  } else if (CODE_MASK(0x2400, 0xFF)) {
    // inc m
    int addr = code & 0xFF;
    wr8(addr, add_to(get_mem(addr), 1));
  } else if (CODE_MASK(0x2500, 0xFF)) {
    // dec m
    int addr = code & 0xFF;
    wr8(addr, sub_to(get_mem(addr), 1));
  } else if (CODE_MASK(0x2600, 0xFF)) {
    // clear m
    wr8(code & 0xFF, 0);
  } else if (code == 0x006A) {
    // sr a
    store_flag(flag_c, rA & 1);
    cA.W(rA >>= 1);
  } else if (CODE_MASK(0x2A00, 0xFF)) {
    // sr m
    int value = get_mem(code & 0xFF);
    SETC(value & 1);
    wr8(code & 0xFF, value >> 1);
  } else if (code == 0x006B) {
    // sl a
    store_flag(flag_c, (rA & 0x80) /*>> 7*/);
    cA.W(rA << 1);
  } else if (CODE_MASK(0x2B00, 0xFF)) {
    // sl m
    int value = get_mem(code & 0xFF);
    SETC((value & 0x80)/* >> 7*/);
    wr8(code & 0xFF, value << 1);
  } else if (code == 0x006C) {
    // src a
    int c = rA & 1;
    rA >>= 1;
    cA.W(rA |= fC << 7);
    store_flag(flag_c, c);
  } else if (CODE_MASK(0x2C00, 0xFF)) {
    // src m
    int value = get_mem(code & 0xFF);
    int c = value & 1;
    wr8(code & 0xFF, (value >> 1) | (fC << 7));
    SETC(c);
  } else if (code == 0x006D) {
    // slc a
    int c = (rA & 0x80) /*>> 7*/;
    rA <<= 1;
    cA.W(rA | fC);
    store_flag(flag_c, c);
  } else if (CODE_MASK(0x2D00, 0xFF)) {
    // slc m
    int value = get_mem(code & 0xFF);
    int c = (value & 0x80)/* >> 7*/;
    wr8(code & 0xFF, (value << 1) | fC);
    SETC(c);
  } else if (CODE_MASK(0x5400, 0xFF)) {
    // and a, k
    cA.W(rA & code & 0xFF);
    SETZ(!rA);
  } else if (CODE_MASK(0x1C00, 0xFF)) {
    // and a, m
    cA.W(rA & get_mem(code & 0xFF));
    SETZ(!rA);
  } else if (CODE_MASK(0x1400, 0xFF)) {
    // and m, a
    int store = rA & get_mem(code & 0xFF);
    SETZ(!store);
    wr8(code & 0xFF, store);
  } else if (CODE_MASK(0x5500, 0xFF)) {
    // or a, k
    cA.W(rA | (code & 0xFF));
    SETZ(!rA);
  } else if (CODE_MASK(0x1D00, 0xFF)) {
    // or a, m
    cA.W(rA | get_mem(code & 0xFF));
    SETZ(!rA);
  } else if (CODE_MASK(0x1500, 0xFF)) {
    // or m, a
    int store = rA | get_mem(code & 0xFF);
    SETZ(!store);
    wr8(code & 0xFF, store);
  } else if (CODE_MASK(0x5600, 0xFF)) {
    // xor a, k
    cA.W(rA ^ (code & 0xFF));
    SETZ(!rA);
  } else if (CODE_MASK(0x1E00, 0xFF)) {
    // xor a, m
    cA.W(rA ^ get_mem(code & 0xFF));
    SETZ(!rA);
  } else if (CODE_MASK(0x1600, 0xFF)) {
    // xor m, a
    int store = rA ^ get_mem(code & 0xFF);
    SETZ(!store);
    wr8(code & 0xFF, store);
  } else if (CODE_MASK(0x0080, 0x7F)) {
    // xor io, a
    write_result = store_io(code & 0x7F, rA ^ get_io(code & 0x7F));
  } else if (code == 0x0068) {
    // not a
    rA = ~rA;
    /*store_flag(flag_z, */SETZ(!rA);
  } else if (CODE_MASK(0x2800, 0xFF)) {
    // not m
    int store = (~get_mem(code & 0xFF) & 0xFF);
    SETZ(!store);
    wr8(code & 0xFF, store);
  } else if (code == 0x0069) {
    // neg a
    cA.W(-rA);
    /*store_flag(flag_z, */SETZ(!rA);
  } else if (CODE_MASK(0x2900, 0xFF)) {
    // neg m
    int store = (-get_mem(code & 0xFF) & 0xFF);
    SETZ(!store);
    wr8(code & 0xFF, store);
  } else if (CODE_MASK(0x3800, 0x3FF)) {
    // set0 io, k
    const u8_t bit = (code & 0x380) >> 7;
    const u8_t addr = code & 0x7F;
    write_result = store_io(addr, get_io(addr) & ~(1 << bit));
  } else if (CODE_MASK(0x4800, 0x3FF)) {
    // set0 m, k
    const u8_t bit = (code & 0x380) >> 7;
    const u8_t addr = code & 0x7F;
    wr8(addr, get_mem(addr) & ~(1 << bit));
  } else if (CODE_MASK(0x3C00, 0x3FF)) {
    // set1 io, k
    const u8_t bit = (code & 0x380) >> 7;
    const u8_t addr = code & 0x7F;
    write_result = store_io(addr, get_io(addr) | (1 << bit));
  } else if (CODE_MASK(0x4C00, 0x3FF)) {
    // set1 m, k
    const u8_t bit = (code & 0x380) >> 7;
    const u8_t addr = code & 0x7F;
    wr8(addr, get_mem(addr) | (1 << bit));
  } else if (CODE_MASK(0x3000, 0x3FF)) {
    // t0sn io, k
    int n = (code & 0x380) >> 7;
    if (!(get_io(code & 0x7F) & (1 << n)))
      ++PC;
  } else if (CODE_MASK(0x4000, 0x3FF)) {
    // t0sn m, k
    int n = (code & 0x380) >> 7;
    if (!(get_mem(code & 0x7F) & (1 << n)))
      ++PC;
  } else if (CODE_MASK(0x3400, 0x3FF)) {
    // t1sn io, k
    int n = (code & 0x380) >> 7;
    if (get_io(code & 0x7F) & (1 << n))
      ++PC;
  } else if (CODE_MASK(0x4400, 0x3FF)) {
    // t1sn m, k
    int n = (code & 0x380) >> 7;
    if (get_mem(code & 0x7F) & (1 << n))
      ++PC;
  } else if (CODE_MASK(0x5200, 0xFF)) {
    // ceqsn a, k
    sub_to(rA, code & 0xFF);
    if (rA == (code & 0xFF))
      ++PC;
  } else if (CODE_MASK(0x2E00, 0xFF)) {
    // ceqsn a, m
    int addr = code & 0xFF;
    u8_t value= get_mem(addr); 
    sub_to(rA, value);
    if (rA == value)
      ++PC;
  } else if (CODE_MASK(0x5300, 0xFF)) {
    // cneqsn a, k
    sub_to(rA, code & 0xFF);
    if (rA != (code & 0xFF))
      ++PC;
  } else if (CODE_MASK(0x2F00, 0xFF)) {
    // cneqsn a, m
    int addr = code & 0xFF;
    u8_t value= get_mem(addr);
    sub_to(rA, value);
    if (rA != value)
      ++PC;
  } else if (code == 0x0062) {
    // izsn
    cA.W(add_to(rA, 1));
    if (!rA)
      ++PC;
  } else if (CODE_MASK(0x2200, 0xFF)) {
    // izsn m
    const int addr = code & 0xFF;
    int result = add_to(get_mem(addr), 1);
    ram->write(addr, result);
    if (!result)
      ++PC;
  } else if (code == 0x0063) {
    // dzsn
    cA.W(sub_to(rA, 1));
    if (!rA)
      ++PC;
  } else if (CODE_MASK(0x2300, 0xFF)) {
    // dzsn m
    const int addr = code & 0xFF;
    int result = sub_to(get_mem(addr), 1);
    ram->write(addr, result);
    if (!result)
      ++PC;
  } else if (CODE_MASK(0x7000, 0xFFF)) {
    // call k
    ram->write(rSP, PC);
    ram->write(rSP + 1, PC >> 8);
    PC = code & 0xFFF;
    write_result = store_io(0x2, rSP + 2);
    tick(1);
  } else if (CODE_MASK(0x6000, 0xFFF)) {
    // goto k
    PC = code & 0xFFF;
    tick(1);
  } else if (CODE_MASK(0x0C00, 0xFF)) {
    // comp a, m
    sub_to(rA, get_mem(code & 0xFF));
  } else if (CODE_MASK(0x0D00, 0xFF)) {
    // comp m, a
    sub_to(get_mem(code & 0xFF), rA);
  } else if (CODE_MASK(0x0E00, 0xFF)) {
    // nadd a, m
    cA.W(add_to(get_mem(code & 0xFF), -rA));
  } else if (CODE_MASK(0x0F00, 0xFF)) {
    // nadd m, a
    int addr = code & 0xFF;
    wr8(addr, add_to(-get_mem(addr), rA));
  } else if (code == 0x006E) {
    // swap
    int high = rA & 0xF;
    cA.W((high << 4) | (rA >> 4));
  } else if (code == 0x0067) {
    // pcadd
    PC += rA - 1;
  } else if (code == 0x0078) {
    // TODO: engint
  } else if (code == 0x0079) {
    // TODO: disgint
  }
  else if (code == 0x0076) {
    // stopsys
    if (puc) puc->mode= pm_pd;
  } else if (code == 0x0077) {
    // stopexe
    if (puc) puc->mode= pm_ps;
  } else if (code == 0x0075) {
    // reset
    reset();
  } else if (code == 0x0070) {
    // wdreset
    if (puc) puc->wdt->clear();
  } else if ((code & 0xfc00) == 0x5c00) {
    // swapc IO, k
    int c, a= code & 0x7f, n= (code>>7)&7, m=1<<n;
    u8_t d= sfr->read(a);
    c= d & m;
    fC?(d|=m):(d&=~m);
    SETC(c);
    sfr->write(a, d);
  }
  else if (code == 0x0006) {
    // ldsptl
    cA.W(rom->read(rSP));
    vc.rd++;
  } else if (code == 0x0007) {
    // ldspth
    cA.W(rom->read(rSP+1));
    vc.rd++;
  } else if (code == 0x007C) {
    // mul
    unsigned result = rA * get_io(0x08);
    cA.W(result);
    if (puc) puc->rMULRH= result >> 8;
  } 
  else {
    return (resINV_INST);
  }
  return (resGO);
}


/* End of pdk.src/pdk15.cc */
