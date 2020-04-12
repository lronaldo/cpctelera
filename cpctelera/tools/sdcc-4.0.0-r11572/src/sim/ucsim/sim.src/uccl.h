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
#include "pobjt.h"

// sim
#include "hwcl.h"
#include "memcl.h"
#include "brkcl.h"
#include "stackcl.h"
#include "varcl.h"


class cl_uc;

typedef int (*instruction_wrapper_fn)(class cl_uc *uc, t_mem code);

/* Counter to count clock ticks */

#define TICK_RUN	0x01
#define TICK_INISR	0x02
#define TICK_IDLE	0x03

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

struct vcounter_t {
  t_mem inst;
  t_mem fetch;
  t_mem rd;
  t_mem wr;
};

class cl_time_measurer: public cl_base
{
public:
  unsigned long to_reach;
  class cl_uc *uc;
public:
  cl_time_measurer(class cl_uc *the_uc);
  virtual void set_reach(unsigned long val);
  virtual void from_now(unsigned long val);
  virtual bool reached();
  virtual unsigned long now();
};

class cl_time_clk: public cl_time_measurer
{
public:
  cl_time_clk(class cl_uc *the_uc): cl_time_measurer(the_uc) { set_name("clk"); }
  virtual unsigned long now();
};
  
class cl_time_vclk: public cl_time_measurer
{
public:
  cl_time_vclk(class cl_uc *the_uc): cl_time_measurer(the_uc) { set_name("vclk"); }
  virtual unsigned long now();
};

class cl_time_fclk: public cl_time_measurer
{
public:
  cl_time_fclk(class cl_uc *the_uc): cl_time_measurer(the_uc) { set_name("fclk"); }
  virtual unsigned long now();
};

class cl_time_rclk: public cl_time_measurer
{
public:
  cl_time_rclk(class cl_uc *the_uc): cl_time_measurer(the_uc) { set_name("rclk"); }
  virtual unsigned long now();
};

class cl_time_wclk: public cl_time_measurer
{
public:
  cl_time_wclk(class cl_uc *the_uc): cl_time_measurer(the_uc) { set_name("wclk"); }
  virtual unsigned long now();
};


class cl_omf_rec: public cl_base
{
 protected:
  unsigned int f_offset, offset;
 public:
  u8_t type;
  u16_t len;
  u8_t *rec;
  u8_t chk;
 public:
  cl_omf_rec(void);
  virtual ~cl_omf_rec(void);
  virtual unsigned char g(cl_f *f);
  virtual u16_t pick_word(int i);
  virtual chars pick_str(int i);
  virtual bool read(cl_f *f);
};

class cl_cdb_rec: public cl_base
{
 public:
  chars fname;
  t_addr addr;
 public:
 cl_cdb_rec(chars fn): cl_base() { fname= fn; }
 cl_cdb_rec(chars fn, t_addr a): cl_base() { fname= fn; addr= a; }
};

class cl_cdb_recs: public cl_sorted_list
{
 public:
 cl_cdb_recs(): cl_sorted_list(2,2,"cdb_recs_list") {}
  virtual void *key_of(void *item)
  { return (char*)(((cl_cdb_rec *)item)->fname); }
  virtual int compare(void *k1, void *k2) {
    return strcmp((char*)k1,(char*)k2);
  }
  virtual cl_cdb_rec *rec(chars n) {
    t_index i;
    if (search((char*)n, i))
      return (cl_cdb_rec*)(at(i));
    return NULL;
  }
  virtual void del(chars n) {
    t_index i;
    if (search((char*)n,i))
      free_at(i);
  }
};

/* Abstract microcontroller */

class cl_uc: public cl_base
{
public:
  struct cpu_entry *type;
  //enum cpu_type type;			// CPU family
  //int technology;		// CMOS, HMOS
  int state;			// GO, IDLE, PD
  //class cl_list *options;
  class cl_xtal_option *xtal_option;

  t_addr PC, instPC;		// Program Counter
  bool inst_exec;		// Instruction is executed
  class cl_ticker *ticks;	// Nr of XTAL clocks
  class cl_ticker *isr_ticks;	// Time in ISRs
  class cl_ticker *idle_ticks;	// Time in idle mode
  class cl_list *counters;	// User definable timers (tickers)
  int inst_ticks;		// ticks of an instruction
  double xtal;			// Clock speed
  struct vcounter_t vc;		// Virtual clk counter
  
  int brk_counter;		// Number of breakpoints
  class brk_coll *fbrk;		// Collection of FETCH break-points
  class brk_coll *ebrk;		// Collection of EVENT breakpoints
  class cl_sim *sim;
  //class cl_list *mems;
  class cl_time_measurer *stop_at_time;
 public:
  class cl_hw *cpu;
  class cl_hws *hws;

 public:
  class cl_list *memchips;      // v3
  class cl_address_space_list *address_spaces;
  class cl_address_space *rom;  // Required for almost every uc
  //class cl_list *address_decoders;
  class cl_address_space *variables;
  class cl_var_list *vars;

  bool irq;
  class cl_irqs *it_sources;	// Sources of interrupts
  class cl_list *it_levels;	// Follow interrupt services
  class cl_list *stack_ops;	// Track stack operations

  class cl_list *errors;	// Errors of instruction execution
  class cl_list *events;	// Events happened during inst exec

  t_addr sp_max;
  t_addr sp_avg;

public:
  cl_uc(class cl_sim *asim);
  virtual ~cl_uc(void);
  virtual int init(void);
  virtual char *id_string(void);
  virtual void reset(void);

  // making objects
  virtual void make_memories(void);
  virtual void make_variables(void);
  virtual void make_cpu_hw(void);
  virtual void mk_hw_elements(void);
  virtual void build_cmdset(class cl_cmdset *cmdset);

  // manipulating memories
  virtual t_mem read_mem(char *id, t_addr addr);
  virtual t_mem get_mem(char *id, t_addr addr);
  virtual void write_mem(char *id, t_addr addr, t_mem val);
  virtual void set_mem(char *id, t_addr addr, t_mem val);
  virtual class cl_address_space *address_space(const char *id);
  virtual class cl_address_space *address_space(class cl_memory_cell *cell);
  virtual class cl_address_space *address_space(class cl_memory_cell *cell, t_addr *addr);
  virtual class cl_memory *memory(const char *id);

  // file handling
  virtual void set_rom(t_addr addr, t_mem val);
  virtual long read_hex_file(const char *nam);
  virtual long read_hex_file(cl_console_base *con);
  virtual long read_hex_file(cl_f *f);
  virtual long read_omf_file(cl_f *f);
  virtual long read_cdb_file(cl_f *f);
  virtual cl_f *find_loadable_file(chars nam);
  virtual long read_file(chars nam, class cl_console_base *con);
  
  // instructions, code analyzer
  virtual void analyze(t_addr addr) {}
  virtual bool inst_at(t_addr addr);
  virtual void set_inst_at(t_addr addr);
  virtual void del_inst_at(t_addr addr);
  virtual bool there_is_inst(void);

  // manipulating hw elements
  virtual void add_hw(class cl_hw *hw);
  virtual int nuof_hws(void);
  virtual class cl_hw *get_hw(int idx);
  virtual class cl_hw *get_hw(enum hw_cath cath, int *idx);
  virtual class cl_hw *get_hw(char *id_string, int *idx);
  virtual class cl_hw *get_hw(enum hw_cath cath, int hwid, int *idx);
  virtual class cl_hw *get_hw(char *id_string, int hwid, int *idx);
  virtual int get_max_hw_id(enum hw_cath cath);
  
  // "virtual" timers
  virtual int tick_hw(int cycles);
  virtual void do_extra_hw(int cycles);
  virtual int tick(int cycles);
  virtual class cl_ticker *get_counter(int nr);
  virtual class cl_ticker *get_counter(const char *nam);
  virtual void add_counter(class cl_ticker *ticker, int nr);
  virtual void add_counter(class cl_ticker *ticker, const char *nam);
  virtual void del_counter(int nr);
  virtual void del_counter(const char *nam);
  virtual double get_rtime(void);
  virtual unsigned long clocks_of_time(double t);
  virtual int clock_per_cycle(void);
  virtual void touch(void);
  
  // execution
  virtual t_mem fetch(void);
  virtual bool fetch(t_mem *code);
  virtual int do_inst(int step);
  virtual void pre_inst(void);
  virtual int exec_inst(void);
  virtual int exec_inst_tab(instruction_wrapper_fn itab[]);
  virtual void post_inst(void);

  virtual int do_interrupt(void);
  virtual int priority_of(uchar nuof_it) {return(0);}
  virtual int priority_main() { return 0; }
  virtual int accept_it(class it_level *il);
  virtual bool it_enabled(void) { return false; }

#include "uccl_instructions.h"
  
  // stack tracking
  virtual void stack_write(class cl_stack_op *op);
  virtual void stack_read(class cl_stack_op *op);
  virtual void stack_check_overflow(class cl_stack_op *op);
  
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
  virtual void stop_when(class cl_time_measurer *t);
  
  // disassembling and symbol recognition
  virtual char *disass(t_addr addr, const char *sep);
  virtual struct dis_entry *dis_tbl(void);
  virtual void print_disass(t_addr addr, class cl_console_base *con);
  virtual void print_regs(class cl_console_base *con);
  virtual int inst_length(t_addr addr);
  virtual int inst_branch(t_addr addr);
  virtual bool is_call(t_addr addr);
  virtual int longest_inst(void);
  virtual bool addr_name(t_addr addr, class cl_address_space *as, char *buf);
  virtual bool addr_name(t_addr addr, class cl_address_space *as, int bitnr, char *buf);
  virtual bool symbol2address(char *sym,
			      class cl_address_space **as,
			      t_addr *addr);
  virtual char *symbolic_bit_name(t_addr bit_address,
				  class cl_memory *mem,
				  t_addr mem_addr,
				  t_mem bit_mask);
  virtual name_entry *get_name_entry(struct name_entry tabl[],
				     char *name);
  virtual chars cell_name(class cl_memory_cell *cell);
  virtual class cl_var *var(char *nam);
  
  /* Converting abstract address spaces into real ones */
  virtual class cl_address_space *bit2mem(t_addr bitaddr,
					  t_addr *memaddr,
					  t_mem *bitmask);
  virtual t_addr bit_address(class cl_memory *mem,
                             t_addr mem_address,
                             int bit_number) { return(-1); }

  // messages from app to handle and broadcast
  virtual bool handle_event(class cl_event &event);
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
