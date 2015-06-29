/*
 * Simulator of microcontrollers (uc51cl.h)
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
 *
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
 *
 */

/*
  This file is part of microcontroller simulator: ucsim.

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
  02111-1307, USA.
*/
/*@1@*/

#ifndef UC51CL_HEADER
#define UC51CL_HEADER

#include <stdio.h>
#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif

#include "pobjcl.h"

#include "simcl.h"
#include "memcl.h"
#include "uccl.h"
#include "itsrccl.h"
#include "brkcl.h"
#include "stypes.h"

#include "interruptcl.h"


class t_uc51;

class cl_irq_stop_option: public cl_optref
{
protected:
  class cl_51core *uc51;
public:
  cl_irq_stop_option(class cl_51core *the_uc51);
  virtual int init(void);
  virtual void option_changed(void);
};

class cl_51core: public cl_uc
{
public:
  // Options
  //bool debug;
  class cl_irq_stop_option *irq_stop_option;
  bool stop_at_it;

  // memories and cells for faster access
  class cl_address_space *sfr, *iram, *xram;
  class cl_memory_cell *acc, *psw;

public:
  // Help to detect external it requests (falling edge)
  uchar prev_p1;        // Prev state of P1
  uchar prev_p3;        // Prev state of P3
  int p3_int0_edge, p3_int1_edge;

public:
  // Simulation of interrupt system
  class cl_interrupt *interrupt;
  //bool  was_reti;     // Instruction had an effect on IE

public:
  int result;           // result of instruction execution

  cl_51core(int Itype, int Itech, class cl_sim *asim);
  virtual ~cl_51core(void);
  virtual int    init(void);
  virtual const char *id_string(void);
  virtual void mk_hw_elements(void);
  virtual void build_cmdset(class cl_cmdset *cmdset);
  //virtual class cl_m *mk_mem(enum mem_class type, char *class_name);
  virtual void make_memories(void);

  virtual int clock_per_cycle(void) { return(12); }
  virtual struct dis_entry *dis_tbl(void);
  virtual struct name_entry *sfr_tbl(void);
  virtual struct name_entry *bit_tbl(void);
  virtual const char *disass(t_addr addr, const char *sep);
  virtual void   print_regs(class cl_console_base *con);
  virtual class cl_address_space *bit2mem(t_addr bitaddr,
                                          t_addr *memaddr, t_mem *bitmask);
  virtual t_addr bit_address(class cl_memory *mem,
                             t_addr mem_address,
                             int bit_number);
  virtual void   reset(void);
  virtual void   clear_sfr(void);
  virtual void   analyze(t_addr addr);
  virtual int    it_priority(uchar ie_mask);

  virtual int    do_inst(int step);

  //virtual void mem_cell_changed(class cl_m *mem, t_addr addr);

protected:
  virtual int  do_interrupt(void);
  virtual int  accept_it(class it_level *il);
protected:
  virtual int  idle_pd(void);

  virtual class cl_memory_cell *get_direct(t_mem addr);
  virtual class cl_memory_cell *get_reg(uchar regnum);

  virtual int   exec_inst(void);
  //virtual void  post_inst(void);

  virtual int inst_unknown(void);
  virtual int inst_nop(uchar code);                     /* 00 */
  virtual int inst_ajmp_addr(uchar code);               /* [02468ace]1 */
  virtual int inst_ljmp(uchar code);                    /* 02 */
  virtual int inst_rr(uchar code);                      /* 03 */
  virtual int inst_inc_a(uchar code);                   /* 04 */
  virtual int inst_inc_addr(uchar code);                /* 05 */
  virtual int inst_inc_Sri(uchar code);                 /* 06,07 */
  virtual int inst_inc_rn(uchar code);                  /* 08-0f */
  virtual int inst_jbc_bit_addr(uchar code);            /* 10 */
  virtual int inst_acall_addr(uchar code);              /* [13579bdf]1 */
  virtual int inst_lcall(uchar code, uint addr, bool intr);/* 12 */
  virtual int inst_rrc(uchar code);                     /* 13 */
  virtual int inst_dec_a(uchar code);                   /* 14 */
  virtual int inst_dec_addr(uchar code);                /* 15 */
  virtual int inst_dec_Sri(uchar code);                 /* 16,17 */
  virtual int inst_dec_rn(uchar code);                  /* 18-1f */
  virtual int inst_jb_bit_addr(uchar code);             /* 20 */
  virtual int inst_ret(uchar code);                     /* 22 */
  virtual int inst_rl(uchar code);                      /* 23 */
  virtual int inst_add_a_Sdata(uchar code);             /* 24 */
  virtual int inst_add_a_addr(uchar code);              /* 25 */
  virtual int inst_add_a_Sri(uchar code);               /* 26,27 */
  virtual int inst_add_a_rn(uchar code);                /* 28-2f */
  virtual int inst_jnb_bit_addr(uchar code);            /* 30 */
  virtual int inst_reti(uchar code);                    /* 32 */
  virtual int inst_rlc(uchar code);                     /* 33 */
  virtual int inst_addc_a_Sdata(uchar code);            /* 34 */
  virtual int inst_addc_a_addr(uchar code);             /* 35 */
  virtual int inst_addc_a_Sri(uchar code);              /* 36,37 */
  virtual int inst_addc_a_rn(uchar code);               /* 38-3f */
  virtual int inst_jc_addr(uchar code);                 /* 40 */
  virtual int inst_orl_addr_a(uchar code);              /* 42 */
  virtual int inst_orl_addr_Sdata(uchar code);          /* 43 */
  virtual int inst_orl_a_Sdata(uchar code);             /* 44 */
  virtual int inst_orl_a_addr(uchar code);              /* 45 */
  virtual int inst_orl_a_Sri(uchar code);               /* 46,47 */
  virtual int inst_orl_a_rn(uchar code);                /* 48-4f */
  virtual int inst_jnc_addr(uchar code);                /* 50 */
  virtual int inst_anl_addr_a(uchar code);              /* 52 */
  virtual int inst_anl_addr_Sdata(uchar code);          /* 53 */
  virtual int inst_anl_a_Sdata(uchar code);             /* 54 */
  virtual int inst_anl_a_addr(uchar code);              /* 55 */
  virtual int inst_anl_a_Sri(uchar code);               /* 56,57 */
  virtual int inst_anl_a_rn(uchar code);                /* 58-5f */
  virtual int inst_jz_addr(uchar code);                 /* 60 */
  virtual int inst_xrl_addr_a(uchar code);              /* 62 */
  virtual int inst_xrl_addr_Sdata(uchar code);          /* 63 */
  virtual int inst_xrl_a_Sdata(uchar code);             /* 64 */
  virtual int inst_xrl_a_addr(uchar code);              /* 65 */
  virtual int inst_xrl_a_Sri(uchar code);               /* 66,67 */
  virtual int inst_xrl_a_rn(uchar code);                /* 68-6f */
  virtual int inst_jnz_addr(uchar code);                /* 70 */
  virtual int inst_orl_c_bit(uchar code);               /* 72 */
  virtual int inst_jmp_Sa_dptr(uchar code);             /* 73 */
  virtual int inst_mov_a_Sdata(uchar code);             /* 74 */
  virtual int inst_mov_addr_Sdata(uchar code);          /* 75 */
  virtual int inst_mov_Sri_Sdata(uchar code);           /* 76,77 */
  virtual int inst_mov_rn_Sdata(uchar code);            /* 78-7f */
  virtual int inst_sjmp(uchar code);                    /* 80 */
  virtual int inst_anl_c_bit(uchar code);               /* 82 */
  virtual int inst_movc_a_Sa_pc(uchar code);            /* 83 */
  virtual int inst_div_ab(uchar code);                  /* 84 */
  virtual int inst_mov_addr_addr(uchar code);           /* 85 */
  virtual int inst_mov_addr_Sri(uchar code);            /* 86,87 */
  virtual int inst_mov_addr_rn(uchar code);             /* 88-8f */
  virtual int inst_mov_dptr_Sdata(uchar code);          /* 90 */
  virtual int inst_mov_bit_c(uchar code);               /* 92 */
  virtual int inst_movc_a_Sa_dptr(uchar code);          /* 93 */
  virtual int inst_subb_a_Sdata(uchar code);            /* 94 */
  virtual int inst_subb_a_addr(uchar code);             /* 95 */
  virtual int inst_subb_a_Sri(uchar code);              /* 96,97 */
  virtual int inst_subb_a_rn(uchar code);               /* 98-9f */
  virtual int inst_orl_c_Sbit(uchar code);              /* a0 */
  virtual int inst_mov_c_bit(uchar code);               /* a2 */
  virtual int inst_inc_dptr(uchar code);                /* a3 */
  virtual int inst_mul_ab(uchar code);                  /* a4 */
  virtual int inst_mov_Sri_addr(uchar code);            /* a6,a7 */
  virtual int inst_mov_rn_addr(uchar code);             /* a8-af */
  virtual int inst_anl_c_Sbit(uchar code);              /* b0 */
  virtual int inst_cpl_bit(uchar code);                 /* b2 */
  virtual int inst_cpl_c(uchar code);                   /* b3 */
  virtual int inst_cjne_a_Sdata_addr(uchar code);       /* b4 */
  virtual int inst_cjne_a_addr_addr(uchar code);        /* b5 */
  virtual int inst_cjne_Sri_Sdata_addr(uchar code);     /* b6,b7 */
  virtual int inst_cjne_rn_Sdata_addr(uchar code);      /* b8-bf */
  virtual int inst_push(uchar code);                    /* c0 */
  virtual int inst_clr_bit(uchar code);                 /* c2 */
  virtual int inst_clr_c(uchar code);                   /* c3*/
  virtual int inst_swap(uchar code);                    /* c4 */
  virtual int inst_xch_a_addr(uchar code);              /* c5 */
  virtual int inst_xch_a_Sri(uchar code);               /* c6,c7 */
  virtual int inst_xch_a_rn(uchar code);                /* c8-cf */
  virtual int inst_pop(uchar code);                     /* d0 */
  virtual int inst_setb_bit(uchar code);                /* d2 */
  virtual int inst_setb_c(uchar code);                  /* d3 */
  virtual int inst_da_a(uchar code);                    /* d4 */
  virtual int inst_djnz_addr_addr(uchar code);          /* d5 */
  virtual int inst_xchd_a_Sri(uchar code);              /* d6,d7 */
  virtual int inst_djnz_rn_addr(uchar code);            /* d8-df */
  virtual int inst_movx_a_Sdptr(uchar code);            /* e0 */
  virtual int inst_movx_a_Sri(uchar code);              /* e2,e3 */
  virtual int inst_clr_a(uchar code);                   /* e4 */
  virtual int inst_mov_a_addr(uchar code);              /* e5 */
  virtual int inst_mov_a_Sri(uchar code);               /* e6,e7 */
  virtual int inst_mov_a_rn(uchar code);                /* e8-ef */
  virtual int inst_movx_Sdptr_a(uchar code);            /* f0 */
  virtual int inst_movx_Sri_a(uchar code);              /* f2,f3 */
  virtual int inst_cpl_a(uchar code);                   /* f4 */
  virtual int inst_mov_addr_a(uchar code);              /* f5 */
  virtual int inst_mov_Sri_a(uchar code);               /* f6,f7 */
  virtual int inst_mov_rn_a(uchar code);                /* f8-ff */
};


class cl_uc51_dummy_hw: public cl_hw
{
protected:
  //class t_uc51 *uc51;
  class cl_memory_cell *cell_acc, *cell_sp, *cell_psw/*, *cell_pcon*/;
public:
  cl_uc51_dummy_hw(class cl_uc *auc);
  virtual int init(void);

  virtual void write(class cl_memory_cell *cell, t_mem *val);
  //virtual void happen(class cl_hw *where, enum hw_event he, void *params);
};


#endif

/* End of s51.src/uc51cl.h */
