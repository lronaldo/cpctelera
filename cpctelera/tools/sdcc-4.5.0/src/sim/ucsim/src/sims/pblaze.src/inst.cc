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


#include "stdio.h"
#include <ctype.h>
#include <stdlib.h>

#include "globals.h"
#include "ddconfig.h"

// sim
#include "memcl.h"

// local
#include "glob.h"
#include "pblazecl.h"
#include "regspblaze.h"
#include "pblazemac.h"

cl_memory_cell *
cl_pblaze::get_register(uint code, bool first_operand) {
    unsigned char reg_code;

    if (first_operand) {
        reg_code = (code & 0x00f00) >> 8;
    }
    else {
        reg_code = (code & 0x000f0) >> 4;
    }

    if (active_regbank == REGISTER_BANK_B) {
      reg_code += 0x10;
    }

    return sfr->get_cell(reg_code);
}

uint
cl_pblaze::get_constant(uint code) {
    return code & 0x000ff;
}

uint
cl_pblaze::get_address6(uint code) {
    return code & 0x0003f;
}

uint
cl_pblaze::get_address10(uint code) {
    return code & 0x003ff;
}

uint
cl_pblaze::get_address12(uint code) {
    return code & 0x00fff;
}

/* ********** INSTRUCTIONS ********** */

int
cl_pblaze::inst_add(uint code, int operands)
{
  application->debug("ADD\n");

  cl_memory_cell *r1 = get_register(code, true);
  u8_t operand;
  int result;

  if (operands == REG_CONSTANT) {
      operand = get_constant(code);
  }
  else if (operands == REG_REG) {
      operand = get_register(code, false)->get();
  }
  else {
    fprintf(stderr, "ADD: Unsupported operand type\n");
    return (resERROR);
  }

  result = r1->get() + operand;

  // carry flag
  FLAGS_SET_C(result > 255);

  result = result % 256;

  //zero flag
  FLAGS_SET_Z(result == 0);

  r1->set(result);

  return(resGO);
}

int
cl_pblaze::inst_addcy(uint code, int operands)
{
  application->debug("ADDCY\n");

  cl_memory_cell *r1 = get_register(code, true);
  u8_t operand;
  int result;

  if (operands == REG_CONSTANT) {
      operand = get_constant(code);
  }
  else if (operands == REG_REG) {
      operand = get_register(code, false)->get();
  }
  else {
    fprintf(stderr, "ADDCY: Unsupported operand type\n");
    return (resERROR);
  }

  result = r1->get() + operand + (FLAGS_GET_C ? 1 : 0);

  // carry flag
  FLAGS_SET_C(result > 255);

  result = result % 256;

  //zero flag
  if (type->type == CPU_PBLAZE_3) {
    FLAGS_SET_Z(result == 0);
  }
  else if (type->type == CPU_PBLAZE_6) {
    FLAGS_SET_Z(FLAGS_GET_Z && (result == 0));
  }

  r1->set(result);

  return(resGO);
}

int
cl_pblaze::inst_and(uint code, int operands)
{
  application->debug("AND\n");

  cl_memory_cell *r1 = get_register(code, true);
  u8_t operand;
  int result;

  if (operands == REG_CONSTANT) {
      operand = get_constant(code);
  }
  else if (operands == REG_REG) {
      operand = get_register(code, false)->get();
  }
  else {
    fprintf(stderr, "ADDCY: Unsupported operand type\n");
    return (resERROR);
  }

  result = r1->get() & operand;

  // carry flag
  FLAGS_SET_C(0);
  //zero flag
  FLAGS_SET_Z(result == 0);

  r1->set(result);

  return(resGO);
}

int
cl_pblaze::inst_call(uint code, int inst, int operands)
{
  bool jump = false;

  uint address;

  if (operands == ADDRESS10)
    address = get_address10(code);
  else if (operands == ADDRESS12)
    address = get_address12(code);
  else if (operands == REG_REG) {
    cl_memory_cell *r1 = get_register(code, true);
    cl_memory_cell *r2 = get_register(code, false);
    address = ((r1->get() & 0x0f) << 8) | r2->get();
  }
  else {
    application->debug("CALL: Unsupported operand type\n");
    return (resERROR);
  }

  switch (inst) {
    case CALL:
      application->debug("CALL\n");
      jump = true;
      break;

    case CALL_AT:
      application->debug("CALL@\n");
      jump = true;
      break;

    case CALL_C:
      application->debug("CALL C\n");
      jump = FLAGS_GET_C;
      break;

    case CALL_NC:
      application->debug("CALL NC\n");
      jump = !FLAGS_GET_C;
      break;

    case CALL_Z:
      application->debug("CALL Z\n");
      jump = FLAGS_GET_Z;
      break;

    case CALL_NZ:
      application->debug("CALL NZ\n");
      jump = !FLAGS_GET_Z;
      break;

    default:
      fprintf(stderr, "Unsupported CALL instruction\n");
      return (resERROR);
  }

  if (jump) {
    if (stack_push(PC-1)) {
      fprintf(stderr, "Call stack overflow.\n");

      if (type->type == CPU_PBLAZE_6)
        reset();
      //return (resERROR);
    }
    PC = address;
  }

  return(resGO);
}

int
cl_pblaze::inst_compare(uint code, int operands)
{
  application->debug("COMPARE\n");

  u8_t operand1 = get_register(code, true)->get();
  u8_t operand2;

  if (operands == REG_CONSTANT) {
      operand2 = get_constant(code);
  }
  else if (operands == REG_REG) {
      operand2 = get_register(code, false)->get();
  }
  else {
    fprintf(stderr, "COMPARE: Unsupported operand type\n");
    return (resERROR);
  }

  FLAGS_SET_C(operand2 > operand1);
  FLAGS_SET_Z(operand1 == operand2);

  return(resGO);
}

int
cl_pblaze::inst_comparecy(uint code, int operands)
{
  application->debug("COMPARECY\n");

  u8_t operand1 = get_register(code, true)->get();
  u8_t operand2;

  if (operands == REG_CONSTANT) {
      operand2 = get_constant(code);
  }
  else if (operands == REG_REG) {
      operand2 = get_register(code, false)->get();
  }
  else {
    fprintf(stderr, "COMPARECY: Unsupported operand type\n");
    return (resERROR);
  }

  FLAGS_SET_Z((operand1 == (operand2 + FLAGS_GET_C)) && FLAGS_GET_Z);
  FLAGS_SET_C((operand2 + FLAGS_GET_C) > operand1);

  return(resGO);
}

int
cl_pblaze::inst_fetch(uint code, int operands)
{
  application->debug("FETCH\n");

  cl_memory_cell *r1 = get_register(code, true);
  u8_t addr;

  if (operands == REG_ADDRESS6) {
    addr = get_constant(code) & 0x3f; // only bits 5..0 are used for addressing
  }
  else if (operands == REG_ADDRESS8) {
    addr = get_constant(code);
  }
  else if (operands == REG_REG) {
    addr = get_register(code, false)->get();
  }
  else {
    fprintf(stderr, "FETCH: Unsupported operand type\n");
    return (resERROR);
  }

  r1->set(ram->read(addr));

  return (resGO);
}

int
cl_pblaze::inst_hwbuild(uint code, int operands)
{
  application->debug("HWBUILD\n");


  if (operands == REG) {
    cl_memory_cell *r1 = get_register(code, true);
    r1->set(hw_constant);
  }
  else {
    fprintf(stderr, "HWBUILD: Unsupported operand type\n");
    return (resERROR);
  }

  FLAGS_SET_C(1);
  FLAGS_SET_Z(hw_constant == 0);

  return (resGO);
}

int
cl_pblaze::inst_input(uint code, int operands)
{
  application->debug("INPUT\n");

  t_mem port;

  if (operands == REG_REG) {
    cl_memory_cell *r2 = get_register(code, false);
    port = r2->get();
  }
  else if (operands == REG_PORT) {
    port = get_constant(code);
  }
  else {
    fprintf(stderr, "INPUT: Unsupported operand type\n");
    return (resERROR);
  }

  cl_memory_cell *r1 = get_register(code, true);
  r1->set(input_port->get_input(port, ticks->get_ticks() / clock_per_cycle()));

  input_port->value = r1->get();
  port_id->value = port;

  return (resGO);
}

int
cl_pblaze::inst_interrupt(bool value)
{
  if (value)
    application->debug("ENABLE INTERRUPT\n");
  else
    application->debug("DISABLE INTERRUPT\n");

  FLAGS_SET_I(value);

  return (resGO);
}

int
cl_pblaze::inst_jump(uint code, int inst, int operands)
{
  bool jump = false;

  uint address;
  if (operands == ADDRESS10)
    address = get_address10(code);
  else if (operands == ADDRESS12)
    address = get_address12(code);
  else if (operands == REG_REG) {
    cl_memory_cell *r1 = get_register(code, true);
    cl_memory_cell *r2 = get_register(code, false);
    address = ((r1->get() & 0x0f) << 8) | r2->get();
  }
  else {
    fprintf(stderr, "JUMP: Unsupported operand type\n");
    return (resERROR);
  }

  switch (inst) {
    case JUMP:
      application->debug("JUMP\n");
      jump = true;
      break;

    case JUMP_AT:
      application->debug("JUMP@\n");
      jump = true;
      break;

    case JUMP_C:
      application->debug("JUMP C\n");
      jump = FLAGS_GET_C;
      break;

    case JUMP_NC:
      application->debug("JUMP NC\n");
      jump = !FLAGS_GET_C;
      break;

    case JUMP_Z:
      application->debug("JUMP Z\n");
      jump = FLAGS_GET_Z;
      break;

    case JUMP_NZ:
      application->debug("JUMP NZ\n");
      jump = !FLAGS_GET_Z;
      break;

    default:
      fprintf(stderr, "Unsupported JUMP instruction\n");
      return (resERROR);
  }

  if (jump) {
    PC = address;
  }

  return (resGO);
}

int
cl_pblaze::inst_load(uint code, int operands)
{
  application->debug("LOAD\n");

  cl_memory_cell *r1 = get_register(code, true);

  if (operands == REG_CONSTANT) {
      r1->set(get_constant(code));
  }
  else if (operands == REG_REG) {
      r1->set(get_register(code, false)->get());
  }
  else {
    fprintf(stderr, "LOAD: Unsupported operand type\n");
    return (resERROR);
  }

  return (resGO);
}

int
cl_pblaze::inst_load_return(uint code, int operands)
{
  application->debug("LOAD&RETURN\n");

  if (operands == REG_CONSTANT) {
    cl_memory_cell *r1 = get_register(code, true);
    r1->set(get_constant(code));

    u32_t addr;
    if (stack_pop(&addr)) {
      fprintf(stderr, "Call stack underflow.\n");

      if (type->type == CPU_PBLAZE_6)
        reset();

      //return (resERROR);
    }
    PC = addr+1;
  }
  else {
    fprintf(stderr, "LOAD&RETURN: Unsupported operand type\n");
    return (resERROR);
  }

  return (resGO);
}

int
cl_pblaze::inst_or(uint code, int operands)
{
  application->debug("OR\n");

  cl_memory_cell *r1 = get_register(code, true);
  u8_t operand;
  u8_t result;

  if (operands == REG_CONSTANT) {
    operand = get_constant(code);
  }
  else if (operands == REG_REG) {
    operand = get_register(code, false)->get();
  }
  else {
    fprintf(stderr, "OR: Unsupported operand type\n");
    return (resERROR);
  }

  result = r1->get() | operand;

  FLAGS_SET_C(0);
  FLAGS_SET_Z(result == 0);

  r1->set(result);

  return (resGO);
}

int
cl_pblaze::inst_output(uint code, int operands)
{
  application->debug("OUTPUT\n");

  cl_memory_cell *r1 = get_register(code, true);
  int port;
  if (operands == REG_REG) {
    cl_memory_cell *r2 = get_register(code, false);
    port = r2->get();
  }
  else if (operands == REG_PORT) {
    port = get_constant(code);
  }
  else {
    fprintf(stderr, "OUTPUT: Unsupported operand type\n");
    return (resERROR);
  }
  t_mem value = r1->get();
  application->dd_printf("Output on port %d: %2x %3d %c\n", port, value, value, isprint(value) ? value : '.');
  output_port->add_output(port, ticks->get_ticks() / clock_per_cycle(), value);

  output_port->value = value;
  port_id->value = port;

  return (resGO);
}

int
cl_pblaze::inst_outputk(uint code, int operands)
{
  application->debug("OUTPUTK\n");

  u8_t constant;
  u8_t port;

  if (operands == CONSTANT_PORT) {
    constant = (code & 0x00ff0) >> 8;
    port = code & 0x0000f;
  }
  else {
    fprintf(stderr, "OUTPUTK: Unsupported operand type\n");
    return (resERROR);
  }

  application->dd_printf("Output on port %d: %2x %3d %c\n", port, constant, constant, isprint(constant) ? constant : '.');
  output_port->add_output(port, ticks->get_ticks() / clock_per_cycle(), constant);

  output_port->value = constant;
  port_id->value = port;

  return (resGO);
}

int
cl_pblaze::inst_regbank(int regbank)
{
  active_regbank = regbank;

  return (resGO);
}

int
cl_pblaze::inst_return(uint code, int inst)
{
  bool jump = false;

  switch (inst) {
    case RETURN:
      application->debug("RETURN\n");
      jump = true;
      break;

    case RETURN_C:
      application->debug("RETURN C\n");
      jump = FLAGS_GET_C;
      break;

    case RETURN_NC:
      application->debug("RETURN NC\n");
      jump = !FLAGS_GET_C;
      break;

    case RETURN_Z:
      application->debug("RETURN Z\n");
      jump = FLAGS_GET_Z;
      break;

    case RETURN_NZ:
      application->debug("RETURN NZ\n");
      jump = !FLAGS_GET_Z;
      break;

    default:
      fprintf(stderr, "Unsupported RETURN instruction\n");
      return (resERROR);
  }

  if (jump) {
    u32_t addr;
    if (stack_pop(&addr)) {
      fprintf(stderr, "Call stack underflow.\n");

      if (type->type == CPU_PBLAZE_6)
        reset();

      //return (resERROR);
    }
    PC = addr+1;
  }

  return(resGO);
}

int
cl_pblaze::inst_returni(bool interrupt_enable)
{
  u32_t addr;
  if (stack_pop(&addr)) {
    fprintf(stderr, "Call stack underflow.\n");

    if (type->type == CPU_PBLAZE_6)
      reset();

    //return (resERROR);
  }
  PC = addr+1;

  FLAGS_SET_C(interrupt->preserved_flag_c);
  FLAGS_SET_Z(interrupt->preserved_flag_z);
  FLAGS_SET_I(interrupt_enable);

  return (resGO);
}

int
cl_pblaze::inst_rl(uint code, int operands)
{
  application->debug("RL\n");

  cl_memory_cell *r1 = get_register(code, true);
  u8_t value = r1->get();
  u8_t result;

  result = (value << 1) /*||*/ | (value >> 7);
  r1->set(result);

  FLAGS_SET_C(value & 0x80);
  FLAGS_SET_Z(result == 0);

  return (resGO);
}

int
cl_pblaze::inst_rr(uint code, int operands)
{
  application->debug("RR\n");

  cl_memory_cell *r1 = get_register(code, true);
  u8_t value = r1->get();
  u8_t result;

  result = (value >> 1) | (value << 7);
  r1->set(result);

  FLAGS_SET_C(value & 0x01);
  FLAGS_SET_Z(result == 0);

  return (resGO);
}

int
cl_pblaze::inst_sl(uint code, int inst)
{
  cl_memory_cell *r1 = get_register(code, true);
  u8_t value = r1->get();
  u8_t result;

  result = (value << 1);

  switch (inst) {
    case SL0:
      application->debug("SL0\n");
      break;
    case SL1:
      application->debug("SL1\n");
      result |= 0x01;
      break;
    case SLA:
      application->debug("SLA\n");
      result |= (FLAGS_GET_C ? 0x01 : 0x00);
      break;
    case SLX:
      application->debug("SLX\n");
      result |= (value & 0x01);
      break;
  }

  r1->set(result);

  FLAGS_SET_C(value & 0x80);
  FLAGS_SET_Z(result == 0);

  return (resGO);
}

int
cl_pblaze::inst_sr(uint code, int inst)
{
  cl_memory_cell *r1 = get_register(code, true);
  u8_t value = r1->get();
  u8_t result;

  result = (value >> 1);

  switch (inst) {
    case SR0:
      application->debug("SR0\n");
      break;
    case SR1:
      application->debug("SR1\n");
      result |= 0x80;
      break;
    case SRA:
      application->debug("SRA\n");
      result |= (FLAGS_GET_C ? 0x80 : 0x00);
      break;
    case SRX:
      application->debug("SRX\n");
      result |= (value & 0x80);
      break;
  }

  r1->set(result);

  FLAGS_SET_C(value & 0x01);
  FLAGS_SET_Z(result == 0);

  return (resGO);
}

int
cl_pblaze::inst_star(uint code, int operands)
{
  application->debug("STAR\n");

  cl_memory_cell *r1;
  cl_memory_cell *r2 = get_register(code, false);

  if (operands == REG_REG) {
    // getting register from inactive register bank
    unsigned char reg_code = (code & 0x000f0) >> 4;
    if (active_regbank == REGISTER_BANK_A)
      reg_code += 0x10;

    r1 = sfr->get_cell(reg_code);

    r1->set(r2->get());
  }
  else {
    fprintf(stderr, "STAR: Unsupported operand type\n");
    return (resERROR);
  }


  return (resGO);
}

int
cl_pblaze::inst_store(uint code, int operands)
{
  application->debug("STORE\n");

  cl_memory_cell *r1 = get_register(code, true);
  u8_t addr;

  if (operands == REG_ADDRESS6) {
    addr = get_constant(code) & 0x3f; // only bits 5..0 are used for addressing
  }
  else if (operands == REG_ADDRESS8) {
    addr = get_constant(code);
  }
  else if (operands == REG_REG) {
    addr = get_register(code, false)->get();
  }
  else {
    fprintf(stderr, "STORE: Unsupported operand type\n");
    return (resERROR);
  }

  ram->write(addr, r1->get());

  return (resGO);
}

int
cl_pblaze::inst_sub(uint code, int operands)
{
  application->debug("SUB\n");

  cl_memory_cell *r1 = get_register(code, true);
  u8_t operand;
  int result;

  if (operands == REG_CONSTANT) {
      operand = get_constant(code);
  }
  else if (operands == REG_REG) {
      operand = get_register(code, false)->get();
  }
  else {
    fprintf(stderr, "SUB: Unsupported operand type\n");
    return (resERROR);
  }

  result = r1->get() - operand;

  // carry flag
  FLAGS_SET_C(result < 0);

  result = result % 256;

  //zero flag
  FLAGS_SET_Z(result == 0);

  r1->set(result);

  return(resGO);
}

int
cl_pblaze::inst_subcy(uint code, int operands)
{
  application->debug("SUBCY\n");

  cl_memory_cell *r1 = get_register(code, true);
  u8_t operand;
  int result;

  if (operands == REG_CONSTANT) {
      operand = get_constant(code);
  }
  else if (operands == REG_REG) {
      operand = get_register(code, false)->get();
  }
  else {
    fprintf(stderr, "SUBCY: Unsupported operand type\n");
    return (resERROR);
  }

  result = r1->get() - operand - (FLAGS_GET_C ? 1 : 0);

  // carry flag
  FLAGS_SET_C(result < 0);

  result = result % 256;

  //zero flag
  if (type->type == CPU_PBLAZE_3) {
    FLAGS_SET_Z(result == 0);
  }
  else if (type->type == CPU_PBLAZE_6) {
    FLAGS_SET_Z(FLAGS_GET_Z && (result == 0));
  }

  r1->set(result);

  return(resGO);
}

int
cl_pblaze::inst_test(uint code, int operands)
{
  application->debug("TEST\n");

  cl_memory_cell *r1 = get_register(code, true);
  u8_t operand;
  u8_t result;

  if (operands == REG_CONSTANT) {
      operand = get_constant(code);
  }
  else if (operands == REG_REG) {
      operand = get_register(code, false)->get();
  }
  else {
    fprintf(stderr, "TEST: Unsupported operand type\n");
    return (resERROR);
  }

  result = r1->get() & operand;
  //zero flag
  FLAGS_SET_Z(result == 0);

  // algorithm source: http://graphics.stanford.edu/~seander/bithacks.html#ParityNaive
  bool parity = false;
  while (result)
  {
    parity = !parity;
    result = result & (result - 1);
  }

  // carry flag
  FLAGS_SET_C(parity);

  return(resGO);
}

int
cl_pblaze::inst_testcy(uint code, int operands)
{
  application->debug("TESTCY\n");

  cl_memory_cell *r1 = get_register(code, true);
  u8_t operand;
  u8_t result;

  if (operands == REG_CONSTANT) {
      operand = get_constant(code);
  }
  else if (operands == REG_REG) {
      operand = get_register(code, false)->get();
  }
  else {
    fprintf(stderr, "TESTCY: Unsupported operand type\n");
    return (resERROR);
  }

  result = r1->get() & operand;
  //zero flag
  FLAGS_SET_Z((result == 0) && FLAGS_GET_Z);

  // algorithm source: http://graphics.stanford.edu/~seander/bithacks.html#ParityNaive
  bool parity = false;
  while (result)
  {
    parity = !parity;
    result = result & (result - 1);
  }

  // carry flag
  FLAGS_SET_C(parity ^ FLAGS_GET_C);

  return(resGO);
}

int
cl_pblaze::inst_xor(uint code, int operands)
{
  application->debug("XOR\n");

  cl_memory_cell *r1 = get_register(code, true);
  u8_t operand;
  u8_t result;

  if (operands == REG_CONSTANT) {
      operand = get_constant(code);
  }
  else if (operands == REG_REG) {
      operand = get_register(code, false)->get();
  }
  else {
    fprintf(stderr, "XOR: Unsupported operand type\n");
    return (resERROR);
  }

  result = r1->get() ^ operand;
  r1->set(result);

  FLAGS_SET_C(0);
  FLAGS_SET_Z(result == 0);

  return(resGO);
}

/* End of pblaze.src/inst.cc */
