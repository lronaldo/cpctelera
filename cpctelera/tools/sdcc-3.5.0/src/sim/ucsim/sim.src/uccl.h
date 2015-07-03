/*
 * Simulator of microcontrollers (sim.src/uccl.h)
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

#ifndef SIM_UCCL_HEADER
#define SIM_UCCL_HEADER

// prj
#include "stypes.h"
#include "pobjcl.h"

// sim
#include "hwcl.h"
#include "memcl.h"
#include "brkcl.h"
#include "stackcl.h"


/* Counter to count clock ticks */

#define TICK_RUN        0x01
#define TICK_INISR      0x02
#define TICK_IDLE       0x03

class cl_ticker: public cl_base
{
public:
  unsigned long ticks;
  int options; // see TICK_XXX above
  int dir;
  //char *name;

  cl_ticker(int adir, int in_isr, const char *aname);
  virtual ~cl_ticker(void);

  virtual int tick(int nr);
  virtual double get_rtime(double xtal);
  virtual void dump(int nr, double xtal, class cl_console_base *con);
};


/* Options of the microcontroller */
class cl_xtal_option: public cl_optref
{
protected:
  class cl_uc *uc;
public:
  cl_xtal_option(class cl_uc *the_uc);
  virtual void option_changed(void);
};

/* Abstract microcontroller */

class cl_uc: public cl_base
{
public:
  int type;                     // CPU family
  int technology;               // CMOS, HMOS
  int state;                    // GO, IDLE, PD
  //class cl_list *options;
  class cl_xtal_option *xtal_option;

  t_addr PC, instPC;            // Program Counter
  bool inst_exec;               // Instruction is executed
  class cl_ticker *ticks;       // Nr of XTAL clocks
  class cl_ticker *isr_ticks;   // Time in ISRs
  class cl_ticker *idle_ticks;  // Time in idle mode
  class cl_list *counters;      // User definable timers (tickers)
  int inst_ticks;               // ticks of an instruction
  double xtal;                  // Clock speed

  int brk_counter;              // Number of breakpoints
  class brk_coll *fbrk;         // Collection of FETCH break-points
  class brk_coll *ebrk;         // Collection of EVENT breakpoints
  class cl_sim *sim;
  //class cl_list *mems;
  class cl_hws *hws;

  class cl_list *memchips;      // v3
  class cl_address_space_list *address_spaces;
  class cl_address_space *rom;  // Required for almost every uc

  //class cl_list *address_decoders;

  class cl_irqs *it_sources;    // Sources of interrupts
  class cl_list *it_levels;     // Follow interrupt services
  class cl_list *stack_ops;     // Track stack operations

  class cl_list *errors;        // Errors of instruction execution
  class cl_list *events;        // Events happened during inst exec

  t_addr sp_max;
  t_addr sp_avg;

public:
  cl_uc(class cl_sim *asim);
  virtual ~cl_uc(void);
  virtual int init(void);
  virtual const char *id_string(void);
  virtual void reset(void);

  // making objects
  //virtual class cl_m *mk_mem(enum mem_class type, char *class_name);
  virtual void make_memories(void);
  //virtual t_addr get_mem_size(char *id);
  //virtual int get_mem_width(char *id);
  virtual void mk_hw_elements(void);
  virtual void build_cmdset(class cl_cmdset *cmdset);

  // manipulating memories
  virtual t_mem read_mem(const char *id, t_addr addr);
  virtual t_mem get_mem(const char *id, t_addr addr);
  virtual void write_mem(const char *id, t_addr addr, t_mem val);
  virtual void set_mem(const char *id, t_addr addr, t_mem val);
  virtual class cl_address_space *address_space(const char *id);
  virtual class cl_memory *memory(const char *id);

  // file handling
  virtual long read_hex_file(const char *nam);

  // instructions, code analyzer
  virtual void analyze(t_addr addr) {}
  virtual bool inst_at(t_addr addr);
  virtual void set_inst_at(t_addr addr);
  virtual void del_inst_at(t_addr addr);
  virtual bool there_is_inst(void);

  // manipulating hw elements
  virtual class cl_hw *get_hw(enum hw_cath cath, int *idx);
  virtual class cl_hw *get_hw(char *id_string, int *idx);
  virtual class cl_hw *get_hw(enum hw_cath cath, int hwid, int *idx);
  virtual class cl_hw *get_hw(char *id_string, int hwid, int *idx);

  // "virtual" timers
  virtual int tick_hw(int cycles);
  virtual void do_extra_hw(int cycles);
  virtual int tick(int cycles);
  virtual class cl_ticker *get_counter(int nr);
  virtual class cl_ticker *get_counter(char *nam);
  virtual void add_counter(class cl_ticker *ticker, int nr);
  virtual void add_counter(class cl_ticker *ticker, char *nam);
  virtual void del_counter(int nr);
  virtual void del_counter(char *nam);
  virtual double get_rtime(void);
  virtual int clock_per_cycle(void);

  // execution
  virtual t_mem fetch(void);
  virtual bool fetch(t_mem *code);
  virtual int do_inst(int step);
  virtual void pre_inst(void);
  virtual int exec_inst(void);
  virtual void post_inst(void);

  virtual int it_priority(uchar ie_mask) {return(0);}

  // stack tracking
  virtual void stack_write(class cl_stack_op *op);
  virtual void stack_read(class cl_stack_op *op);

  // breakpoints
  virtual class cl_fetch_brk *fbrk_at(t_addr addr);
  virtual class cl_ev_brk *ebrk_at(t_addr addr, char *id);
  virtual class cl_brk *brk_by_nr(int nr);
  virtual class cl_brk *brk_by_nr(class brk_coll *bpcoll, int nr);
  virtual void rm_ebrk(t_addr addr, char *id);
  virtual bool rm_brk(int nr);
  virtual void put_breaks(void);
  virtual void remove_all_breaks(void);
  virtual int make_new_brknr(void);
  virtual class cl_ev_brk *mk_ebrk(enum brk_perm perm,
                                   class cl_address_space *mem,
                                   char op, t_addr addr, int hit);
  virtual void check_events(void);

  // disassembling and symbol recognition
  virtual const char *disass(t_addr addr, const char *sep);
  virtual struct dis_entry *dis_tbl(void);
  virtual struct name_entry *sfr_tbl(void);
  virtual struct name_entry *bit_tbl(void);
  virtual void print_disass(t_addr addr, class cl_console_base *con);
  virtual void print_regs(class cl_console_base *con);
  virtual int inst_length(t_addr addr);
  virtual int inst_branch(t_addr addr);
  virtual int longest_inst(void);
  virtual bool get_name(t_addr addr, struct name_entry tab[], char *buf);
  virtual bool symbol2address(char *sym, struct name_entry tab[],
                              t_addr *addr);
  virtual char *symbolic_bit_name(t_addr bit_address,
                                  class cl_memory *mem,
                                  t_addr mem_addr,
                                  t_mem bit_mask);

  /* Converting abstract address spaces into real ones */
  virtual class cl_address_space *bit2mem(t_addr bitaddr,
                                          t_addr *memaddr,
                                          t_mem *bitmask);
  virtual t_addr bit_address(class cl_memory *mem,
                             t_addr mem_address,
                             int bit_number) { return(-1); }

  // messages from app to handle and broadcast
  virtual bool handle_event(class cl_event &event);
  //virtual void mem_cell_changed(class cl_address_space *mem, t_addr addr);
  virtual void address_space_added(class cl_address_space *as);

  // Error handling
  virtual void error(class cl_error *error);
  virtual void check_errors(void);

  /* Following fields and virtual methods defined in uc51 I don't have
     energy to redesign them:-( */
public:
  virtual void eram2xram(void) {} // Dirty hack for 51R
  virtual void xram2eram(void) {}
};


/*
 * Errors
 */

#include "errorcl.h"

class cl_error_unknown_code: public cl_error
{
protected:
  class cl_uc *uc;
public:
  cl_error_unknown_code(class cl_uc *the_uc);

  virtual void print(class cl_commander_base *c);
};

class cl_uc_error_registry: public cl_error_registry
{
public:
  cl_uc_error_registry(void);
};

#endif


/* End of uccl.h */
