/*
 * Simulator of microcontrollers (sim.src/stackcl.h)
 *
 * Copyright (C) 2000,00 Drotos Daniel, Talker Bt.
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

#ifndef SIM_STACKCL_HEADER
#define SIM_STACKCL_HEADER

#include "stypes.h"
#include "pobjcl.h"
#include "errorcl.h"


enum stack_op {
  stack_call   = 0x01,
  stack_intr   = 0x02,
  stack_push   = 0x04,
  stack_ret    = 0x08,
  stack_iret   = 0x10,
  stack_pop    = 0x20
};

const int stack_write_operation= (stack_call|stack_intr|stack_push);
const int stack_read_operation = (stack_ret|stack_iret|stack_pop);

/* Abstraction of a stack operation */
class cl_stack_op: public cl_base
{
protected:
  enum stack_op operation;
  t_addr PC;	// of instruction
  t_addr SP_before;
  t_addr SP_after;
public:
  cl_stack_op(enum stack_op op,
	      t_addr iPC, t_addr iSP_before, t_addr iSP_after);
  virtual ~cl_stack_op(void);
  virtual class cl_stack_op *mk_copy(void);
  static void info_head(class cl_console_base *con);
  virtual void info(class cl_console_base *con, class cl_uc *uc);
protected:
  virtual void print_info(class cl_console_base *con);
public:
  virtual const char *get_op_name(void);
  virtual const char *get_matching_name(void) { return(cchars("unknown")); }
  virtual bool sp_increased(void);
  virtual int data_size(void);
  virtual bool match(class cl_stack_op *op);
  virtual enum stack_op get_op(void) { return(operation); }
  virtual enum stack_op get_matching_op(void) { return(operation); }
  virtual t_addr get_after(void) { return(SP_after); }
  virtual t_addr get_before(void) { return(SP_before); }
  virtual t_addr get_pc(void) { return(PC); }
  virtual bool can_removed(class cl_stack_op *op);
};

/* Call of a subrutine, must match with RET */
class cl_stack_call: public cl_stack_op
{
protected:
  t_addr called_addr; // called routine
  t_addr pushed_addr;
public:
  cl_stack_call(t_addr iPC, t_addr called, t_addr pushed,
		t_addr iSP_before, t_addr iSP_after);
  virtual class cl_stack_op *mk_copy(void);
protected:
  virtual const char *get_op_name(void);
  virtual void print_info(class cl_console_base *con);
public:
  virtual const char *get_matching_name(void);
  virtual enum stack_op get_matching_op(void);
  virtual bool match(class cl_stack_op *op);
};

/* Call of an ISR, must match with IRET */
class cl_stack_intr: public cl_stack_call
{
public:
  cl_stack_intr(t_addr iPC, t_addr called, t_addr pushed,
		t_addr iSP_before, t_addr iSP_after);
  virtual class cl_stack_op *mk_copy(void);
protected:
  virtual const char *get_op_name(void);
  virtual void print_info(class cl_console_base *con);
public:
  virtual const char *get_matching_name(void);
  virtual enum stack_op get_matching_op(void);
  virtual bool match(class cl_stack_op *op);
};

/* Push data to stack, must match with POP */
class cl_stack_push: public cl_stack_op
{
protected:
  t_mem data;  // pushed data
public:
  cl_stack_push(t_addr iPC, t_mem idata, t_addr iSP_before, t_addr iSP_after);
  virtual class cl_stack_op *mk_copy(void);
protected:
  virtual const char *get_op_name(void);
  virtual void print_info(class cl_console_base *con);
public:
  virtual const char *get_matching_name(void);
  virtual enum stack_op get_matching_op(void);
  virtual bool match(class cl_stack_op *op);
};

/* Returning from a subroutine, tos must be CALL */
class cl_stack_ret: public cl_stack_call
{
public:
  cl_stack_ret(t_addr iPC, t_addr iaddr, t_addr iSP_before, t_addr iSP_after);
  virtual class cl_stack_op *mk_copy(void);
protected:
  virtual const char *get_op_name(void);
public:
  virtual const char *get_matching_name(void);
  virtual enum stack_op get_matching_op(void);
  virtual bool match(class cl_stack_op *op);
};

/* Returning from an ISR, yos must be INTR */
class cl_stack_iret: public cl_stack_ret
{
public:
  cl_stack_iret(t_addr iPC, t_addr iaddr, t_addr iSP_before, t_addr iSP_after);
  virtual class cl_stack_op *mk_copy(void);
protected:
  virtual const char *get_op_name(void);
public:
  virtual const char *get_matching_name(void);
  virtual enum stack_op get_matching_op(void);
  virtual bool match(class cl_stack_op *op);
};

/* Pop out data from stack, tos must be PUSH */
class cl_stack_pop: public cl_stack_push
{
public:
  cl_stack_pop(t_addr iPC, t_mem idata, t_addr iSP_before, t_addr iSP_after);
  virtual class cl_stack_op *mk_copy(void);
protected:
  virtual const char *get_op_name(void);
public:
  virtual const char *get_matching_name(void);
  virtual enum stack_op get_matching_op(void);
  virtual bool match(class cl_stack_op *op);
};


/*
 * All kind of stack errors
 */
class cl_error_stack: public cl_error
{
private:
  static class cl_error_class *error_stack_class;
public:
  cl_error_stack(void);
};

/*
 * Stack overflow error
 */
class cl_error_stack_overflow: public cl_error_stack
{
protected:
  t_addr PC, SP_before, SP_after;
public:
  cl_error_stack_overflow(class cl_stack_op *op);
  virtual void print(class cl_commander_base *c);
};
  
/*
 * All kind of stack tracker errors
 */
class cl_error_stack_tracker: public cl_error_stack
{
public:
  cl_error_stack_tracker(void);
};

class cl_error_stack_tracker_wrong_handle: public cl_error_stack_tracker
{
public:
  bool write_operation;
public:
  cl_error_stack_tracker_wrong_handle(bool write_op);
  virtual void print(class cl_commander_base *c);
};

class cl_error_stack_tracker_empty: public cl_error_stack_tracker
{
protected:
  class cl_stack_op *operation;
public:
  cl_error_stack_tracker_empty(class cl_stack_op *op);
  virtual ~cl_error_stack_tracker_empty(void);
  virtual void print(class cl_commander_base *c);
};

class cl_error_stack_tracker_unmatch: public cl_error_stack_tracker
{
protected:
  class cl_stack_op *top, *operation;
public:
  cl_error_stack_tracker_unmatch(class cl_stack_op *Top,
                                class cl_stack_op *op);
  virtual ~cl_error_stack_tracker_unmatch(void);
  virtual void print(class cl_commander_base *c);
};

class cl_error_stack_tracker_inconsistent: public cl_error_stack_tracker
{
protected:
  class cl_stack_op *operation;
  int unread_data_size;
public:
  cl_error_stack_tracker_inconsistent(class cl_stack_op *op,
				      int the_unread_data_size);
  virtual ~cl_error_stack_tracker_inconsistent(void);
  virtual void print(class cl_commander_base *c);
};

class cl_stack_error_registry: public cl_error_registry
{
public:
  cl_stack_error_registry(void);
};


#endif

/* End of sim.src/stackcl.h */
