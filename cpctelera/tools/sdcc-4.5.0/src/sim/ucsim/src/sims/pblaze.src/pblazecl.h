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



#ifndef PBLAZECL_HEADER
#define PBLAZECL_HEADER

#include <list>

#include "uccl.h"

#include "regspblaze.h"
#include "interruptcl.h"
#include "portidcl.h"
#include "inputportcl.h"
#include "outputportcl.h"

using namespace std;




/*
 * Base type of PicoBlaze microcontrollers
 */

class cl_pblaze: public cl_uc
{
public:
 unsigned int ram_size;
 unsigned int rom_size;
 unsigned int stack_size;
 unsigned int interrupt_vector;
 u8_t hw_constant;
 int active_regbank;

public:
  class cl_address_space *ram;
  class cl_address_space *rom;
  class cl_address_space *stack;
  class cl_address_space *sfr;

  // Simulation of interrupt system
  class cl_interrupt *interrupt;
  list<long> stored_interrupts;

  class cl_port_id *port_id;
  class cl_output_port *output_port;
  class cl_input_port *input_port;

public:
  cl_pblaze(struct cpu_entry *cputype, class cl_sim *asim);
  virtual ~cl_pblaze(void);
  virtual int init(void);
  virtual const char *id_string(void);

  virtual cl_address_space * bit2mem(t_addr bitaddr, t_addr *memaddr, t_mem *bitmask);
  virtual t_addr get_mem_size(enum mem_class type);
  virtual void mk_hw_elements(void);
  virtual void make_memories(void);
  virtual void build_cmdset(class cl_cmdset *cmdset);

  virtual long read_hex_file(const char *nam);

  virtual struct dis_entry *dis_tbl(void);
  virtual struct name_entry *sfr_tbl(void);
  virtual struct name_entry *bit_tbl(void);

  virtual double def_xtal(void) { return 8000000; }
  virtual int clock_per_cycle(void) { return(2); }
  virtual int inst_length(t_addr addr);
  virtual int inst_branch(t_addr addr);
  virtual int longest_inst(void);
  virtual char *disass(t_addr addr);
  virtual void print_regs(class cl_console_base *con);
  virtual void print_state(class cl_console_base *con, char *file_name);
  virtual void load_state(class cl_console_base *con, char *file_name);
  virtual void read_interrupt_file(void);
  virtual void read_input_file(void);

  virtual int exec_inst(void);
  virtual int do_interrupt(void);

  virtual const char *get_disasm_info(t_addr addr,
                        int *ret_len,
                        int *ret_branch,
                        int *immed_offset);

  virtual void reset(void);
#include "instcl.h"
#include "stackcl.h"

private:
  void init_uc_parameters(void);
  virtual long std_read_hex_file(const char *nam);
  virtual long pblaze_read_hex_file(const char *nam);

};

#endif

/* End of pblaze.src/pblazecl.h */
