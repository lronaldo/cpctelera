/*
 * Simulator of microcontrollers (i8080.cc)
 *
 * Copyright (C) 2022 Drotos Daniel
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

#include <ctype.h>

#include "utils.h"

#include "dregcl.h"

#include "i8080cl.h"


/*
 * Flags
 */

t_mem
cl_flag80_op::write(t_mem val)
{
  val&= 0xd7;
  val|= 0x02;
  return val;
}


/*
 * CPU
 */

cl_i8080::cl_i8080(class cl_sim *asim):
  cl_uc(asim)
{
}

int
cl_i8080::init(void)
{
  cl_uc::init();
  fill_def_wrappers(itab);
  //set_xtal(1000000);
  class cl_memory_operator *o= make_flag_op();
  cF.append_operator(o);
  
#define RCV(R) reg_cell_var(&c ## R , &r ## R , "" #R "" , "CPU register " #R "")
  RCV(AF); RCV(A); RCV(F);
  RCV(BC); RCV(B); RCV(C);
  RCV(DE); RCV(D); RCV(E);
  RCV(HL); RCV(H); RCV(L);
  RCV(SP);
#undef RCV
  sp_limit= 0;

  /* According to https://pastraiser.com/cpu/i8080/i8080_opcodes.html
     invalid opcodes execute some existing instructions */
  itab[0x10]= instruction_wrapper_00; // NOP
  itab[0x20]= instruction_wrapper_00; // NOP
  itab[0x30]= instruction_wrapper_00; // NOP
  itab[0x08]= instruction_wrapper_00; // NOP
  itab[0x18]= instruction_wrapper_00; // NOP
  itab[0x28]= instruction_wrapper_00; // NOP
  itab[0x38]= instruction_wrapper_00; // NOP
  itab[0xd9]= instruction_wrapper_c9; // RET
  itab[0xcb]= instruction_wrapper_c3; // JMP
  itab[0xdd]= instruction_wrapper_cd; // CALL
  itab[0xed]= instruction_wrapper_cd; // CALL
  itab[0xfd]= instruction_wrapper_cd; // CALL
  
  cF.W(urnd());
  cA.W(urnd());
  cBC.W(urnd());
  cDE.W(urnd());
  cHL.W(urnd());
  cSP.W(urnd());

  reset();
  return 0;
}

const char *
cl_i8080::id_string(void)
{
  return "i8080";
}

void
cl_i8080::reset(void)
{
  cl_uc::reset();
  PC= 0;
}

void
cl_i8080::set_PC(t_addr addr)
{
  PC= addr;
}

void
cl_i8080::mk_hw_elements(void)
{
  class cl_hw *h;
  cl_uc::mk_hw_elements();

  add_hw(h= new cl_dreg(this, 0, "dreg"));
  h->init();
}

void
cl_i8080::make_cpu_hw(void)
{
  cpu= new cl_i8080_cpu(this);
  add_hw(cpu);
  cpu->init();
}

void
cl_i8080::make_memories(void)
{
  class cl_address_space *as;
  class cl_address_decoder *ad;
  class cl_memory_chip *chip;
  
  rom= as= new cl_address_space("rom", 0, 0x10000, 8);
  as->init();
  address_spaces->add(as);

  chip= new cl_chip8("rom_chip", 0x10000, 8);
  chip->init();
  memchips->add(chip);
  ad= new cl_address_decoder(as= rom,
			     chip, 0, 0xffff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);
}

class cl_memory_operator *
cl_i8080::make_flag_op(void)
{
  class cl_memory_operator *o;
  class cl_cell8 *c8;
  class cl_memory_cell *c;
  c8= &cF;
  c= (class cl_memory_cell *)c8;
  o= new cl_flag80_op(c);
  o->init();
  o->set_name("8080_flag_operator");
  return o;
}

struct dis_entry *
cl_i8080::dis_tbl(void)
{
  return(disass_common);
}

struct dis_entry *
cl_i8080::get_dis_entry(t_addr addr)
{
  t_mem code= rom->get(addr);
  struct dis_entry *de;
  
  for (de = disass_8080; de && de->mnemonic; de++)
    {
      if ((code & de->mask) == de->code)
        return de;
    }
  for (de = disass_common; de && de->mnemonic; de++)
    {
      if ((code & de->mask) == de->code)
        return de;
    }

  return NULL;
}

char rm_names[8]= {
  'B',
  'C',
  'D',
  'E',
  'H',
  'L',
  'M',
  'A'
};

const char *rp_names[4]= {
  "B",
  "D",
  "H",
  "SP"
};
  
const char *srp_names[4]= {
  "B",
  "D",
  "H",
  "PSW"
};
  
void
cl_i8080::dis_M(chars *comment)
{
  u8_t m= rM;
  if (comment->empty())
    *comment= "; ";
  comment->appendf("[0x%04x]= 0x%02x", rHL, m);
}

void
cl_i8080::dis_rp8(chars *comment, int rp)
{
  u16_t a= 0;
  switch (rp)
    {
    case 0: a= rBC; break;
    case 1: a= rDE; break;
    case 2: a= rHL; break;
    case 3: a= rSP; break;
    }
  if (comment->empty())
    *comment= "; ";
  comment->appendf("[0x%04x]= 0x%02x", a, rom->read(a));
}

void
cl_i8080::dis_rp16(chars *comment, int rp)
{
  u16_t a= 0;
  switch (rp)
    {
    case 0: a= rBC; break;
    case 1: a= rDE; break;
    case 2: a= rHL; break;
    case 3: a= rSP; break;
    }
  if (comment->empty())
    *comment= "; ";
  comment->appendf("[0x%04x]= 0x%04x", a, read_addr(rom, a));
}

char *
cl_i8080::disassc(t_addr addr, chars *comment)
{
  chars work= chars(), temp= chars(), fmt= chars();
  const char *b;
  struct dis_entry *de;
  int i;
  bool first;
  u8_t h, l, r, code;
  u16_t a;

  de= get_dis_entry(addr);
  code= rom->read(addr);
  
  if (!de || !de->mnemonic)
    return strdup("-- UNKNOWN/INVALID");

  b= de->mnemonic;

  first= true;
  work= "";
  for (i=0; b[i]; i++)
    {
      if ((b[i] == ' ') && first)
	{
	  first= false;
	  while (work.len() < 6) work.append(' ');
	}
      if (b[i] == '\'')
	{
	  fmt= "";
	  i++;
	  while (b[i] && (b[i]!='\''))
	    fmt.append(b[i++]);
	  if (!b[i]) i--;
	  if (fmt.empty())
	    work.append("'");
	  if (strcmp(fmt.c_str(), "rm5") == 0)
	    {
	      work.appendf("%c", rm_names[r= (code>>3)&7]);
	      if (r==6) dis_M(comment);
	    }
	  if (strcmp(fmt.c_str(), "rm2") == 0)
	    {
	      work.appendf("%c", rm_names[r= (code)&7]);
	      if (r==6) dis_M(comment);
	    }
	  if (strcmp(fmt.c_str(), "i8") == 0)
	    {
	      work.appendf("0x%02x", rom->read(addr+1));
	    }
	  if (strcmp(fmt.c_str(), "hli8") == 0)
	    {
	      l= rom->read(addr+1);
	      work.appendf("0x%02x", l);
	      comment->appendf("; [0x%04x]= 0x%02x", rHL+l, rom->read(rHL+l));
	    }
	  if (strcmp(fmt.c_str(), "spi8") == 0)
	    {
	      l= rom->read(addr+1);
	      work.appendf("0x%02x", l);
	      comment->appendf("; [0x%04x]= 0x%02x", rSP+l, rom->read(rSP+l));
	    }
	  if (strcmp(fmt.c_str(), "i16") == 0)
	    {
	      work.appendf("0x%04x", read_addr(rom, addr+1));
	    }
	  if (strcmp(fmt.c_str(), "rp5") == 0)
	    {
	      work.appendf("%s", rp_names[h= (code>>4)&3]);
	    }
	  if (strcmp(fmt.c_str(), "rp5_8") == 0)
	    {
	      l= (code>>4)&3;
	      work.appendf("%s", rp_names[l]);
	      dis_rp8(comment, l);
	    }
	  if (strcmp(fmt.c_str(), "rp5_16") == 0)
	    {
	      l= (code>>4)&3;
	      work.appendf("%s", rp_names[l]);
	      dis_rp16(comment, l);
	    }
	  if (strcmp(fmt.c_str(), "srp") == 0)
	    {
	      work.appendf("%s", srp_names[h= (code>>4)&3]);
	    }
	  if (strcmp(fmt.c_str(), "a16") == 0)
	    {
	      a= read_addr(rom, addr+1);
	      work.appendf("0x%04x", a);
	    }
	  if (strcmp(fmt.c_str(), "a16_8") == 0)
	    {
	      a= read_addr(rom, addr+1);
	      work.appendf("0x%04x", a);
	      comment->appendf("; [0x%04x]= 0x%02x", a, rom->read(a));
	    }
	  if (strcmp(fmt.c_str(), "a16_16") == 0)
	    {
	      a= read_addr(rom, addr+1);
	      work.appendf("0x%04x", a);
	      comment->appendf("; [0x%04x]= 0x%04x", a, read_addr(rom, a));
	    }
	  if (strcmp(fmt.c_str(), "de16") == 0)
	    {
	      a= rDE;
	      comment->appendf("; [0x%04x]= 0x%04x", a, read_addr(rom, a));
	    }
	  if (strcmp(fmt.c_str(), "rst") == 0)
	    {
	      l= (code>>3)&7;
	      work.appendf("%d", l);
	    }
	  if (strcmp(fmt.c_str(), "in") == 0)
	    {
	      l= rom->read(addr+1);
	      work.appendf("0x%02x", l);
	    }
	  if (strcmp(fmt.c_str(), "out") == 0)
	    {
	      l= rom->read(addr+1);
	      work.appendf("0x%02x", l);
	    }
	  continue;
	}
      if (b[i] == '%')
	{
	  i++;
	  temp= "";
	  switch (b[i])
	    {
	    }
	  if (comment && temp.nempty())
	    comment->append(temp);
	}
      else
	work+= b[i];
    }

  return(strdup(work.c_str()));
}

void
cl_i8080::print_regs(class cl_console_base *con)
{
  con->dd_color("answer");
  con->dd_printf("SZ-A-P-C  Flags= 0x%02x %3d %c  ",
                 rF, rF, isprint(rF)?rF:'.');
  con->dd_printf("A= 0x%02x %3d %c\n",
                 rA, rA, isprint(rA)?rA:'.');
  con->dd_printf("%s\n", cbin(rF, 8).c_str());
  con->dd_printf("BC= 0x%04x [BC]= 0x%02x %3d %c\n",
                 rBC, rom->get(rBC), rom->get(rBC),
                 isprint(rom->get(rBC))?rom->get(rBC):'.');
  con->dd_printf("DE= 0x%04x [DE]= 0x%02x %3d %c\n",
                 rDE, rom->get(rDE), rom->get(rDE),
                 isprint(rom->get(rDE))?rom->get(rDE):'.');
  con->dd_printf("HL= 0x%04x [HL]= 0x%02x %3d %c\n",
                 rHL, rom->get(rHL), rom->get(rHL),
                 isprint(rom->get(rHL))?rom->get(rHL):'.');

  int i;
  con->dd_cprintf("answer", "SP= ");
  con->dd_cprintf("dump_address", "0x%04x ->", rSP);
  for (i= 0; i < 2*12; i+= 2)
    {
      t_addr al, ah;
      al= (rSP+i)&0xffff;
      ah= (al+1)&0xffff;
      con->dd_cprintf("dump_number", " %02x%02x",
		      (u8_t)(rom->read(al)),
		      (u8_t)(rom->read(ah)));
    }
  con->dd_printf("\n");
  
  print_disass(PC, con);
}

class cl_memory_cell &
cl_i8080::cM(void)
{
  vc.wr++;
  class cl_memory_cell *c= rom->get_cell(rHL);
  return *c;
}

u16_t
cl_i8080::fetch16()
{
  u8_t h, l;
  l= fetch();
  h= fetch();
  return h*256+l;
}

int
cl_i8080::exec_inst(void)
{
  int res;
  u16_t *ttab= tick_tab();

  tick_shift= 0;
  if ((res= exec_inst_tab(itab)) != resNOT_DONE)
    {
      u8_t code= rom->read(instPC);
      u16_t t= ttab[code];
      t>>= tick_shift;
      t&= 0xff;
      tick(t-1);
      return res;
    }
  
  inst_unknown(rom->read(instPC));
  return(resINV_INST);
}

void
cl_i8080::push2(u16_t v)
{
  t_addr sp_before= rSP;
  cSP.W(rSP-1);
  rom->write(rSP, v>>8);
  cSP.W(rSP-1);
  rom->write(rSP, v);
  vc.wr+= 2;
  stack_write(sp_before);
}

u16_t
cl_i8080::pop2(void)
{
  u8_t h, l;
  l= rom->read(rSP);
  cSP.W(rSP+1);
  h= rom->read(rSP);
  cSP.W(rSP+1);
  vc.rd+= 2;
  return h*256+l;
}

void
cl_i8080::stack_check_overflow(t_addr sp_before)
{
  if (rSP < sp_limit)
    {
      class cl_error_stack_overflow *e=
	new cl_error_stack_overflow(instPC, sp_before, rSP);
      e->init();
      error(e);
    }
}


cl_i8080_cpu::cl_i8080_cpu(class cl_uc *auc):
  cl_hw(auc, HW_CPU, 0, "cpu")
{
}

int
cl_i8080_cpu::init(void)
{
  cl_hw::init();

  cl_var *v;
  uc->vars->add(v= new cl_var("sp_limit", cfg, i8080cpu_sp_limit,
			      cfg_help(i8080cpu_sp_limit)));
  v->init();

  return 0;
}

const char *
cl_i8080_cpu::cfg_help(t_addr addr)
{
  switch (addr)
    {
    case i8080cpu_sp_limit:
      return "Stack overflows when SP is below this limit";
    }
  return "Not used";
}

t_mem
cl_i8080_cpu::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  class cl_i8080 *u= (class cl_i8080 *)uc;
  if (val)
    cell->set(*val);
  switch ((enum i8080cpu_confs)addr)
    {
    case i8080cpu_sp_limit:
      if (val)
	u->sp_limit= *val & 0xffff;
      else
	cell->set(u->sp_limit);
      break;
    case i8080cpu_nuof: break;
    }
  return cell->get();
}


/* End of i8085.src/i8080.cc */
