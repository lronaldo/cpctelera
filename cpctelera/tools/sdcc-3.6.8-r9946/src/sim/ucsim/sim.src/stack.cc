/*
 * Simulator of microcontrollers (stack.cc)
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

#include <stdlib.h>

// cmd.src
#include "newcmdcl.h"

// sim.src
#include "uccl.h"

#include "stackcl.h"


static class cl_stack_error_registry stack_error_registry;

cl_stack_op::cl_stack_op(enum stack_op op,
			 t_addr iPC,
			 t_addr iSP_before, t_addr iSP_after):
  cl_base()
{
  operation= op;
  PC= iPC;
  //addr= iaddr;
  //data= idata;
  SP_before= iSP_before;
  SP_after= iSP_after;
}

cl_stack_op::~cl_stack_op(void)
{
  //printf("stack op %p deleting...\n", this);
}


class cl_stack_op *
cl_stack_op::mk_copy(void)
{
  class cl_stack_op *so= new cl_stack_op(*this);
  return(so);
}

void
cl_stack_op::info_head(class cl_console_base *con)
{
  con->dd_printf("OP   SP before-after   L DATA/ADDR   INSTRUCTION\n");
}

void
cl_stack_op::info(class cl_console_base *con, class cl_uc *uc)
{
  con->dd_printf("%-4s 0x%06x-0x%06x %d ",
		 get_op_name(),
		 (int)SP_before, (int)SP_after, labs(SP_before-SP_after));
  print_info(con);
  con->dd_printf(" ");
  uc->print_disass(PC, con);
  //con->dd_printf("\n");
}

const char *
cl_stack_op::get_op_name(void)
{
  return("op");
}

void
cl_stack_op::print_info(class cl_console_base *con)
{
  con->dd_printf("-");
}

bool
cl_stack_op::sp_increased(void)
{
  if (operation & stack_write_operation)
    return(SP_after > SP_before);
  else // read operation
    return(SP_after < SP_before);
}

int
cl_stack_op::data_size(void)
{
  int r= SP_after - SP_before;

  return(r<0?-r:r);
}

bool
cl_stack_op::match(class cl_stack_op *op)
{
  return(false);
}

bool bigbig= false;

bool
cl_stack_op::can_removed(class cl_stack_op *op)
{
  return false;
  bool incr= sp_increased(); // FIXME
  bool op_incr= op->sp_increased(); // FIXME

  if ((incr && !op_incr) ||
      (!incr && op_incr))
    {
      printf("BIGBIG ERROR!\n");
      bigbig= true;
      return(false);
    }
  if (incr)
    {
      t_mem opa= op->get_after();
      return(SP_before >= opa);
    }
  else
    {
      t_mem opa= op->get_after();
      return(SP_before <= opa);
    }
}


/*
 * CALL operation on stack
 */

cl_stack_call::cl_stack_call(t_addr iPC, t_addr called, t_addr pushed,
                            t_addr iSP_before, t_addr iSP_after):
  cl_stack_op(stack_call, iPC, iSP_before, iSP_after)
{
  called_addr= called;
  pushed_addr= pushed;
}

class cl_stack_op *
cl_stack_call::mk_copy(void)
{
  class cl_stack_call *so= new cl_stack_call(*this);
  return(so);
}

const char *
cl_stack_call::get_op_name(void)
{
  return("call");
}

void
cl_stack_call::print_info(class cl_console_base *con)
{
  con->dd_printf("0x%06x", (int)called_addr);
}

const char *
cl_stack_call::get_matching_name(void)
{
  return("ret");
}

enum stack_op
cl_stack_call::get_matching_op(void)
{
  return(stack_ret);
}

bool
cl_stack_call::match(class cl_stack_op *op)
{
  return(op->get_op() == stack_ret);
}


/*
 * INTERRUPT operation (call) on stack
 */

cl_stack_intr::cl_stack_intr(t_addr iPC, t_addr called, t_addr pushed,
                            t_addr iSP_before, t_addr iSP_after):
  cl_stack_call(iPC, called, pushed, iSP_before, iSP_after)
{
  //called_addr= called;
  //pushed_addr= pushed;
  operation= stack_intr;
}

class cl_stack_op *
cl_stack_intr::mk_copy(void)
{
  class cl_stack_intr *so= new cl_stack_intr(*this);
  return(so);
}

const char *
cl_stack_intr::get_op_name(void)
{
  return("intr");
}

void
cl_stack_intr::print_info(class cl_console_base *con)
{
  con->dd_printf("0x%06x", (int)called_addr);
}

const char *
cl_stack_intr::get_matching_name(void)
{
  return("iret");
}

enum stack_op
cl_stack_intr::get_matching_op(void)
{
  return(stack_iret);
}

bool
cl_stack_intr::match(class cl_stack_op *op)
{
  return(op->get_op() == stack_iret);
}


/*
 * PUSH operation on stack
 */

cl_stack_push::cl_stack_push(t_addr iPC, t_mem idata,
                            t_addr iSP_before, t_addr iSP_after):
  cl_stack_op(stack_push, iPC, iSP_before, iSP_after)
{
  data= idata;
}

class cl_stack_op *
cl_stack_push::mk_copy(void)
{
  class cl_stack_push *so= new cl_stack_push(*this);
  return(so);
}

const char *
cl_stack_push::get_op_name(void)
{
  return("push");
}

const char *
cl_stack_push::get_matching_name(void)
{
  return("pop");
}

enum stack_op
cl_stack_push::get_matching_op(void)
{
  return(stack_pop);
}

void
cl_stack_push::print_info(class cl_console_base *con)
{
  t_addr d= data;
  con->dd_printf("0x%06x", (int)d);
}

bool
cl_stack_push::match(class cl_stack_op *op)
{
  return(op->get_op() == stack_pop);
}


/*
 * RETURN operation on stack
 */

cl_stack_ret::cl_stack_ret(t_addr iPC, t_addr iaddr,
                          t_addr iSP_before, t_addr iSP_after):
  cl_stack_call(iPC, iaddr, 0, iSP_before, iSP_after)
{
  operation= stack_ret;
}

class cl_stack_op *
cl_stack_ret::mk_copy(void)
{
  class cl_stack_ret *so= new cl_stack_ret(*this);
  return(so);
}

const char *
cl_stack_ret::get_op_name(void)
{
  return("ret");
}

const char *
cl_stack_ret::get_matching_name(void)
{
  return("call");
}

enum stack_op
cl_stack_ret::get_matching_op(void)
{
  return(stack_call);
}

bool
cl_stack_ret::match(class cl_stack_op *op)
{
  return(op->get_op() == stack_call);
}


/*
 * RETURN from interrupt operation on stack
 */

cl_stack_iret::cl_stack_iret(t_addr iPC, t_addr iaddr,
                            t_addr iSP_before, t_addr iSP_after):
  cl_stack_ret(iPC, iaddr, iSP_before, iSP_after)
{
  operation= stack_iret;
}

class cl_stack_op *
cl_stack_iret::mk_copy(void)
{
  class cl_stack_iret *so= new cl_stack_iret(*this);
  return(so);
}

const char *
cl_stack_iret::get_op_name(void)
{
  return("iret");
}

const char *
cl_stack_iret::get_matching_name(void)
{
  return("intr");
}

enum stack_op
cl_stack_iret::get_matching_op(void)
{
  return(stack_intr);
}

bool
cl_stack_iret::match(class cl_stack_op *op)
{
  return(op->get_op() == stack_intr);
}


/*
 * POP operation on stack
 */

cl_stack_pop::cl_stack_pop(t_addr iPC, t_mem idata,
                          t_addr iSP_before, t_addr iSP_after):
  cl_stack_push(iPC, idata, iSP_before, iSP_after)
{
  operation= stack_pop;
}

class cl_stack_op *
cl_stack_pop::mk_copy(void)
{
  class cl_stack_pop *so= new cl_stack_pop(*this);
  return(so);
}

const char *
cl_stack_pop::get_op_name(void)
{
  return("pop");
}

const char *
cl_stack_pop::get_matching_name(void)
{
  return("push");
}

enum stack_op
cl_stack_pop::get_matching_op(void)
{
  return(stack_push);
}

bool
cl_stack_pop::match(class cl_stack_op *op)
{
  return(op->get_op() == stack_push);
}


/*
 * Stack Errors
 */

cl_error_stack::cl_error_stack(void)
{
classification = stack_error_registry.find("stack");
}

/* Stack Tracker Errors */

cl_error_stack_tracker::cl_error_stack_tracker(void)
{
classification = stack_error_registry.find("stack_tracker");
}

/* Stack Tracker: wrong handle */

cl_error_stack_tracker_wrong_handle::cl_error_stack_tracker_wrong_handle(bool write_op):
  cl_error_stack_tracker()
{
  write_operation= write_op;
  classification= stack_error_registry.find("stack_tracker_wrong_handle");
}

void
cl_error_stack_tracker_wrong_handle::print(class cl_commander_base *c)
{
  c->dd_printf("%s: wrong stack tracker handle called for %s operation\n",
	       get_type_name(), write_operation?"write":"read");
}

/* Stack Tracker: operation on empty stack */

cl_error_stack_tracker_empty::
cl_error_stack_tracker_empty(class cl_stack_op *op):
  cl_error_stack_tracker()
{
  operation= op->mk_copy();
  classification= stack_error_registry.find("operation_on_empty_stack");
}

cl_error_stack_tracker_empty::~cl_error_stack_tracker_empty(void)
{
  delete operation;
}

void
cl_error_stack_tracker_empty::print(class cl_commander_base *c)
{
  c->dd_printf("%s(0x%06x): %s on empty stack, PC="
	       "0x06x, SP=0x%06x->0x%06x\n",
	       get_type_name(), (int)operation->get_pc(),
	       operation->get_op_name(),
	       (int)operation->get_pc(),
	       (int)operation->get_before(),
	       (int)operation->get_after());
}

/* Stack Tracker: operation on empty stack */

cl_error_stack_tracker_unmatch::
cl_error_stack_tracker_unmatch(class cl_stack_op *Top, class cl_stack_op *op):
  cl_error_stack_tracker()
{
  top= Top->mk_copy();
  operation= op->mk_copy();
  //printf("top=%p op=%p\n", top, operation);
  classification=
    stack_error_registry.find("stack_operation_unmatched_to_top_of_stack");
}

cl_error_stack_tracker_unmatch::~cl_error_stack_tracker_unmatch(void)
{
  //printf("trying delete stackop %p op\n", operation);
  //printf("trying delete stackop %p top\n", top);
  if (bigbig)
    {
      delete operation;
      delete top;
    }
  else
    {
      delete operation;
      delete top;
    }
}

void
cl_error_stack_tracker_unmatch::print(class cl_commander_base *c)
{
  c->dd_printf("%s(0x%06x): %s when %s expected, "
	       "SP=0x%06x->0x%06x\n",
	       get_type_name(), (int)operation->get_pc(),
	       operation->get_op_name(), top->get_matching_name(),
	       (int)operation->get_before(),
	       (int)operation->get_after());
}

/* Stack Tracker: stack is inconsistent */

cl_error_stack_tracker_inconsistent::
cl_error_stack_tracker_inconsistent(class cl_stack_op *op,
                                   int the_unread_data_size)
{
  operation= op->mk_copy();
  unread_data_size= the_unread_data_size;
  classification= stack_error_registry.find("stack_looks_corrupted");
}

cl_error_stack_tracker_inconsistent::~cl_error_stack_tracker_inconsistent(void)
{
  delete operation;
}

void
cl_error_stack_tracker_inconsistent::print(class cl_commander_base *c)
{
  c->dd_printf("%s(0x%06x): %d byte(s) unread from the stack\n",
	       get_type_name(), (int)operation->get_pc(),
	       unread_data_size);
}

cl_stack_error_registry::cl_stack_error_registry(void)
{
  class cl_error_class *prev = stack_error_registry.find("non-classified");
  prev = register_error(new cl_error_class(err_error, "stack", prev, ERROR_OFF));
  prev = register_error(new cl_error_class(err_error, "stack_tracker", prev));
  prev = register_error(new cl_error_class(err_error, "stack_tracker_wrong_handle", prev));
  prev = register_error(new cl_error_class(err_error, "operation_on_empty_stack", prev));
  prev = register_error(new cl_error_class(err_warning, "stack_operation_unmatched_to_top_of_stack", prev));
  prev = register_error(new cl_error_class(err_warning, "stack_looks_corrupted", prev));
}


/* End of sim.src/stack.cc */
