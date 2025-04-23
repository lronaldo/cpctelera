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


  cl_memory_cell * get_register(uint code, bool first_operand);
  uint get_constant(uint code);
  uint get_address6(uint code);
  uint get_address10(uint code);
  uint get_address12(uint code);

  virtual int inst_add(uint code, int operands);
  virtual int inst_addcy(uint code, int operands);
  virtual int inst_and(uint code, int operands);
  virtual int inst_call(uint code, int inst, int operands);
  virtual int inst_compare(uint code, int operands);
  virtual int inst_comparecy(uint code, int operands);
  virtual int inst_interrupt(bool value);
  virtual int inst_fetch(uint code, int inst);
  virtual int inst_hwbuild(uint code, int inst);
  virtual int inst_input(uint code, int operands);
  virtual int inst_jump(uint code, int inst, int operands);
  virtual int inst_load(uint code, int operands);
  virtual int inst_load_return(uint code, int operands);
  virtual int inst_or(uint code, int operands);
  virtual int inst_output(uint code, int operands);
  virtual int inst_outputk(uint code, int operands);
  virtual int inst_regbank(int regbank);
  virtual int inst_return(uint code, int inst);
  virtual int inst_returni(bool interrupt_enable);
  virtual int inst_rl(uint code, int operands);
  virtual int inst_rr(uint code, int operands);
  virtual int inst_sl(uint code, int inst);
  virtual int inst_sr(uint code, int inst);
  virtual int inst_star(uint code, int operands);
  virtual int inst_store(uint code, int operands);
  virtual int inst_sub(uint code, int operands);
  virtual int inst_subcy(uint code, int operands);
  virtual int inst_test(uint code, int operands);
  virtual int inst_testcy(uint code, int operands);
  virtual int inst_xor(uint code, int operands);

/* End of pblaze.src/instcl.h */
