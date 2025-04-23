/*
 * Simulator of microcontrollers (memcl.h)
 *
 * Copyright (C) 1999 Drotos Daniel
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

#ifndef SIM_MEMCL_HEADER
#define SIM_MEMCL_HEADER

#include <stdio.h>

#include "ddconfig.h"

// prj
#include "stypes.h"
#include "pobjcl.h"

// gui.src
#include "guiobjcl.h"


class cl_event_handler;

// Cell flags
enum cell_flag {
  CELL_NONE		= 0x00,
  CELL_INST		= 0x04,	/* Marked as instruction */
  CELL_FETCH_BRK	= 0x08,	/* Fetch breakpoint */
  CELL_READ_ONLY	= 0x10, /* Cell is readonly */
  CELL_NON_DECODED	= 0x40	/* Cell is not decoded (yet) */
};


#define CELL_GENERAL	(CELL_NORMAL|CELL_INST|CELL_FETCH_BRK)

extern t_mem def_data;

class cl_dump_ads: public cl_base
{
public:
  t_addr start, stop;
  bool use_start, use_stop;
public:
  cl_dump_ads(void): cl_base()
  {
    start= 0; stop= 0;
    use_start= false; use_stop= false;
  }
  cl_dump_ads(t_addr the_start): cl_base()
  {
    start= the_start; stop= 0;
    use_start= true; use_stop= false;
  }
  cl_dump_ads(t_addr the_start,
	      t_addr the_stop): cl_base()
  {
    start= the_start; stop= the_stop;
    use_start= true; use_stop= true;
  }
  cl_dump_ads(t_addr the_start, bool the_use_start,
	      t_addr the_stop, bool the_use_stop): cl_base()
  {
    start= the_start; stop= the_stop;
    use_start= the_use_start; use_stop= the_use_stop;
  }
  cl_dump_ads(const cl_dump_ads &da)
  {
    start= da.start; stop= da.stop;
    use_start= da.use_start; use_stop= da.use_stop;
  }
public:
  virtual void _start(t_addr the_start) {
    start= the_start;
    use_start= true;
  }
  virtual void _stop(t_addr the_stop) {
    stop= the_stop;
    use_stop= true;
  }
  virtual void _ss(t_addr the_start, t_addr the_stop) {
    start= the_start; stop= the_stop;
    use_start= true; use_stop= true;
  }
  virtual void _ss7(t_addr the_start) {
    start= the_start; stop= the_start+7;
    use_start= true; use_stop= true;
  }
};

  
/*
 * 3rd version of memory system
 */

class cl_memory: public cl_base
{
private:
  unsigned int uid;
public:
  t_addr start_address;
protected:
  class cl_uc *uc;
  t_addr size;
public:
  char *addr_format, *data_format;
  int width; // in bits
  t_mem data_mask;
  bool hidden;
  chars altname;
protected:
  t_addr dump_finished;
public:
  cl_memory(const char *id, t_addr asize, int awidth);
  virtual ~cl_memory(void);
  virtual bool is_named(const char *the_name) const;
  virtual bool is_inamed(const char *the_name) const;
  virtual int init(void);

  t_addr get_start_address(void) { return(start_address); }
  t_addr get_size(void) { return(size); }
  unsigned int get_uid() { return uid; }
  virtual void set_uc(class cl_uc *auc) { uc= auc; }
  virtual bool valid_address(t_addr addr);
  virtual t_addr inc_address(t_addr addr, int val);
  virtual t_addr inc_address(t_addr addr);
  virtual t_addr validate_address(t_addr addr);

  virtual bool is_chip(void) { return(false); }
  virtual bool is_address_space(void) { return(false); }
  
  virtual void err_inv_addr(t_addr addr);
  virtual void err_non_decoded(t_addr addr);

  virtual t_addr dump(int smart,
		      //t_addr start, t_addr stop,
		      class cl_dump_ads *ads,
		      int bitnr_high, int bitnr_low,
		      int bpl,
		      class cl_console_base *con);
  virtual t_addr dump(int smart,
		      //t_addr start, t_addr stop,
		      class cl_dump_ads *ads,
		      int bpl,
		      class cl_console_base *con) {
    return dump(smart, /*start, stop*/ads, -1, -1, bpl, con);
  }
  virtual t_addr dump(t_addr addr,
		      int bitnr_high, int bitnr_low,
		      class cl_console_base *con) {
    cl_dump_ads ads(addr, true, addr, true);
    return dump(2,
		//addr, addr,
		&ads,
		bitnr_high, bitnr_low,
		-1,
		con);
  }
  virtual t_addr dump_s(//t_addr start, t_addr stop,
			class cl_dump_ads *ads,
			int bpl,
			class cl_console_base *con);
  virtual t_addr dump_b(//t_addr start, t_addr stop,
			class cl_dump_ads *ads,
			int bpl,
			class cl_console_base *con);
  virtual t_addr dump_i(//t_addr start, t_addr stop,
			class cl_dump_ads *ads,
			int bpl,
			class cl_console_base *con);
  virtual t_addr dump(//t_addr start, t_addr stop,
		      class cl_dump_ads *ads,
		      int bpl,
		      class cl_console_base *con) {
    return dump(1, /*start, stop*/ads, bpl, con);
  }
  //virtual t_addr dump(class cl_console_base *con) { return(dump(df_smart, -1, -1, -1, con)); }
  virtual bool search_next(bool case_sensitive,
			   t_mem *array, int len, t_addr *addr);


  virtual t_addr lowest_valid_address(void) { return(start_address); }
  virtual t_addr highest_valid_address(void) { return(start_address+size-1); }

  virtual t_mem read(t_addr addr)=0;
  virtual t_mem read(t_addr addr, enum hw_cath skip)=0;
  virtual t_mem get(t_addr addr)=0;
  //virtual t_mem get8(t_addr addr)=0;
  //virtual t_mem get16(t_addr addr)=0;
  virtual t_mem write(t_addr addr, t_mem val)=0;
  virtual void set(t_addr addr, t_mem val)=0;

  virtual void print_info(const char *pre, class cl_console_base *con);
};


/*
 * Operators for memory cells
 */

class cl_banker;

class cl_memory_operator: public cl_base
{
public:
  t_mem mask;
  class cl_memory_cell *cell;
public:
  cl_memory_operator(class cl_memory_cell *acell);

  virtual bool match(class cl_hw *the_hw) { return(false); }
  virtual bool match(class cl_brk *brk) { return(false); }

  virtual t_mem read(void);
  virtual t_mem read(enum hw_cath skip) { return(read()); }
  virtual t_mem write(t_mem val);

  virtual class cl_banker *get_banker(void) { return NULL; }
};

class cl_bank_switcher_operator: public cl_memory_operator
{
 protected:
  class cl_banker *banker;
 public:
  cl_bank_switcher_operator(class cl_memory_cell *acell,
			    class cl_banker *the_banker);
  
  virtual t_mem write(t_mem val);
  virtual class cl_banker *get_banker(void) { return banker; }
};

class cl_hw_operator: public cl_memory_operator
{
protected:
  class cl_hw *hw;
public:
  cl_hw_operator(class cl_memory_cell *acell,
		 class cl_hw *ahw);

  virtual bool match(class cl_hw *the_hw) { return(hw == the_hw); }

  virtual t_mem read(void);
  virtual t_mem read(enum hw_cath skip);
  virtual t_mem write(t_mem val);
};

class cl_event_break_operator: public cl_memory_operator
{
protected:
  class cl_uc *uc;
  class cl_brk *bp;
public:
  cl_event_break_operator(class cl_memory_cell *acell,
			  class cl_uc *auc, class cl_brk *the_bp):
  cl_memory_operator(acell)
  {
    uc= auc;
    bp= the_bp;
  }

  virtual bool match(class cl_brk *brk) { return(bp == brk); }
};

class cl_write_operator: public cl_event_break_operator
{
public:
  cl_write_operator(class cl_memory_cell *acell,
		    class cl_uc *auc, class cl_brk *the_bp);

  virtual t_mem write(t_mem val);
};

class cl_read_operator: public cl_event_break_operator
{
public:
  cl_read_operator(class cl_memory_cell *acell,
		   class cl_uc *auc, class cl_brk *the_bp);

  virtual t_mem read(void);
};


/*
 * version 3 of cell
 */

class cl_cell_data: public cl_abs_base
{
 protected:
  /*t_mem*/void *data;
  virtual t_mem d()=0;
  virtual void d(t_mem v)=0;
  virtual void dl(t_mem v) { d(v); };
};

class cl_memory_cell: public cl_cell_data
{
#ifdef STATISTIC
 public:
  unsigned long nuof_writes, nuof_reads;
  class cl_memory *as;
#endif
 public:
  t_mem mask;
  //t_mem def_data;
 protected:
  uchar width;
  uchar flags;
  //class cl_memory_operator *operators;
  class cl_memory_operator **ops;
  //u64_t fillup;
 public:
  cl_memory_cell();
  cl_memory_cell(uchar awidth);
  virtual ~cl_memory_cell(void);
  virtual int init(void);
  virtual void set_width(uchar awidth);
  
  virtual /*t_mem*/void *get_data(void) { return(data); }
  virtual t_mem get_mask(void) { return(mask); }
  virtual void set_mask(t_mem m) { mask= m; }
  virtual uchar get_flags(void);
  virtual bool get_flag(enum cell_flag flag);
  virtual void set_flags(uchar what);
  virtual void set_flag(enum cell_flag flag, bool val);
  virtual uchar get_width(void) { return width; }
  
  virtual void un_decode(void);
  virtual void decode(class cl_memory_chip *chip, t_addr addr);
  virtual void decode(void *data_ptr);
  virtual void decode(void *data_ptr, t_mem bit_mask);
  
  virtual t_mem read(void);
  virtual t_mem R(void) { return read(); }
  virtual t_mem read(enum hw_cath skip);
  virtual t_mem get(void);
  virtual t_mem write(t_mem val);
  virtual t_mem W(t_mem val) { return write(val); }
  virtual t_mem set(t_mem val);
  virtual t_mem download(t_mem val);

  virtual int nuof_ops(void);
  virtual void append_operator(class cl_memory_operator *op);
  virtual void prepend_operator(class cl_memory_operator *op);
  virtual void remove_operator(class cl_memory_operator *op);
  virtual void del_operator(class cl_brk *brk);
  virtual void del_operator(class cl_hw *hw);
  virtual class cl_banker *get_banker(void);
  virtual void set_brk(class cl_uc *uc, class cl_brk *brk);
  virtual void del_brk(class cl_brk *brk);
  
  virtual class cl_memory_cell *add_hw(class cl_hw *hw/*, t_addr addr*/);
  virtual void remove_hw(class cl_hw *hw);
  virtual class cl_event_handler *get_event_handler(void);

  virtual void print_info(const char *pre, class cl_console_base *con);
  virtual void print_operators(const char *pre, class cl_console_base *con);
};

/*
class cl_bit_cell: public cl_memory_cell
{
 public:
  cl_bit_cell(uchar awidth): cl_memory_cell(awidth) {}
  virtual t_mem d();
  virtual void d(t_mem v);
};
*/

/* Cell for 1 byte storage */

class cl_cell8: public cl_memory_cell
{
 public:
  cl_cell8(uchar awidth): cl_memory_cell(awidth) {}
  cl_cell8(): cl_memory_cell(8) {}
  virtual t_mem d();
  virtual void d(t_mem v);
  virtual void dl(t_mem v);
};

class cl_bit_cell8: public cl_memory_cell
{
 public:
  cl_bit_cell8(uchar awidth): cl_memory_cell(awidth) {}
  cl_bit_cell8(): cl_memory_cell(8) {}
  virtual t_mem d();
  virtual void d(t_mem v);
};


/* Cell for 2 byte storage */

class cl_cell16: public cl_memory_cell
{
 public:
  cl_cell16(uchar awidth): cl_memory_cell(awidth) {}
  cl_cell16(): cl_memory_cell(16) {}
  virtual t_mem d();
  virtual void d(t_mem v);
  virtual void dl(t_mem v);
};

class cl_bit_cell16: public cl_memory_cell
{
 public:
  cl_bit_cell16(uchar awidth): cl_memory_cell(awidth) {}
  virtual t_mem d();
  virtual void d(t_mem v);
};


/* Cell for 4 byte storage */

class cl_cell32: public cl_memory_cell
{
 public:
  cl_cell32(uchar awidth): cl_memory_cell(awidth) {}
  cl_cell32(): cl_memory_cell(32) {}
  virtual t_mem d();
  virtual void d(t_mem v);
  virtual void dl(t_mem v);
};

class cl_mc32: public cl_cell32
{
public:
  cl_mc32(uchar awidth): cl_cell32(awidth)
  {
    data= &def_data;
    flags&= ~CELL_NON_DECODED;
  }
  cl_mc32(): cl_cell32(32)
  {
    data= &def_data;
    flags&= ~CELL_NON_DECODED;
  }
};

class cl_bit_cell32: public cl_memory_cell
{
 public:
  cl_bit_cell32(uchar awidth): cl_memory_cell(awidth) {}
  virtual t_mem d();
  virtual void d(t_mem v);
};


/* Dummy cell */

class cl_dummy_cell: public cl_memory_cell
{
public:
  cl_dummy_cell(): cl_memory_cell() {}
  cl_dummy_cell(uchar awidth): cl_memory_cell(awidth) {}

  virtual t_mem d();
  virtual void d(t_mem v);
  virtual t_mem write(t_mem val);
  virtual t_mem set(t_mem val);
};


/*
 * Address space
 */

class cl_memory_chip;

class cl_address_space: public cl_memory
{
 public:
  class cl_memory_cell /* **cells,*/ *dummy;
 protected:
  class cl_memory_cell *cella;
 public:
  class cl_decoder_list *decoders;
 public:
  cl_address_space(const char *id, t_addr astart, t_addr asize, int awidth);
  virtual ~cl_address_space(void);
  virtual int init(void);
  virtual class cl_memory_cell *cell_template();
  
  virtual bool is_address_space(void) { return(true); }

  virtual t_mem read(t_addr addr);
  virtual t_mem read(t_addr addr, enum hw_cath skip);
  virtual t_mem get(t_addr addr);
  //virtual t_mem get8(t_addr addr) { return get(addr)&0xff; }
  //virtual t_mem get16(t_addr addr) { return get(addr)&0xffff; }
  virtual t_mem write(t_addr addr, t_mem val);
  virtual void set(t_addr addr, t_mem val);
  virtual void download(t_addr, t_mem val);
  
  virtual class cl_memory_cell *get_cell(t_addr addr);
  virtual int get_cell_flag(t_addr addr);
  virtual bool get_cell_flag(t_addr addr, enum cell_flag flag);
  virtual void set_cell_flag(t_addr addr, bool set_to, enum cell_flag flag);
  virtual void set_cell_flag(t_addr start_addr, t_addr end_addr, bool set_to, enum cell_flag flag);
  virtual class cl_memory_cell *search_cell(enum cell_flag flag, bool value,
					    t_addr *addr);
  virtual bool is_owned(class cl_memory_cell *cell, t_addr *addr);
  
  virtual class cl_address_decoder *get_decoder_of(t_addr addr);
  virtual bool decode_cell(t_addr addr,
			   class cl_memory_chip *chip, t_addr chipaddr);
  virtual void undecode_cell(t_addr addr);
  virtual void undecode_area(class cl_address_decoder *skip,
			     t_addr begin, t_addr end, class cl_console_base *con);

  virtual class cl_memory_cell *register_hw(t_addr addr, class cl_hw *hw,
					    bool announce);
  virtual void unregister_hw(class cl_hw *hw);

  virtual void set_brk(t_addr addr, class cl_brk *brk);
  virtual void del_brk(t_addr addr, class cl_brk *brk);

#ifdef STATISTIC
  virtual unsigned long get_nuof_reads(void) { return(0); }
  virtual unsigned long get_nuof_writes(void) { return(0); }
  virtual void set_nuof_reads(unsigned long value) {}
  virtual void set_nuof_writes(unsigned long value) {}
#endif

  virtual void print_info(const char *pre, class cl_console_base *con);
};

class cl_memory_list: public cl_list
{
protected:
  class cl_uc *uc;
public:
  cl_memory_list(class cl_uc *the_uc, const char *name);
  virtual t_index add(class cl_memory *mem);
};


/*
 * Memory chip (storage)
 */

class cl_chip_data: public cl_memory
{
public:
  cl_chip_data(const char *id, t_addr asize, int awidth);
  virtual t_mem d(t_addr addr)= 0;
  virtual void d(t_addr addr, t_mem v)= 0;
};

class cl_memory_chip: public cl_chip_data
{
protected:
  void *array;
  int init_value;
  bool array_is_mine;
  int alloc_size;
  int bwidth; // size of array elements, in bytes
public:
  cl_memory_chip(const char *id, int asize, int awidth, int initial= -1);
  cl_memory_chip(const char *id, int asize, int awidth, void *aarray, int arrsize);
  virtual ~cl_memory_chip(void);
  virtual int init(void);

  virtual bool is_chip(void) { return(true); }

  virtual void *get_slot(t_addr addr);
  virtual bool is_slot(void *data_ptr, t_addr *addr_of);
  
  virtual t_mem read(t_addr addr) { return d(addr); }
  virtual t_mem read(t_addr addr, enum hw_cath skip) { return d(addr); }
  virtual t_mem get(t_addr addr) { return d(addr); }
  virtual t_mem write(t_addr addr, t_mem val) { d(addr, val); return(val); }
  virtual void set(t_addr addr, t_mem val) { d(addr, val); }

  virtual void print_info(const char *pre, class cl_console_base *con);
};

class cl_chip8: public cl_memory_chip
{
public:
  cl_chip8(const char *id, int asize, int awidth, int initial= -1);
  cl_chip8(const char *id, int asize, int awidth, void *aarray, int arrsize);
public:
  virtual t_mem d(t_addr addr);
  virtual void d(t_addr addr, t_mem v);
};

class cl_chip16: public cl_memory_chip
{
public:
  cl_chip16(const char *id, int asize, int awidth, int initial= -1);
  cl_chip16(const char *id, int asize, int awidth, void *aarray, int arrsize);
public:
  virtual t_mem d(t_addr addr);
  virtual void d(t_addr addr, t_mem v);
};

class cl_chip32: public cl_memory_chip
{
public:
  cl_chip32(const char *id, int asize, int awidth, int initial= -1);
  cl_chip32(const char *id, int asize, int awidth, void *aarray, int arrsize);
public:
  virtual t_mem d(t_addr addr);
  virtual void d(t_addr addr, t_mem v);
};

extern class cl_memory_chip *new_chip(const char *id,
				      int asize,
				      int awidth,
				      int initial= -1);


/*
 * Address decoder
 */

class cl_address_decoder: public cl_base
{
public:
  class cl_address_space *address_space;
  class cl_memory_chip *memchip;
  t_addr as_begin, as_end;
  t_addr chip_begin;
  bool activated;
public:
  cl_address_decoder(class cl_memory *as, class cl_memory *chip,
		     t_addr asb, t_addr ase, t_addr cb);
  virtual ~cl_address_decoder(void);
  virtual int init(void);
  virtual bool is_banker() { return false; }
  virtual bool is_bander() { return false; }
  virtual bool uses_chip(class cl_memory *chip) { return chip==memchip; }

  virtual bool activate(class cl_console_base *con);

  t_addr as_to_chip(t_addr addr) { return addr - as_begin + chip_begin; }
  t_addr chip_to_as(t_addr addr) { return addr - chip_begin + as_begin; }

  virtual bool fully_covered_by(t_addr begin, t_addr end);
  virtual bool is_in(t_addr begin, t_addr end);
  virtual bool covers(t_addr begin, t_addr end);

  virtual bool shrink_out_of(t_addr begin, t_addr end);
  virtual class cl_address_decoder *split(t_addr begin, t_addr end);

  virtual void print_info(const char *pre, class cl_console_base *con);
};


/*
 * Address decoder with bank switching support
 */

class cl_banker: public cl_address_decoder
{
 protected:
  class cl_address_space *banker_as, *banker2_as;
  t_addr banker_addr, banker2_addr;
  t_mem banker_mask, banker2_mask;
  //int banker_shift;
  int banker2_shift;
  int nuof_banks;
  int bank;
  class cl_address_decoder **banks;
  int shift_by, shift2_by;
  class cl_memory_operator *op1, *op2;
 public:
  cl_banker(class cl_address_space *the_banker_as,
	    t_addr the_banker_addr,
	    t_mem the_banker_mask,
	    //int the_banker_shift,
	    class cl_address_space *the_as,
	    t_addr the_asb,
	    t_addr the_ase);
  cl_banker(class cl_address_space *the_banker_as,
	    t_addr the_banker_addr,
	    t_mem the_banker_mask,
	    //int the_banker_shift,
	    class cl_address_space *the_banker2_as,
	    t_addr the_banker2_addr,
	    t_mem the_banker2_mask,
	    int the_banker2_shift,
	    class cl_address_space *the_as,
	    t_addr the_asb,
	    t_addr the_ase);
  virtual ~cl_banker();
  virtual int init();
  virtual bool is_banker() { return true; }

  virtual void add_bank(int bank_nr, class cl_memory *chip, t_addr chip_start);

  virtual t_mem actual_bank();
  virtual bool activate(class cl_console_base *con);
  virtual bool switch_to(int bank_nr, class cl_console_base *con);
  virtual bool uses_chip(class cl_memory *chip);
  
  virtual void print_info(const char *pre, class cl_console_base *con);
};


/*
 * Address decoder which maps to individual bits
 */

class cl_bander: public cl_address_decoder
{
 protected:
  int bpc; // bits_per_chip
  int distance; // distance of next chip location
 public:
  cl_bander(class cl_address_space *the_as,
	    t_addr the_asb,
	    t_addr the_ase,
	    class cl_memory *the_chip,
	    t_addr the_cb,
	    int the_bpc,
	    int the_distance);
 public:
  virtual bool is_bander() { return true; }

  virtual bool activate(class cl_console_base *con);
  virtual void print_info(const char *pre, class cl_console_base *con);
};


/* List of address decoders */

class cl_decoder_list: public cl_sorted_list
{
protected:
  bool by_chip;
public:
  cl_decoder_list(t_index alimit, t_index adelta, bool bychip);

  virtual const void *key_of(const void *item) const;
  virtual int compare(const void *key1, const void *key2);
};


/*
 * Messages
 */

#include "eventcl.h"

class cl_event_address_space_added: public cl_event
{
public:
  class cl_address_space *as;
  cl_event_address_space_added(class cl_address_space *the_as):
    cl_event(ev_address_space_added)
  { as= the_as; }
};


/*
 * Errors in memory handling
 */

#include "errorcl.h"

class cl_error_mem: public cl_error
{
protected:
  class cl_memory *mem;
  t_addr addr;
public:
  cl_error_mem(class cl_memory *amem, t_addr aaddr);
};

class cl_error_mem_invalid_address: public cl_error_mem
{
public:
  cl_error_mem_invalid_address(class cl_memory *amem, t_addr aaddr);

  virtual void print(class cl_commander_base *c);
};

 class cl_error_mem_non_decoded: public cl_error_mem
{
public:
  cl_error_mem_non_decoded(class cl_memory *amem, t_addr aaddr);

  virtual void print(class cl_commander_base *c);
};

class cl_mem_error_registry: public cl_error_registry
{
public:
  cl_mem_error_registry(void);
};


#endif

/* End of sim.src/memcl.h */
