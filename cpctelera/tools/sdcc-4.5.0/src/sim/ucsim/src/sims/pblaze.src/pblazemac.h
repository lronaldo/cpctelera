/*
 * Copyright (C) 2012-2013 Jiří Šimek
 * Copyright (C) 2013 Zbyněk Křivka <krivka@fit.vutbr.cz>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


/*
#define store2(addr, val) { ram->set((t_addr) (addr), (val >> 8) & 0xff); \
                            ram->set((t_addr) (addr+1), val & 0xff); }
#define store1(addr, val) ram->set((t_addr) (addr), val)
#define get1(addr) ram->get((t_addr) (addr))
#define get2(addr) ((ram->get((t_addr) (addr)) << 8) | ram->get((t_addr) (addr+1)) )
#define fetch2() ((fetch() << 8) | fetch() )
#define fetch1() fetch()
*
#define push(val) { stack->set(regs.internal_sp, val); regs.internal_sp = (regs.internal_sp + 1) % 31; }
#define pop(var) {var = ram->get(regs.internal_sp); regs.SP+=1;}

/
#define FLAG_SET(f) {regs.P |= f;}
#define FLAG_CLEAR(f) {regs.P &= ~(f);}
#define FLAG_ASSIGN(f,c) {regs.P = (c) ? regs.P | (f) : regs.P & ~(f);}
#define FLAG_NZ(f) { \
      regs.P = (regs.P & ~(BIT_N|BIT_Z)) \
      | (((f) & 0xff) ? 0 : BIT_Z) \
      | (((f) & 0x80) ? BIT_N : 0) \
      ; }
#define EA_IMM(c) ((((c) >> 4) & 0xf)==0xa)
#define OPERAND(code) (EA_IMM(code) ? fetch() : get1(fetchea(code)))
*/

/* End of pblaze.src/pblazemac.h */
