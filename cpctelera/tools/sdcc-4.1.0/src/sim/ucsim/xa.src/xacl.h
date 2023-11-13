/*
 * Simulator of microcontrollers (xacl.h)
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
 *
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
 * Other contributors include:
 *   Karl Bongers karl@turbobit.com,
 *   Johan Knol johan.knol@iduna.nl
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

#ifndef XACL_HEADER
#define XACL_HEADER

#include "uccl.h"

#include "regsxa.h"

/*
 * Base type of XA microcontrollers
 */

class cl_xa: public cl_uc
{
public:
  class cl_address_space *ram;
  class cl_address_space *rom;
  struct t_regs regs;

  class cl_address_space *sfr, *iram;

  // for now make it as simple as possible
//  TYPE_UBYTE mem_direct[1024*2];
//#ifndef WORDS_BIGENDIAN
//  TYPE_UWORD *wmem_direct;  /* word pointer at mem_direct */
//#endif

public:
  cl_xa(class cl_sim *asim);
  virtual int init(void);
  virtual const char *id_string(void);

  //virtual class cl_m *mk_mem(enum mem_class type, char *class_name);
  //virtual t_addr get_mem_size(enum mem_class type);
  virtual void mk_hw_elements(void);
  virtual void make_memories(void);

  virtual struct dis_entry *dis_tbl(void);

  virtual struct name_entry *sfr_tbl(void);
  virtual struct name_entry *bit_tbl(void);
  virtual char *get_dir_name(short);
  virtual char *get_bit_name(short);

  virtual int inst_length(t_addr addr);
  virtual int inst_branch(t_addr addr);
  virtual int longest_inst(void);

  virtual int get_disasm_info(t_addr addr,
                       int *ret_len,
                       int *ret_branch,
                       int *immed_offset,
                       int *parms,
                       int *mnemonic);

  virtual char *disass(t_addr addr, const char *sep);
  virtual void print_regs(class cl_console_base *con);

  virtual int exec_inst(void);
  virtual int get_reg(int word_flag, unsigned int index);

  virtual void store1(t_addr addr, unsigned char val);
  virtual void store2(t_addr addr, unsigned short val);
  virtual unsigned char get1(t_addr addr);
  virtual unsigned short get2(t_addr addr);

  virtual bool get_bit(int bit);
  virtual void set_bit(int bit, int value);

#include "instcl.h"

  private :

   /* following are macros which get substituted for FUNC1() and FUNC2()
      in the inst.cc to form the body of ADD,ADDC,SUB,XOR,... */
  /* can I put these in the .cc file and still have them do the inline thing? */
  /*-------------------------------------
    add - flags changed:C,AC,V,N,Z.
  |---------------------------------------*/
  inline unsigned char add1(unsigned char dst, unsigned char src)
  {
    unsigned int result;
    unsigned char flags;
    flags = get_psw();
    flags &= ~BIT_ALL; /* clear these bits */
    result = dst + src;
    if (result == 0) flags |= BIT_Z;
    if (result > 0xff) flags |= BIT_C;
    if (result & 0x80) flags |= BIT_N;
    /* fixme: do AC, V */
    set_psw(flags);
    return (unsigned char) result;
  }

  inline unsigned short add2(unsigned short dst, unsigned short src)
  {
    unsigned int result;
    unsigned char flags;
    flags = get_psw();
    flags &= ~BIT_ALL; /* clear these bits */
    result = dst + src;
    if (result == 0) flags |= BIT_Z;
    if (result > 0xff) flags |= BIT_C;
    if (result & 0x80) flags |= BIT_N;
    /* fixme: do AC, V */
    set_psw(flags);
    return (unsigned short) result;
  }

  /*-------------------------------------
    addc - flags changed:C,AC,V,N,Z.
  |---------------------------------------*/
  inline unsigned char addc1(unsigned char dst, unsigned char src)
  {
    unsigned int result;
    unsigned char flags;
    flags = get_psw();
    if (flags & BIT_C) {
      flags &= ~BIT_ALL; /* clear these bits */
      result = dst + src + 1;
    } else {
      flags &= ~BIT_ALL; /* clear these bits */
      result = dst + src;
    }
    if (result == 0) flags |= BIT_Z;
    if (result > 0xff) flags |= BIT_C;
    if (result & 0x80) flags |= BIT_N;
    /* fixme: do AC, V */
    set_psw(flags);
    return (unsigned char) result;
  }

  inline unsigned short addc2(unsigned short dst, unsigned short src)
  {
    unsigned int result;
    unsigned char flags;
    flags = get_psw();
    flags &= ~BIT_ALL; /* clear these bits */
    if (flags & BIT_C) {
      flags &= ~BIT_ALL; /* clear these bits */
      result = dst + src + 1;
    } else {
      flags &= ~BIT_ALL; /* clear these bits */
      result = dst + src;
    }
    if (result == 0) flags |= BIT_Z;
    if (result > 0xff) flags |= BIT_C;
    if (result & 0x80) flags |= BIT_N;
    /* fixme: do AC, V */
    set_psw(flags);
    return (unsigned short) result;
  }

  /*-------------------------------------
    sub - flags changed:C,AC,V,N,Z.
  |---------------------------------------*/
  inline unsigned char sub1(unsigned char dst, unsigned char src)
  {
    unsigned int result;
    unsigned char flags;
    flags = get_psw();
    flags &= ~BIT_ALL; /* clear these bits */
    result = dst - src;
    if (result == 0) flags |= BIT_Z;
    if (result > 0xff) flags |= BIT_C;
    if (dst < src) flags |= BIT_N;
    /* fixme: do AC, V */
    set_psw(flags);
    return (unsigned char) result;
  }

  inline unsigned short sub2(unsigned short dst, unsigned short src)
  {
    unsigned int result;
    unsigned char flags;
    flags = get_psw();
    flags &= ~BIT_ALL; /* clear these bits */
    result = dst - src;
    if (result == 0) flags |= BIT_Z;
    if (result > 0xff) flags |= BIT_C;
    if (dst < src) flags |= BIT_N;
    /* fixme: do AC, V */
    set_psw(flags);
    return (unsigned short) result;
  }

  /*-------------------------------------
    subb - flags changed:C,AC,V,N,Z.
  |---------------------------------------*/
  inline unsigned char subb1(unsigned char dst, unsigned char src)
  {
    unsigned int result;
    unsigned char flags;
    flags = get_psw();
    if (flags & BIT_C) {
      flags &= ~BIT_ALL; /* clear these bits */
      result = dst - src - 1;
    } else {
      flags &= ~BIT_ALL; /* clear these bits */
      result = dst - src;
    }
    if (result == 0) flags |= BIT_Z;
    if (result > 0xff) flags |= BIT_C;
    if (dst < src) flags |= BIT_N;
    /* fixme: do AC, V */
    set_psw(flags);
    return (unsigned char) result;
  }

  inline unsigned short subb2(unsigned short dst, unsigned short src)
  {
    unsigned int result;
    unsigned char flags;
    flags = get_psw();
    flags &= ~BIT_ALL; /* clear these bits */
    if (flags & BIT_C) {
      flags &= ~BIT_ALL; /* clear these bits */
      result = dst - src - 1;
    } else {
      flags &= ~BIT_ALL; /* clear these bits */
      result = dst - src;
    }
    if (result == 0) flags |= BIT_Z;
    if (result > 0xff) flags |= BIT_C;
    if (dst < src) flags |= BIT_N;
    /* fixme: do AC, V */
    set_psw(flags);
    return (unsigned short) result;
  }

  /*-------------------------------------
    cmp - flags changed:C,AC,V,N,Z.
  |---------------------------------------*/
  inline unsigned char cmp1(unsigned char dst, unsigned char src)
  {
    unsigned int result;
    unsigned char flags;
    flags = get_psw();
    flags &= ~BIT_ALL; /* clear these bits */
    result = dst - src;
    if (result == 0) flags |= BIT_Z;
    if (result > 0xff) flags |= BIT_C;
    if (dst < src) flags |= BIT_N;
    /* fixme: do AC, V */
    set_psw(flags);
    return (unsigned char) dst;
  }

  inline unsigned short cmp2(unsigned short dst, unsigned short src)
  {
    unsigned int result;
    unsigned char flags;
    flags = get_psw();
    flags &= ~BIT_ALL; /* clear these bits */
    result = dst - src;
    if (result == 0) flags |= BIT_Z;
    if (result > 0xff) flags |= BIT_C;
    if (dst < src) flags |= BIT_N;
    /* fixme: do AC, V */
    set_psw(flags);
    return (unsigned short) dst;
  }

  /*-------------------------------------
    and - flags changed:N,Z.
  |---------------------------------------*/
  inline unsigned char and1(unsigned char dst, unsigned char src)
  {
    unsigned int result;
    unsigned char flags;
    flags = get_psw() & ~(BIT_N | BIT_Z); /* clear these bits */
    result = dst & src;
    if (result == 0) flags |= BIT_Z;
    if (result & 0x80) flags |= BIT_N;
    set_psw(flags);
    return (unsigned char) result;
  }

  inline unsigned short and2(unsigned short dst, unsigned short src)
  {
    unsigned int result;
    unsigned char flags;
    flags = get_psw() & ~(BIT_N | BIT_Z); /* clear these bits */
    result = dst & src;
    if (result == 0) flags |= BIT_Z;
    if (result & 0x80) flags |= BIT_N;
    set_psw(flags);
    return (unsigned short) result;
  }

  /*-------------------------------------
    or - flags changed:N,Z.
  |---------------------------------------*/
  inline unsigned char or1(unsigned char dst, unsigned char src)
  {
    unsigned int result;
    unsigned char flags;
    flags = get_psw() & ~(BIT_N | BIT_Z); /* clear these bits */
    result = dst | src;
    if (result == 0) flags |= BIT_Z;
    if (result & 0x80) flags |= BIT_N;
    set_psw(flags);
    return (unsigned char) result;
  }

  inline unsigned short or2(unsigned short dst, unsigned short src)
  {
    unsigned int result;
    unsigned char flags;
    flags = get_psw() & ~(BIT_N | BIT_Z); /* clear these bits */
    result = dst | src;
    if (result == 0) flags |= BIT_Z;
    if (result & 0x80) flags |= BIT_N;
    set_psw(flags);
    return (unsigned short) result;
  }

  /*-------------------------------------
    xor - flags changed:N,Z.
  |---------------------------------------*/
  inline unsigned char xor1(unsigned char dst, unsigned char src)
  {
    unsigned char flags;
    flags = get_psw() & ~(BIT_N | BIT_Z); /* clear these bits */
    dst ^= src;
    if (dst == 0) flags |= BIT_Z;
    if (dst & 0x80) flags |= BIT_N;
    set_psw(flags);
    return (unsigned char) dst;
  }

  inline unsigned short xor2(unsigned short dst, unsigned short src)
  {
    unsigned char flags;
    flags = get_psw() & ~(BIT_N | BIT_Z); /* clear these bits */
    dst ^= src;
    if (dst == 0) flags |= BIT_Z;
    if (dst & 0x8000) flags |= BIT_N;
    set_psw(flags);
    return (unsigned short) dst;
  }

  /*-------------------------------------
    mov - flags changed:N,Z.
  |---------------------------------------*/
  inline unsigned char mov1(unsigned char dst, unsigned char src)
  {
    unsigned char flags;
    flags = get_psw() & ~(BIT_N | BIT_Z); /* clear these bits */
    dst = src;
    if (dst == 0) flags |= BIT_Z;
    if (dst & 0x80) flags |= BIT_N;
    set_psw(flags);
    return (unsigned char) dst;
  }

  inline unsigned short mov2(unsigned short dst, unsigned short src)
  {
    unsigned char flags;
    flags = get_psw() & ~(BIT_N | BIT_Z); /* clear these bits */
    dst = src;
    if (dst == 0) flags |= BIT_Z;
    if (dst & 0x8000) flags |= BIT_N;
    set_psw(flags);
    return (unsigned short) dst;
  }
};

#endif

/* End of xa.src/xacl.h */
