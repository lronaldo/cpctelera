/*
 * Simulator of microcontrollers (inst.cc)
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

#include <stdlib.h>
#include <cassert>
#include "ddconfig.h"
#include "stdio.h"

// local
#include "pdkcl.h"
#include "regspdk.h"

#define CODE_MASK(op, m) ((code & ~(m)) == (op))

unsigned char cl_pdk::add_to(unsigned char initial, int value, bool carry) {
  store_flag(flag_z, initial + value + carry == 0);
  store_flag(flag_c, initial + value + carry > 0xFF);
  store_flag(flag_ac, (initial & 0xF) + (value & 0xF) + carry > 0xF);
  store_flag(
      flag_ov,
      get_flag(flag_c) ^ ((initial & 0x7F) + (value & 0x7F) + carry > 0x7F));

  return initial + value + carry;
}

unsigned char cl_pdk::sub_to(unsigned char initial, int value, bool carry) {
  store_flag(flag_z, initial - value - carry == 0);
  store_flag(flag_c, initial < value + carry);
  store_flag(flag_ac, (value & 0xF) > (initial & 0xF) - carry);
  store_flag(
      flag_ov,
      get_flag(flag_c) ^ ((initial & 0x7F) - (value & 0x7F) - carry < 0));

  return initial - value - carry;
}

int cl_pdk::get_mem(unsigned int addr) {
  vc.rd++;
  return ram->read((t_addr)(addr));
}

unsigned char cl_pdk::get_io(t_addr addr) {
  return regs8->get(addr);
}

void cl_pdk::store_io(t_addr addr, unsigned char value) {
  regs8->set(addr, value);
  if (get_SP() > sp_max)
    sp_max = get_SP();
}

unsigned char cl_pdk::get_SP() {
    return get_io(0x02);
}

unsigned char cl_pdk::get_flags() {
    return get_io(0x00);
}

void cl_pdk::set_flags(unsigned char flags) {
    return store_io(0x00, flags);
}

int cl_pdk::get_flag(flag n) {
  switch (n) {
  case flag_z: return get_flags() & BIT_Z;
  case flag_c: return (get_flags() & BIT_C) >> 1;
  case flag_ac: return (get_flags() & BIT_AC) >> 2;
  case flag_ov: return (get_flags() & BIT_OV) >> 3;
  default:
    assert(!"invalid bit access to FLAG");
  }
  return 0;
}

void cl_pdk::store_flag(flag n, int value) {
  switch (n) {
  case flag_z: set_flags((get_flags() & ~1) | value); break;
  case flag_c: set_flags((get_flags() & ~2) | (value << 1)); break;
  case flag_ac: set_flags((get_flags() & ~4) | (value << 2)); break;
  case flag_ov: set_flags((get_flags() & ~8) | (value << 3)); break;
  default:
    assert(!"invalid bit store to FLAG");
  }
}

int cl_pdk::execute(unsigned int code) {
  switch (type->type) {
  case CPU_PDK13:
    return(execute_pdk13(code));
  case CPU_PDK14:
    return(execute_pdk14(code));
  case CPU_PDK15:
    return(execute_pdk15(code));
  default:
    return 0;//__builtin_unreachable();
  }
}

int cl_pdk::execute_pdk14(unsigned int code) {
  if (code == 0x0000) {
    // nop
  } else if (CODE_MASK(0x0200, 0xFF)) {
    // ret k
    regs.a = code & 0xFF;
    store_io(0x2, get_SP() - 2);
    PC = get_mem(get_SP()) | (get_mem(get_SP() + 1) << 8);
  } else if (code == 0x007A) {
    // ret
    store_io(0x2, get_SP() - 2);
    PC = get_mem(get_SP()) | (get_mem(get_SP() + 1) << 8);
  } else if (CODE_MASK(0x2F00, 0xFF)) {
    // mov a, k
    regs.a = code & 0xFF;
  } else if (CODE_MASK(0x0180, 0x3F)) {
    // mov i, a
    store_io(code & 0x3F, regs.a);
  } else if (CODE_MASK(0x01C0, 0x3F)) {
    // mov a, i
    regs.a = get_io(code & 0x3F);
  } else if (CODE_MASK(0x0B80, 0x7F)) {
    // mov m, a
    ram->write(code & 0x7F, regs.a);
  } else if (CODE_MASK(0x0F80, 0x7F)) {
    // mov a, m
    regs.a = get_mem(code & 0x7F);
  } else if (CODE_MASK(0x0301, 0x7E)) {
    // TODO: ldt16
  } else if (CODE_MASK(0x0300, 0x7E)) {
    // TODO: stt16
  } else if ((CODE_MASK(0x381, 0x7E))) {
    // idxm a, m
    regs.a = get_mem(get_mem(code & 0x7E));
  } else if ((CODE_MASK(0x380, 0x7E))) {
    // idxm m, a
    ram->write(get_mem(code & 0x7E), regs.a);
  } else if (CODE_MASK(0x1380, 0x7F)) {
    // xch m
    int mem = get_mem(code & 0x7F);
    ram->write(code & 0x7F, regs.a);
    regs.a = mem;
  } else if (code == 0x0072) {
    // pushaf
    ram->write(get_SP(), regs.a);
    ram->write(get_SP() + 1, get_flags());
    store_io(0x2, get_SP() + 2);
  } else if (code == 0x0073) {
    // popaf
    set_flags(get_mem(get_SP() - 1));
    regs.a = get_mem(get_SP() - 2);
    store_io(0x2, get_SP() - 2);
  } else if (CODE_MASK(0x2800, 0xFF)) {
    // add a, k
    regs.a = add_to(regs.a, code & 0xFF);
  } else if (CODE_MASK(0x0C00, 0x7F)) {
    // add a, m
    regs.a = add_to(regs.a, get_mem(code & 0x7F));
  } else if (CODE_MASK(0x0800, 0x7F)) {
    // add m, a
    int addr = code & 0x7F;
    ram->write(addr, add_to(regs.a, get_mem(addr)));
  } else if (CODE_MASK(0x2900, 0xFF)) {
    // sub a, k
    regs.a = sub_to(regs.a, code & 0xFF);
  } else if (CODE_MASK(0x0C80, 0x7F)) {
    // sub a, m
    regs.a = sub_to(regs.a, get_mem(code & 0x7F));
  } else if (CODE_MASK(0x0880, 0x7F)) {
    // sub m, a
    int addr = code & 0x7F;
    ram->write(addr, sub_to(get_mem(addr), regs.a));
  } else if (CODE_MASK(0x0D00, 0x7F)) {
    // addc a, m
    regs.a = add_to(regs.a, get_mem(code & 0x7F), get_flag(flag_c));
  } else if (CODE_MASK(0x0900, 0x7F)) {
    // addc m, a
    int addr = code & 0x7F;
    ram->write(addr, add_to(regs.a, get_mem(addr), get_flag(flag_c)));
  } else if (code == 0x0060) {
    // addc a
    regs.a = add_to(regs.a, get_flag(flag_c));
  } else if (CODE_MASK(0x1000, 0x7F)) {
    // addc m
    int addr = code & 0x7F;
    ram->write(addr, add_to(get_mem(addr), get_flag(flag_c)));
  } else if (CODE_MASK(0x0D80, 0x7F)) {
    // subc a, m
    regs.a = sub_to(regs.a, get_mem(code & 0x7F), get_flag(flag_c));
  } else if (CODE_MASK(0x0980, 0x7F)) {
    // subc m, a
    int addr = code & 0x7F;
    ram->write(addr, sub_to(get_mem(addr), regs.a, get_flag(flag_c)));
  } else if (code == 0x0061) {
    // subc a
    regs.a = sub_to(regs.a, get_flag(flag_c));
  } else if (CODE_MASK(0x1080, 0x7F)) {
    // subc m
    int addr = code & 0x7F;
    ram->write(addr, sub_to(get_mem(addr), get_flag(flag_c)));
  } else if (CODE_MASK(0x1200, 0x7F)) {
    // inc m
    int addr = code & 0x7F;
    ram->write(addr, add_to(get_mem(addr), 1));
  } else if (CODE_MASK(0x1280, 0x7F)) {
    // dec m
    int addr = code & 0x7F;
    ram->write(addr, sub_to(get_mem(addr), 1));
  } else if (CODE_MASK(0x1300, 0x7F)) {
    // clear m
    ram->write(code & 0x7F, 0);
  } else if (code == 0x006A) {
    // sr a
    store_flag(flag_c, regs.a & 1);
    regs.a >>= 1;
  } else if (CODE_MASK(0x1500, 0x7F)) {
    // sr m
    int value = get_mem(code & 0x7F);
    store_flag(flag_c, value & 1);
    ram->write(code & 0x7F, value >> 1);
  } else if (code == 0x006B) {
    // sl a
    store_flag(flag_c, (regs.a & 0x80) >> 7);
    regs.a <<= 1;
  } else if (CODE_MASK(0x1580, 0x7F)) {
    // sl m
    int value = get_mem(code & 0x7F);
    store_flag(flag_c, (value & 0x80) >> 7);
    ram->write(code & 0x7F, value << 1);
  } else if (code == 0x006C) {
    // src a
    int c = regs.a & 1;
    regs.a >>= 1;
    regs.a |= get_flag(flag_c) << 7;
    store_flag(flag_c, c);
  } else if (CODE_MASK(0x1600, 0x7F)) {
    // src m
    int value = get_mem(code & 0x7F);
    int c = value & 1;
    ram->write(code & 0x7F, (value >> 1) | (get_flag(flag_c) << 7));
    store_flag(flag_c, c);
  } else if (code == 0x006D) {
    // slc a
    int c = (regs.a & 0x80) >> 7;
    regs.a <<= 1;
    regs.a |= get_flag(flag_c);
    store_flag(flag_c, c);
  } else if (CODE_MASK(0x1680, 0x7F)) {
    // slc m
    int value = get_mem(code & 0x7F);
    int c = (value & 0x80) >> 7;
    ram->write(code & 0x7F, (value << 1) | get_flag(flag_c));
    store_flag(flag_c, c);
  } else if (CODE_MASK(0x2C00, 0xFF)) {
    // and a, k
    regs.a &= code & 0xFF;
    store_flag(flag_z, !regs.a);
  } else if (CODE_MASK(0x0E00, 0x7F)) {
    // and a, m
    regs.a &= get_mem(code & 0x7F);
    store_flag(flag_z, !regs.a);
  } else if (CODE_MASK(0x0A00, 0x7F)) {
    // and m, a
    int store = regs.a & get_mem(code & 0x7F);
    store_flag(flag_z, !store);
    ram->write(code & 0x7F, store);
  } else if (CODE_MASK(0x2D00, 0xFF)) {
    // or a, k
    regs.a |= code & 0xFF;
    store_flag(flag_z, !regs.a);
  } else if (CODE_MASK(0x0E80, 0x7F)) {
    // or a, m
    regs.a |= get_mem(code & 0x7F);
    store_flag(flag_z, !regs.a);
  } else if (CODE_MASK(0x0A80, 0x7F)) {
    // or m, a
    int store = regs.a | get_mem(code & 0x7F);
    store_flag(flag_z, !store);
    ram->write(code & 0x7F, store);
  } else if (CODE_MASK(0x2E00, 0xFF)) {
    // xor a, k
    regs.a ^= code & 0xFF;
    store_flag(flag_z, !regs.a);
  } else if (CODE_MASK(0x0F00, 0x7F)) {
    // xor a, m
    regs.a ^= get_mem(code & 0x7F);
    store_flag(flag_z, !regs.a);
  } else if (CODE_MASK(0x0B00, 0x7F)) {
    // xor m, a
    int store = regs.a ^ get_mem(code & 0x7F);
    store_flag(flag_z, !store);
    ram->write(code & 0x7F, store);
  } else if (CODE_MASK(0x00C0, 0x3F)) {
    // xor io, a
    store_io(code & 0x3F, regs.a ^ get_io(code & 0x3F));
  } else if (code == 0x0068) {
    // not a
    regs.a = ~regs.a;
  } else if (CODE_MASK(0x1400, 0x7F)) {
    // not m
    ram->write(code & 0x7F, ~get_mem(code & 0x7F));
  } else if (code == 0x0069) {
    // neg a
    regs.a = -regs.a;
  } else if (CODE_MASK(0x1480, 0x7F)) {
    // neg m
    ram->write(code & 0x7F, -get_mem(code & 0x7F));
  } else if (CODE_MASK(0x1C00, 0x1FF)) {
    // set0 io, k
    const u8_t bit = (code & 0x1C0) >> 6;
    const u8_t addr = code & 0x3F;
    store_io(addr, get_io(addr) & ~(1 << bit));
  } else if (CODE_MASK(0x2400, 0x1FF)) {
    // set0 m, k
    const u8_t bit = (code & 0x1C0) >> 6;
    const u8_t addr = code & 0x3F;
    ram->write(addr, get_mem(addr) & ~(1 << bit));
  } else if (CODE_MASK(0x1E00, 0x1FF)) {
    // set1 io, k
    const u8_t bit = (code & 0x1C0) >> 6;
    const u8_t addr = code & 0x3F;
    store_io(addr, get_io(addr) | (1 << bit));
  } else if (CODE_MASK(0x2600, 0x1FF)) {
    // set1 m, k
    const u8_t bit = (code & 0x1C0) >> 6;
    const u8_t addr = code & 0x3F;
    ram->write(addr, get_mem(addr) | (1 << bit));
  } else if (CODE_MASK(0x1800, 0x1FF)) {
    // t0sn io, k
    int n = (code & 0x1C0) >> 6;
    if (!(get_io(code & 0x3F) & (1 << n)))
      ++PC;
  } else if (CODE_MASK(0x2000, 0x1FF)) {
    // t0sn m, k
    int n = (code & 0x1C0) >> 6;
    if (!(get_mem(code & 0x3F) & (1 << n)))
      ++PC;
  } else if (CODE_MASK(0x1A00, 0x1FF)) {
    // t1sn io, k
    int n = (code & 0x1C0) >> 6;
    if (get_io(code & 0x3F) & (1 << n))
      ++PC;
  } else if (CODE_MASK(0x2200, 0x1FF)) {
    // t1sn m, k
    int n = (code & 0x1C0) >> 6;
    if (get_mem(code & 0x3F) & (1 << n))
      ++PC;
  } else if (CODE_MASK(0x2A00, 0xFF)) {
    // ceqsn a, k
    sub_to(regs.a, code & 0xFF);
    if (regs.a == (code & 0xFF))
      ++PC;
  } else if (CODE_MASK(0x1700, 0x7F)) {
    // ceqsn a, m
    int addr = code & 0x7F;
    sub_to(regs.a, get_mem(addr));
    if (regs.a == get_mem(addr))
      ++PC;
  } else if (CODE_MASK(0x2B00, 0xFF)) {
    // cneqsn a, k
    sub_to(regs.a, code & 0xFF);
    if (regs.a != (code & 0xFF))
      ++PC;
  } else if (CODE_MASK(0x1780, 0x7F)) {
    // cneqsn a, m
    int addr = code & 0x7F;
    sub_to(regs.a, get_mem(addr));
    if (regs.a != get_mem(addr))
      ++PC;
  } else if (code == 0x0062) {
    // izsn
    regs.a = add_to(regs.a, 1);
    if (!regs.a)
      ++PC;
  } else if (CODE_MASK(0x1100, 0x7F)) {
    // izsn m
    const int addr = code & 0x7F;
    int result = add_to(get_mem(addr), 1);
    ram->write(addr, result);
    if (!result)
      ++PC;
  } else if (code == 0x0063) {
    // dzsn
    regs.a = sub_to(regs.a, 1);
    if (!regs.a)
      ++PC;
  } else if (CODE_MASK(0x1180, 0x7F)) {
    // dzsn m
    const int addr = code & 0x7F;
    int result = sub_to(get_mem(addr), 1);
    ram->write(addr, result);
    if (!result)
      ++PC;
  } else if (CODE_MASK(0x3800, 0x7FF)) {
    // call k
    ram->write(get_SP(), PC);
    ram->write(get_SP() + 1, PC >> 8);
    PC = code & 0x7FF;
    store_io(0x2, get_SP() + 2);
  } else if (CODE_MASK(0x3000, 0x7FF)) {
    // goto k
    PC = code & 0x7FF;
  } else if (CODE_MASK(0x0600, 0x7F)) {
    // comp a, m
    sub_to(regs.a, get_mem(code & 0x7F));
  } else if (CODE_MASK(0x0680, 0x7F)) {
    // comp m, a
    sub_to(get_mem(code & 0x7F), regs.a);
  } else if (CODE_MASK(0x0700, 0x7F)) {
    // nadd a, m
    regs.a = add_to(get_mem(code & 0x7F), -regs.a);
  } else if (CODE_MASK(0x0780, 0x7F)) {
    // nadd m, a
    int addr = code & 0x7F;
    ram->write(addr, add_to(-get_mem(addr), regs.a));
  } else if (code == 0x006E) {
    // swap
    int high = regs.a & 0xF;
    regs.a = (high << 4) | (regs.a >> 4);
  } else if (code == 0x0067) {
    // pcadd
    PC += regs.a - 1;
  }
  // TODO: engint
  // TODO: disint
  else if (code == 0x0076) {
    // stopsys
    return (resHALT);
  }
  // TODO: stopexe
  // TODO: reset
  // TODO: wdreset
  // TODO: swapc IO, k
  else if (code == 0x0006) {
    // ldsptl
    regs.a = rom->get(get_SP()) & 0xFF;
  } else if (code == 0x0007) {
    // ldspth
    regs.a = (rom->get(get_SP()) & 0xFF00) >> 8;
  } else if (code == 0x007C) {
    // mul
    unsigned result = regs.a * get_io(0x08);
    regs.a = result & 0xFF;
    store_io(0x08, (result & 0xFF00) >> 8);
  } else if (code == 0xFF00) {
    // putchar - usim specific instruction
    putchar(regs.a);
    fflush(stdout);
  } else {
    return (resINV_INST);
  }
  return (resGO);
}

int cl_pdk::execute_pdk13(unsigned int code) {
  if (code == 0x0000) {
    // nop
  } else if (CODE_MASK(0x0100, 0xFF)) {
    // ret k
    regs.a = code & 0xFF;
    store_io(0x2, get_SP() - 2);
    PC = get_mem(get_SP()) | (get_mem(get_SP() + 1) << 8);
  } else if (code == 0x003A) {
    // ret
    store_io(0x2, get_SP() - 2);
    PC = get_mem(get_SP()) | (get_mem(get_SP() + 1) << 8);
  } else if (CODE_MASK(0x1700, 0xFF)) {
    // mov a, k
    regs.a = code & 0xFF;
  } else if (CODE_MASK(0x0080, 0x1F)) {
    // mov i, a
    store_io(code & 0x1F, regs.a);
  } else if (CODE_MASK(0x00A0, 0x1F)) {
    // mov a, i
    regs.a = get_io(code & 0x1F);
  } else if (CODE_MASK(0x05C0, 0x3F)) {
    // mov m, a
    ram->write(code & 0x3F, regs.a);
  } else if (CODE_MASK(0x07C0, 0x3F)) {
    // mov a, m
    regs.a = get_mem(code & 0x3F);
  } else if (CODE_MASK(0x00C1, 0x1E)) {
    // TODO: ldt16
  } else if (CODE_MASK(0x0C00, 0x1E)) {
    // TODO: stt16
  } else if ((CODE_MASK(0x0E1, 0x1E))) {
    // idxm a, m
    regs.a = get_mem(get_mem(code & 0x1E));
  } else if ((CODE_MASK(0x0E0, 0x1E))) {
    // idxm m, a
    ram->write(get_mem(code & 0x1E), regs.a);
  } else if (CODE_MASK(0x09C0, 0x3F)) {
    // xch m
    int mem = get_mem(code & 0x3F);
    ram->write(code & 0x3F, regs.a);
    regs.a = mem;
  } else if (code == 0x0032) {
    // pushaf
    ram->write(get_SP(), regs.a);
    ram->write(get_SP() + 1, get_flags());
    store_io(0x2, get_SP() + 2);
  } else if (code == 0x0033) {
    // popaf
    set_flags(get_mem(get_SP() - 1));
    regs.a = get_mem(get_SP() - 2);
    store_io(0x2, get_SP() - 2);
  } else if (CODE_MASK(0x1000, 0xFF)) {
    // add a, k
    regs.a = add_to(regs.a, code & 0xFF);
  } else if (CODE_MASK(0x0600, 0x3F)) {
    // add a, m
    regs.a = add_to(regs.a, get_mem(code & 0x3F));
  } else if (CODE_MASK(0x0400, 0x3F)) {
    // add m, a
    int addr = code & 0x3F;
    ram->write(addr, add_to(regs.a, get_mem(addr)));
  } else if (CODE_MASK(0x1100, 0xFF)) {
    // sub a, k
    regs.a = sub_to(regs.a, code & 0xFF);
  } else if (CODE_MASK(0x0640, 0x3F)) {
    // sub a, m
    regs.a = sub_to(regs.a, get_mem(code & 0x3F));
  } else if (CODE_MASK(0x0440, 0x3F)) {
    // sub m, a
    int addr = code & 0x3F;
    ram->write(addr, sub_to(get_mem(addr), regs.a));
  } else if (CODE_MASK(0x0680, 0x3F)) {
    // addc a, m
    regs.a = add_to(regs.a, get_mem(code & 0x3F), get_flag(flag_c));
  } else if (CODE_MASK(0x0480, 0x3F)) {
    // addc m, a
    int addr = code & 0x3F;
    ram->write(addr, add_to(regs.a, get_mem(addr), get_flag(flag_c)));
  } else if (code == 0x0010) {
    // addc a
    regs.a = add_to(regs.a, get_flag(flag_c));
  } else if (CODE_MASK(0x0800, 0x3F)) {
    // addc m
    int addr = code & 0x3F;
    ram->write(addr, add_to(get_mem(addr), get_flag(flag_c)));
  } else if (CODE_MASK(0x06C0, 0x3F)) {
    // subc a, m
    regs.a = sub_to(regs.a, get_mem(code & 0x3F), get_flag(flag_c));
  } else if (CODE_MASK(0x04C0, 0x3F)) {
    // subc m, a
    int addr = code & 0x3F;
    ram->write(addr, sub_to(get_mem(addr), regs.a, get_flag(flag_c)));
  } else if (code == 0x0011) {
    // subc a
    regs.a = sub_to(regs.a, get_flag(flag_c));
  } else if (CODE_MASK(0x0840, 0x3F)) {
    // subc m
    int addr = code & 0x3F;
    ram->write(addr, sub_to(get_mem(addr), get_flag(flag_c)));
  } else if (CODE_MASK(0x0900, 0x3F)) {
    // inc m
    int addr = code & 0x3F;
    ram->write(addr, add_to(get_mem(addr), 1));
  } else if (CODE_MASK(0x0940, 0x3F)) {
    // dec m
    int addr = code & 0x3F;
    ram->write(addr, sub_to(get_mem(addr), 1));
  } else if (CODE_MASK(0x0980, 0x3F)) {
    // clear m
    ram->write(code & 0x3F, 0);
  } else if (code == 0x001A) {
    // sr a
    store_flag(flag_c, regs.a & 1);
    regs.a >>= 1;
  } else if (CODE_MASK(0x0A80, 0x3F)) {
    // sr m
    int value = get_mem(code & 0x3F);
    store_flag(flag_c, value & 1);
    ram->write(code & 0x3F, value >> 1);
  } else if (code == 0x001B) {
    // sl a
    store_flag(flag_c, (regs.a & 0x80) >> 7);
    regs.a <<= 1;
  } else if (CODE_MASK(0x0AC0, 0x3F)) {
    // sl m
    int value = get_mem(code & 0x3F);
    store_flag(flag_c, (value & 0x80) >> 7);
    ram->write(code & 0x3F, value << 1);
  } else if (code == 0x001C) {
    // src a
    int c = regs.a & 1;
    regs.a >>= 1;
    regs.a |= get_flag(flag_c) << 7;
    store_flag(flag_c, c);
  } else if (CODE_MASK(0x0B00, 0x3F)) {
    // src m
    int value = get_mem(code & 0x3F);
    int c = value & 1;
    ram->write(code & 0x3F, (value >> 1) | (get_flag(flag_c) << 7));
    store_flag(flag_c, c);
  } else if (code == 0x001D) {
    // slc a
    int c = (regs.a & 0x80) >> 7;
    regs.a <<= 1;
    regs.a |= get_flag(flag_c);
    store_flag(flag_c, c);
  } else if (CODE_MASK(0x0B40, 0x3F)) {
    // slc m
    int value = get_mem(code & 0x3F);
    int c = (value & 0x80) >> 7;
    ram->write(code & 0x3F, (value << 1) | get_flag(flag_c));
    store_flag(flag_c, c);
  } else if (CODE_MASK(0x1400, 0xFF)) {
    // and a, k
    regs.a &= code & 0xFF;
    store_flag(flag_z, !regs.a);
  } else if (CODE_MASK(0x0700, 0x3F)) {
    // and a, m
    regs.a &= get_mem(code & 0x3F);
    store_flag(flag_z, !regs.a);
  } else if (CODE_MASK(0x0500, 0x3F)) {
    // and m, a
    int store = regs.a & get_mem(code & 0x3F);
    store_flag(flag_z, !store);
    ram->write(code & 0x3F, store);
  } else if (CODE_MASK(0x1500, 0xFF)) {
    // or a, k
    regs.a |= code & 0xFF;
    store_flag(flag_z, !regs.a);
  } else if (CODE_MASK(0x0740, 0x3F)) {
    // or a, m
    regs.a |= get_mem(code & 0x3F);
    store_flag(flag_z, !regs.a);
  } else if (CODE_MASK(0x0540, 0x3F)) {
    // or m, a
    int store = regs.a | get_mem(code & 0x3F);
    store_flag(flag_z, !store);
    ram->write(code & 0x3F, store);
  } else if (CODE_MASK(0x1600, 0xFF)) {
    // xor a, k
    regs.a ^= code & 0xFF;
    store_flag(flag_z, !regs.a);
  } else if (CODE_MASK(0x0780, 0x3F)) {
    // xor a, m
    regs.a ^= get_mem(code & 0x3F);
    store_flag(flag_z, !regs.a);
  } else if (CODE_MASK(0x0580, 0x3F)) {
    // xor m, a
    int store = regs.a ^ get_mem(code & 0x3F);
    store_flag(flag_z, !store);
    ram->write(code & 0x3F, store);
  } else if (CODE_MASK(0x0060, 0x1F)) {
    // xor io, a
    store_io(code & 0x1F, regs.a ^ get_io(code & 0x1F));
  } else if (code == 0x0018) {
    // not a
    regs.a = ~regs.a;
  } else if (CODE_MASK(0x0A00, 0x3F)) {
    // not m
    ram->write(code & 0x3F, ~get_mem(code & 0x3F));
  } else if (code == 0x0019) {
    // neg a
    regs.a = -regs.a;
  } else if (CODE_MASK(0x0A40, 0x3F)) {
    // neg m
    ram->write(code & 0x3F, -get_mem(code & 0x3F));
  } else if (CODE_MASK(0x0E00, 0xFF)) {
    // set0 io, k
    const u8_t bit = (code & 0xE0) >> 5;
    const u8_t addr = code & 0x1F;
    store_io(addr, get_io(addr) & ~(1 << bit));
  } else if (CODE_MASK(0x0300, 0xFE)) {
    // set0 m, k
    const u8_t bit = (code & 0xE0) >> 5;
    const u8_t addr = (code & 0xFE) >> 1;
    ram->write(addr, get_mem(addr) & ~(1 << bit));
  } else if (CODE_MASK(0x0F00, 0xFF)) {
    // set1 io, k
    const u8_t bit = (code & 0xE0) >> 5;
    const u8_t addr = code & 0x1F;
    store_io(addr, get_io(addr) | (1 << bit));
  } else if (CODE_MASK(0x0301, 0xFE)) {
    // set1 m, k
    const u8_t bit = (code & 0xE0) >> 5;
    const u8_t addr = (code & 0x1E) >> 1;
    ram->write(addr, get_mem(addr) | (1 << bit));
  } else if (CODE_MASK(0x0C00, 0xFF)) {
    // t0sn io, k
    int n = (code & 0xE0) >> 5;
    if (!(get_io(code & 0x1F) & (1 << n)))
      ++PC;
  } else if (CODE_MASK(0x0200, 0xFE)) {
    // t0sn m, k
    int n = (code & 0xE0) >> 5;
    if (!(get_mem((code & 0xFE) >> 1) & (1 << n)))
      ++PC;
  } else if (CODE_MASK(0x0D00, 0xFF)) {
    // t1sn io, k
    int n = (code & 0xE0) >> 5;
    if (get_io(code & 0x1F) & (1 << n))
      ++PC;
  } else if (CODE_MASK(0x0201, 0xFE)) {
    // t1sn m, k
    int n = (code & 0xE0) >> 5;
    if (get_mem((code & 0xFE) >> 1) & (1 << n))
      ++PC;
  } else if (CODE_MASK(0x1200, 0xFF)) {
    // ceqsn a, k
    sub_to(regs.a, code & 0xFF);
    if (regs.a == (code & 0xFF))
      ++PC;
  } else if (CODE_MASK(0x0B80, 0x3F)) {
    // ceqsn a, m
    int addr = code & 0x3F;
    sub_to(regs.a, get_mem(addr));
    if (regs.a == get_mem(addr))
      ++PC;
  } else if (code == 0x0012) {
    // izsn
    regs.a = add_to(regs.a, 1);
    if (!regs.a)
      ++PC;
  } else if (CODE_MASK(0x0880, 0x3F)) {
    // izsn m
    const int addr = code & 0x3F;
    int result = add_to(get_mem(addr), 1);
    ram->write(addr, result);
    if (!result)
      ++PC;
  } else if (code == 0x0013) {
    // dzsn
    regs.a = sub_to(regs.a, 1);
    if (!regs.a)
      ++PC;
  } else if (CODE_MASK(0x08C0, 0x3F)) {
    // dzsn m
    const int addr = code & 0x3F;
    int result = sub_to(get_mem(addr), 1);
    ram->write(addr, result);
    if (!result)
      ++PC;
  } else if (CODE_MASK(0x1C00, 0x3FF)) {
    // call k
    ram->write(get_SP(), PC);
    ram->write(get_SP() + 1, PC >> 8);
    PC = code & 0x3FF;
    store_io(0x2, get_SP() + 2);
  } else if (CODE_MASK(0x1800, 0x3FF)) {
    // goto k
    PC = code & 0x3FF;
  } else if (code == 0x001E) {
    // swap
    int high = regs.a & 0xF;
    regs.a = (high << 4) | (regs.a >> 4);
  } else if (code == 0x0017) {
    // pcadd
    PC += regs.a - 1;
  }
  // TODO: engint
  // TODO: disint
  else if (code == 0x0036) {
    // stopsys
    return (resHALT);
  }
  // TODO: stopexe
  // TODO: reset
  // TODO: wdreset
  // TODO: swapc IO, k
  else if (code == 0x0006) {
    // ldsptl
    regs.a = rom->get(get_SP()) & 0xFF;
  } else if (code == 0x0007) {
    // ldregs[0x02]th
    regs.a = (rom->get(get_SP()) & 0xFF00) >> 8;
  } else if (code == 0x003C) {
    // mul
    unsigned result = regs.a * get_io(0x08);
    regs.a = result & 0xFF;
    store_io(0x08, (result & 0xFF00) >> 8);
  } else if (code == 0xFF00) {
    // putchar - usim specific instruction
    putchar(regs.a);
    fflush(stdout);
  } else {
    return (resINV_INST);
  }
  return (resGO);
}

int cl_pdk::execute_pdk15(unsigned int code) {
  if (code == 0x0000) {
    // nop
  } else if (CODE_MASK(0x0200, 0xFF)) {
    // ret k
    regs.a = code & 0xFF;
    store_io(0x2, get_SP() - 2);
    PC = get_mem(get_SP()) | (get_mem(get_SP() + 1) << 8);
  } else if (code == 0x007A) {
    // ret
    store_io(0x2, get_SP() - 2);
    PC = get_mem(get_SP()) | (get_mem(get_SP() + 1) << 8);
  } else if (CODE_MASK(0x5700, 0xFF)) {
    // mov a, k
    regs.a = code & 0xFF;
  } else if (CODE_MASK(0x0100, 0x7F)) {
    // mov i, a
    store_io(code & 0x7F, regs.a);
  } else if (CODE_MASK(0x0180, 0x7F)) {
    // mov a, i
    regs.a = get_io(code & 0x7F);
  } else if (CODE_MASK(0x1700, 0xFF)) {
    // mov m, a
    ram->write(code & 0xFF, regs.a);
  } else if (CODE_MASK(0x1F00, 0xFF)) {
    // mov a, m
    regs.a = get_mem(code & 0xFF);
  } else if (CODE_MASK(0x0601, 0xFE)) {
    // TODO: ldt16
  } else if (CODE_MASK(0x0600, 0xFE)) {
    // TODO: stt16
  } else if ((CODE_MASK(0x701, 0xFE))) {
    // idxm a, m
    regs.a = get_mem(get_mem(code & 0xFE));
  } else if ((CODE_MASK(0x700, 0xFE))) {
    // idxm m, a
    ram->write(get_mem(code & 0xFE), regs.a);
  } else if (CODE_MASK(0x2700, 0xFF)) {
    // xch m
    int mem = get_mem(code & 0xFF);
    ram->write(code & 0xFF, regs.a);
    regs.a = mem;
  } else if (code == 0x0072) {
    // pushaf
    ram->write(get_SP(), regs.a);
    ram->write(get_SP() + 1, get_flags());
    store_io(0x2, get_SP() + 2);
  } else if (code == 0x0073) {
    // popaf
    set_flags(get_mem(get_SP() - 1));
    regs.a = get_mem(get_SP() - 2);
    store_io(0x2, get_SP() - 2);
  } else if (CODE_MASK(0x5000, 0xFF)) {
    // add a, k
    regs.a = add_to(regs.a, code & 0xFF);
  } else if (CODE_MASK(0x1800, 0xFF)) {
    // add a, m
    regs.a = add_to(regs.a, get_mem(code & 0xFF));
  } else if (CODE_MASK(0x1000, 0xFF)) {
    // add m, a
    int addr = code & 0xFF;
    ram->write(addr, add_to(regs.a, get_mem(addr)));
  } else if (CODE_MASK(0x5100, 0xFF)) {
    // sub a, k
    regs.a = sub_to(regs.a, code & 0xFF);
  } else if (CODE_MASK(0x1900, 0xFF)) {
    // sub a, m
    regs.a = sub_to(regs.a, get_mem(code & 0xFF));
  } else if (CODE_MASK(0x1100, 0xFF)) {
    // sub m, a
    int addr = code & 0xFF;
    ram->write(addr, sub_to(get_mem(addr), regs.a));
  } else if (CODE_MASK(0x1A00, 0xFF)) {
    // addc a, m
    regs.a = add_to(regs.a, get_mem(code & 0xFF), get_flag(flag_c));
  } else if (CODE_MASK(0x1200, 0xFF)) {
    // addc m, a
    int addr = code & 0xFF;
    ram->write(addr, add_to(regs.a, get_mem(addr), get_flag(flag_c)));
  } else if (code == 0x0060) {
    // addc a
    regs.a = add_to(regs.a, get_flag(flag_c));
  } else if (CODE_MASK(0x2000, 0xFF)) {
    // addc m
    int addr = code & 0xFF;
    ram->write(addr, add_to(get_mem(addr), get_flag(flag_c)));
  } else if (CODE_MASK(0x1B00, 0xFF)) {
    // subc a, m
    regs.a = sub_to(regs.a, get_mem(code & 0xFF), get_flag(flag_c));
  } else if (CODE_MASK(0x1300, 0xFF)) {
    // subc m, a
    int addr = code & 0xFF;
    ram->write(addr, sub_to(get_mem(addr), regs.a, get_flag(flag_c)));
  } else if (code == 0x0061) {
    // subc a
    regs.a = sub_to(regs.a, get_flag(flag_c));
  } else if (CODE_MASK(0x2100, 0xFF)) {
    // subc m
    int addr = code & 0xFF;
    ram->write(addr, sub_to(get_mem(addr), get_flag(flag_c)));
  } else if (CODE_MASK(0x2400, 0xFF)) {
    // inc m
    int addr = code & 0xFF;
    ram->write(addr, add_to(get_mem(addr), 1));
  } else if (CODE_MASK(0x2500, 0xFF)) {
    // dec m
    int addr = code & 0xFF;
    ram->write(addr, sub_to(get_mem(addr), 1));
  } else if (CODE_MASK(0x2600, 0xFF)) {
    // clear m
    ram->write(code & 0xFF, 0);
  } else if (code == 0x006A) {
    // sr a
    store_flag(flag_c, regs.a & 1);
    regs.a >>= 1;
  } else if (CODE_MASK(0x2A00, 0xFF)) {
    // sr m
    int value = get_mem(code & 0xFF);
    store_flag(flag_c, value & 1);
    ram->write(code & 0xFF, value >> 1);
  } else if (code == 0x006B) {
    // sl a
    store_flag(flag_c, (regs.a & 0x80) >> 7);
    regs.a <<= 1;
  } else if (CODE_MASK(0x2B00, 0xFF)) {
    // sl m
    int value = get_mem(code & 0xFF);
    store_flag(flag_c, (value & 0x80) >> 7);
    ram->write(code & 0xFF, value << 1);
  } else if (code == 0x006C) {
    // src a
    int c = regs.a & 1;
    regs.a >>= 1;
    regs.a |= get_flag(flag_c) << 7;
    store_flag(flag_c, c);
  } else if (CODE_MASK(0x2C00, 0xFF)) {
    // src m
    int value = get_mem(code & 0xFF);
    int c = value & 1;
    ram->write(code & 0xFF, (value >> 1) | (get_flag(flag_c) << 7));
    store_flag(flag_c, c);
  } else if (code == 0x006D) {
    // slc a
    int c = (regs.a & 0x80) >> 7;
    regs.a <<= 1;
    regs.a |= get_flag(flag_c);
    store_flag(flag_c, c);
  } else if (CODE_MASK(0x2D00, 0xFF)) {
    // slc m
    int value = get_mem(code & 0xFF);
    int c = (value & 0x80) >> 7;
    ram->write(code & 0xFF, (value << 1) | get_flag(flag_c));
    store_flag(flag_c, c);
  } else if (CODE_MASK(0x5400, 0xFF)) {
    // and a, k
    regs.a &= code & 0xFF;
    store_flag(flag_z, !regs.a);
  } else if (CODE_MASK(0x1C00, 0xFF)) {
    // and a, m
    regs.a &= get_mem(code & 0xFF);
    store_flag(flag_z, !regs.a);
  } else if (CODE_MASK(0x1400, 0xFF)) {
    // and m, a
    int store = regs.a & get_mem(code & 0xFF);
    store_flag(flag_z, !store);
    ram->write(code & 0xFF, store);
  } else if (CODE_MASK(0x5500, 0xFF)) {
    // or a, k
    regs.a |= code & 0xFF;
    store_flag(flag_z, !regs.a);
  } else if (CODE_MASK(0x1D00, 0xFF)) {
    // or a, m
    regs.a |= get_mem(code & 0xFF);
    store_flag(flag_z, !regs.a);
  } else if (CODE_MASK(0x1500, 0xFF)) {
    // or m, a
    int store = regs.a | get_mem(code & 0xFF);
    store_flag(flag_z, !store);
    ram->write(code & 0xFF, store);
  } else if (CODE_MASK(0x5600, 0xFF)) {
    // xor a, k
    regs.a ^= code & 0xFF;
    store_flag(flag_z, !regs.a);
  } else if (CODE_MASK(0x1E00, 0xFF)) {
    // xor a, m
    regs.a ^= get_mem(code & 0xFF);
    store_flag(flag_z, !regs.a);
  } else if (CODE_MASK(0x1600, 0xFF)) {
    // xor m, a
    int store = regs.a ^ get_mem(code & 0xFF);
    store_flag(flag_z, !store);
    ram->write(code & 0xFF, store);
  } else if (CODE_MASK(0x0080, 0x7F)) {
    // xor io, a
    store_io(code & 0x3F, regs.a ^ get_io(code & 0x3F));
  } else if (code == 0x0068) {
    // not a
    regs.a = ~regs.a;
  } else if (CODE_MASK(0x2800, 0xFF)) {
    // not m
    ram->write(code & 0xFF, ~get_mem(code & 0xFF));
  } else if (code == 0x0069) {
    // neg a
    regs.a = -regs.a;
  } else if (CODE_MASK(0x2900, 0xFF)) {
    // neg m
    ram->write(code & 0xFF, -get_mem(code & 0xFF));
  } else if (CODE_MASK(0x3800, 0x3FF)) {
    // set0 io, k
    const u8_t bit = (code & 0x380) >> 7;
    const u8_t addr = code & 0x7F;
    store_io(addr, get_io(addr) & ~(1 << bit));
  } else if (CODE_MASK(0x4800, 0x3FF)) {
    // set0 m, k
    const u8_t bit = (code & 0x380) >> 7;
    const u8_t addr = code & 0x7F;
    ram->write(addr, get_mem(addr) & ~(1 << bit));
  } else if (CODE_MASK(0x3C00, 0x3FF)) {
    // set1 io, k
    const u8_t bit = (code & 0x380) >> 7;
    const u8_t addr = code & 0x7F;
    store_io(addr, get_io(addr) | (1 << bit));
  } else if (CODE_MASK(0x4C00, 0x3FF)) {
    // set1 m, k
    const u8_t bit = (code & 0x380) >> 7;
    const u8_t addr = code & 0x7F;
    ram->write(addr, get_mem(addr) | (1 << bit));
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
    sub_to(regs.a, code & 0xFF);
    if (regs.a == (code & 0xFF))
      ++PC;
  } else if (CODE_MASK(0x2E00, 0xFF)) {
    // ceqsn a, m
    int addr = code & 0xFF;
    sub_to(regs.a, get_mem(addr));
    if (regs.a == get_mem(addr))
      ++PC;
  } else if (CODE_MASK(0x5300, 0xFF)) {
    // cneqsn a, k
    sub_to(regs.a, code & 0xFF);
    if (regs.a != (code & 0xFF))
      ++PC;
  } else if (CODE_MASK(0x2F00, 0xFF)) {
    // cneqsn a, m
    int addr = code & 0xFF;
    sub_to(regs.a, get_mem(addr));
    if (regs.a != get_mem(addr))
      ++PC;
  } else if (code == 0x0062) {
    // izsn
    regs.a = add_to(regs.a, 1);
    if (!regs.a)
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
    regs.a = sub_to(regs.a, 1);
    if (!regs.a)
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
    ram->write(get_SP(), PC);
    ram->write(get_SP() + 1, PC >> 8);
    PC = code & 0xFFF;
    store_io(0x2, get_SP() + 2);
  } else if (CODE_MASK(0x6000, 0xFFF)) {
    // goto k
    PC = code & 0xFFF;
  } else if (CODE_MASK(0x0C00, 0xFF)) {
    // comp a, m
    sub_to(regs.a, get_mem(code & 0xFF));
  } else if (CODE_MASK(0x0D00, 0xFF)) {
    // comp m, a
    sub_to(get_mem(code & 0xFF), regs.a);
  } else if (CODE_MASK(0x0E00, 0xFF)) {
    // nadd a, m
    regs.a = add_to(get_mem(code & 0xFF), -regs.a);
  } else if (CODE_MASK(0x0F00, 0xFF)) {
    // nadd m, a
    int addr = code & 0xFF;
    ram->write(addr, add_to(-get_mem(addr), regs.a));
  } else if (code == 0x006E) {
    // swap
    int high = regs.a & 0xF;
    regs.a = (high << 4) | (regs.a >> 4);
  } else if (code == 0x0067) {
    // pcadd
    PC += regs.a - 1;
  }
  // TODO: engint
  // TODO: disint
  else if (code == 0x0076) {
    // stopsys
    return (resHALT);
  }
  // TODO: stopexe
  // TODO: reset
  // TODO: wdreset
  // TODO: swapc IO, k
  else if (code == 0x0006) {
    // ldsptl
    regs.a = rom->get(get_SP()) & 0xFF;
  } else if (code == 0x0007) {
    // ldspth
    regs.a = (rom->get(get_SP()) & 0xFF00) >> 8;
  } else if (code == 0x007C) {
    // mul
    unsigned result = regs.a * get_io(0x08);
    regs.a = result & 0xFF;
    store_io(0x08, (result & 0xFF00) >> 8);
  } else if (code == 0xFF00) {
    // putchar - usim specific instruction
    putchar(regs.a);
    fflush(stdout);
  } else {
    return (resINV_INST);
  }
  return (resGO);
}

/* End of pdk.src/inst.cc */

