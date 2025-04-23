/*
 * Simulator of microcontrollers (mos6502.cc)
 *
 * Copyright (C) 2020 Drotos Daniel
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

#include <stdlib.h>
#include <ctype.h>

#include "appcl.h"
#include "globals.h"
#include "utils.h"

#include "dregcl.h"

#include "glob.h"
#include "irqcl.h"

#include "mos6502cl.h"


static class cl_console_base *c;
static u16_t dbg_a;

static int
con()
{
  if (!jaj) return 0;
  c= application->get_commander()->frozen_or_actual();
  return c!=NULL;
}

int
ccn(class cl_memory_cell *c)
{
  if (!jaj) return 0;
  cl_address_space *rom= application->sim->uc->rom;
  t_addr a;
  if (!rom) return 0;
  if (!rom->is_owned(c, &a)) return 0;
  dbg_a= a;
  return con();
}

static cl_c65 c65_tmpl;

class cl_memory_cell *
cl_as65::cell_template()
{
  return &c65_tmpl;
}

#ifdef DEVEL
t_mem
cl_c65::read(void)
{
  t_mem v= cl_cell8::read();
  if (ccn(this)) c->dd_printf("R[%04x]-> %02x\n", dbg_a, u8_t(v));
  return v;
}
#endif

#ifdef DEVEL
t_mem
cl_c65::get(void)
{
  t_mem v= cl_cell8::get();
  if (ccn(this)) c->dd_printf("R[%04x]-> %02x\n", dbg_a, u8_t(v));
  return v;
}
#endif

#ifdef DEVEL
t_mem
cl_c65::write(t_mem val)
{
  t_mem v= cl_cell8::get();
  if (ccn(this)) c->dd_printf("W[%04x] %02x <- %02x\n", dbg_a, u8_t(v), u8_t(val));
  v= cl_cell8::write(val);
  return v;
}
#endif

#ifdef DEVEL
t_mem
cl_c65::set(t_mem val)
{
  t_mem v= cl_cell8::get();
  if (ccn(this)) c->dd_printf("W[%04x] %02x <- %02x\n", dbg_a, u8_t(v), u8_t(val));
  v= cl_cell8::set(val);
  return v;
}
#endif

cl_mos6502::cl_mos6502(class cl_sim *asim):
  cl_uc(asim)
{
  my_id= new chars();
  *my_id= "MOS6502";
}

int
cl_mos6502::init(void)
{
#define RCV(R) reg_cell_var(&c ## R , &r ## R , "" #R "" , "CPU register " #R "")
  cCC.init();
  cl_uc::init();
  fill_def_wrappers(itab);

  //set_xtal(1000000);
    
  RCV(A);
  RCV(X);
  RCV(Y);
  RCV(SP);
  RCV(P);
#undef RCV
  ci8.decode(&i8d);

  class cl_memory_operator *op= new cl_cc_operator(&cCC);
  cCC.append_operator(op);

  for (int i= 0; i<=0xffff; i++) rom->set(i,0);
  
  return 0;
}


const char *
cl_mos6502::id_string(void)
{
  return my_id->c_str();
}

void
cl_mos6502::reset(void)
{
  cl_uc::reset();

  cF.W(CC= 0x00 | flagI);
  PC= read_addr(rom, RESET_AT);
  rSP= 0xfd;
  
  tick(7);
}

  
void
cl_mos6502::set_PC(t_addr addr)
{
  PC= addr;
}

void
cl_mos6502::mk_hw_elements(void)
{
  class cl_hw *h;
  
  cl_uc::mk_hw_elements();

  add_hw(h= new cl_dreg(this, 0, "dreg"));
  h->init();

  add_hw(h= new cl_irq_hw(this));
  h->init();

  src_irq= new cl_it_src(this,
			 irq_irq,
			 &cCC, flagI,
			 h->cfg_cell(m65_irq), 1,
			 IRQ_AT, false, true,
			 "Interrupt request",
			 0);
  src_irq->set_cid('i');
  src_irq->set_ie_value(0);
  src_irq->init();
  it_sources->add(src_irq);
  
  src_nmi= new cl_it_src(this,
			 irq_nmi,
			 h->cfg_cell(m65_nmi_en), 1,
			 h->cfg_cell(m65_nmi), 1,
			 NMI_AT, false, true,
			 "Non-maskable interrupt request",
			 0);
  src_nmi->set_cid('n');
  src_nmi->set_nmi(true);
  src_nmi->init();
  it_sources->add(src_nmi);
  
  src_brk= new cl_it_src(this,
			 irq_brk,
			 h->cfg_cell(m65_brk_en), 1,
			 h->cfg_cell(m65_brk), 1,
			 IRQ_AT, true, true,
			 "BRK",
			 0);
  src_brk->set_cid('b');
  src_brk->init();
  src_brk->set_nmi(true);
  it_sources->add(src_brk);
}

void
cl_mos6502::make_cpu_hw(void)
{
}

void
cl_mos6502::make_memories(void)
{
  class cl_address_space *as;
  class cl_address_decoder *ad;
  class cl_memory_chip *chip;
  
  rom= as= new cl_as65("rom", 0, 0x10000, 8);
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

struct dis_entry *
cl_mos6502::dis_tbl(void)
{
  return(disass_mos6502);
}

struct dis_entry *
cl_mos6502::get_dis_entry(t_addr addr)
{
  t_mem code= rom->get(addr);

  for (struct dis_entry *de = dis_tbl(); de && de->mnemonic; de++)
    {
      if ((code & de->mask) == de->code)
        return de;
    }

  return NULL;
}

void
cl_mos6502::analyze_start(void)
{
  struct {
    const char *name;
    t_addr vector, addr;
  } vectors[] = {
    { ".reset", RESET_AT },
    { ".nmi",   NMI_AT },
    { ".irq",   IRQ_AT },
  };

  for (size_t i = 0; i < sizeof(vectors) / sizeof(vectors[0]); i++)
    {
      vectors[i].addr = read_addr(rom, vectors[i].vector);
      class cl_var *v = new cl_var(vectors[i].name, rom, vectors[i].addr, chars("Auto-generated by analyze"), -1, -1);
      v->init();
      vars->add(v);
    }

  //for (size_t i = 0; i < sizeof(vectors) / sizeof(vectors[0]); i++)
  analyze(vectors[0].addr);
}

void
cl_mos6502::analyze(t_addr addr)
{
  struct dis_entry *de= 0;

  while (!inst_at(addr) && (de = get_dis_entry(addr)))
    {
      set_inst_at(addr);

      if (de->branch != ' ')
        {
          t_addr target = addr;

          switch (de->branch)
            {
	    case 'x': // Returns or indirect jumps that end this execution path immediately
	      return;
		
	    case 's': // Subroutine calls
	    case 'j': // Unconditional jumps
	      target= rom->read(addr+1) + (rom->read(addr+2) << 8);
	      break;
	      
	    case 'b': // Conditional branches
	      target= addr + 2 + (i8_t)rom->read(addr+1);
	      break;
	    case 'B': // BBR/BBS in 65c02
	      target= addr + 3 + (i8_t)rom->read(addr+2);
	      break;
	      
	    default:
	      break;
            }

          analyze_jump(addr, target, de->branch);

          // Unconditional jumps end this execution path
          if (de->branch == 'j')
            break;
        }

      addr= rom->validate_address(addr + de->length);
    }
}

char *
cl_mos6502::disassc(t_addr addr, chars *comment)
{
  chars work= chars(), temp= chars();
  const char *b;
  struct dis_entry *de;
  int i;
  bool first;
  u8_t h, l;
  u16_t a;

  de= get_dis_entry(addr);

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
      if (b[i] == '%')
	{
	  i++;
	  temp= "";
	  switch (b[i])
	    {
	    case 'x': // (ind,X)
	      l= rom->read(addr+1);
	      work.appendf("($%02x", l);
	      addr_name(l, rom, &work);
	      work.append(",X)");
	      l+= rX;
	      a= read_addr(rom, l);
	      temp.appendf("; [$%04x]=$%02x", a, rom->read(a));
	      break;
	    case 'y': // (ind),Y
	      l= rom->read(addr+1);
	      work.appendf("($%02x", l);
	      addr_name(l, rom, &work);
	      work.append(",Y)");
	      a= read_addr(rom, l) + rY;
	      temp.appendf("; [$%04x]=$%02x", a, rom->read(a));
	      break;
	    case 'a': // abs
	      l= rom->read(addr+1);
	      h= rom->read(addr+2);
	      a= h*256+l;
	      work.appendf("$%04x", a);
	      addr_name(a, rom, &work);
	      temp.appendf("; [$%04x]=$%02x", a, rom->read(a));
	      break;
	    case 'j': // JMP abs
	      l= rom->read(addr+1);
	      h= rom->read(addr+2);
	      a= h*256+l;
	      work.appendf("$%04x", a);
	      addr_name(a, rom, &work);
	      break;
	    case 'J': // JMP (ind)
	      l= rom->read(addr+1);
	      h= rom->read(addr+2);
	      a= h*256+l;
	      work.appendf("($%04x", a);
	      addr_name(a, rom, &work);
	      work.append(")");
	      temp.appendf("; [$%04x]=$%04x", a, read_addr(rom, a));
	      break;
	    case 'z': // zpg
	      l= rom->read(addr+1);
	      work.appendf("$%04x", a= l);
	      addr_name(a, rom, &work);
	      temp.appendf("; [$%04x]=$%02x", a, rom->read(a));
	      break;
	    case 'X': // zpg.X
	      l= rom->read(addr+1);
	      work.appendf("$%04x", l);
	      addr_name(l, rom, &work);
	      work.append(",X");
	      l+= rX;
	      a= l;
	      temp.appendf("; [$%04x]=$%02x", a, rom->read(a));
	      break;
	    case 'Y': // zpg.Y
	      l= rom->read(addr+1);
	      work.appendf("$%04x", l);
	      addr_name(l, rom, &work);
	      work.append(",Y");
	      l+= rY;
	      a= l;
	      temp.appendf("; [$%04x]=$%02x", a, rom->read(a));
	      break;
	    case 'I': // (abs,X)
	      l= rom->read(addr+1);
	      h= rom->read(addr+2);
	      a= h*256+l;
	      work.appendf("($%04x,X)", a);
	      a+= rX;
	      temp.appendf("; [$%04x]=$%04x", a, read_addr(rom, a));
	      break;
	    case 'i': // abs,X
	      l= rom->read(addr+1);
	      h= rom->read(addr+2);
	      a= h*256+l;
	      work.appendf("$%04x", a);
	      addr_name(a, rom, &work);
	      work.append(",X");
	      a+= rX;
	      temp.appendf("; [$%04x]=$%02x", a, rom->read(a));
	      break;
	    case 'p': // abs,Y
	      l= rom->read(addr+1);
	      h= rom->read(addr+2);
	      a= h*256+l;
	      work.appendf("$%04x", a);
	      addr_name(a, rom, &work);
	      work.append(",Y");
	      a+= rY;
	      temp.appendf("; [$%04x]=$%02x", a, rom->read(a));
	      break;
	    case 'r': // rel
	      l= rom->read(addr+1);
	      a= addr + (i8_t)l + 2;
	      work.appendf("$%04x", a);
	      addr_name(a, rom, &work);
	      break;
	    case 'R': // rel in BBR/BBS
	      l= rom->read(addr+2);
	      a= addr + (i8_t)l + 3;
	      work.appendf("$%04x", a);
	      addr_name(a, rom, &work);
	      break;
	    case '#': // imm8
	      l= rom->read(addr+1);
	      work.appendf("#$%02x", l);
	      break;
	    case '3': // (ind16)
	      a= read_addr(rom, addr+1);
	      a= read_addr(rom, a);
	      work.appendf("($%04x)", a);
	      addr_name(a, rom, &work);
	      break;
	    case '4': // (zind)
	      a= rom->read(addr+1);
	      work.appendf("($%04x)", a);
	      a= read_addr(rom, a);
	      addr_name(a, rom, &work);
	      temp.appendf("; [$%04x]=$%02x", a, rom->read(a));
	      break;	      
	    }
	  if (comment && temp.nempty())
	    comment->append(temp);
	}
      else
	work+= b[i];
    }

  return(strdup(work.c_str()));
}

t_addr
cl_mos6502::read_addr(class cl_memory *m, t_addr start_addr)
{
  u8_t h, l;
  l= m->read(start_addr);
  h= m->read(start_addr+1);
  return h*256+l;
}

class cl_cell8 &
cl_mos6502::imm8(void)
{
  //class cl_cell8 *c= (class cl_cell8 *)rom->get_cell(PC);
  i8d= fetch();
  tick(1);
  return ci8;
}

class cl_cell8 &
cl_mos6502::zpg(void)
{
  u8_t a= fetch();
  class cl_cell8 *c= (class cl_cell8 *)rom->get_cell(a);
  vc.rd++;
  tick(2);
  return *c;
}

class cl_cell8 &
cl_mos6502::zpgX(void)
{
  u8_t a= fetch() + rX;
  class cl_cell8 *c= (class cl_cell8 *)rom->get_cell(a);
  vc.rd++;
  tick(3);
  return *c;
}

class cl_cell8 &
cl_mos6502::zpgY(void)
{
  u8_t a= fetch() + rY;
  class cl_cell8 *c= (class cl_cell8 *)rom->get_cell(a);
  vc.rd++;
  tick(3);
  return *c;
}

class cl_cell8 &
cl_mos6502::abs(void)
{
  u16_t a= i16();
  class cl_cell8 *c= (class cl_cell8 *)rom->get_cell(a);
  vc.rd++;
  tick(3);
  return *c;
}

class cl_cell8 &
cl_mos6502::absX(void)
{
  u16_t a1= i16();
  u16_t a2= a1 + rX;
  class cl_cell8 *c= (class cl_cell8 *)rom->get_cell(a2);
  vc.rd++;
  tick(3);
  if ((a1&0xff00) != (a2&0xff00))
    tick(1);
  return *c;
}

class cl_cell8 &
cl_mos6502::absY(void)
{
  u16_t a1= i16();
  u16_t a2= a1 + rY;
  class cl_cell8 *c= (class cl_cell8 *)rom->get_cell(a2);
  vc.rd++;
  tick(3);
  if ((a1&0xff00) != (a2&0xff00))
    tick(1);
  return *c;
}

class cl_cell8 &
cl_mos6502::ind(void)
{
  u16_t a= i16();
  a= read_addr(rom, a);
  class cl_cell8 *c= (class cl_cell8 *)rom->get_cell(a);
  vc.rd+= 3;
  tick(3);
  return *c;
}

class cl_cell8 &
cl_mos6502::zind(void)
{
  u16_t a= fetch();
  a= read_addr(rom, a);
  class cl_cell8 *c= (class cl_cell8 *)rom->get_cell(a);
  vc.rd+= 2;
  tick(4);
  return *c;
}

class cl_cell8 &
cl_mos6502::indX(void)
{
  u8_t a0= fetch() + rX;
  u16_t a= read_addr(rom, a0);
  class cl_cell8 *c= (class cl_cell8 *)rom->get_cell(a);
  vc.rd+= 3;
  tick(5);
  return *c;
}

class cl_cell8 &
cl_mos6502::indY(void)
{
  u16_t a1= read_addr(rom, fetch());
  u16_t a2= a1 + rY;
  class cl_cell8 *c= (class cl_cell8 *)rom->get_cell(a2);
  vc.rd+= 3;
  tick(4);
  if ((a1&0xff00) != (a2&0xff00))
    tick(1);
  return *c;
}

void
cl_mos6502::print_regs(class cl_console_base *con)
{
  int ojaj= jaj;
  jaj= 0;
  con->dd_color("answer");
  con->dd_printf("A= $%02x %3d %+4d %c  ", A, A, (i8_t)A, isprint(A)?A:'.');
  con->dd_printf("X= $%02x %3d %+4d %c  ", X, X, (i8_t)X, isprint(X)?X:'.');
  con->dd_printf("Y= $%02x %3d %+4d %c  ", Y, Y, (i8_t)Y, isprint(Y)?Y:'.');
  con->dd_printf("\n");
  con->dd_printf("P= "); con->print_bin(CC, 8); con->dd_printf("\n");
  con->dd_printf("   NV BDIZC\n");

  con->dd_printf("S= ");
  class cl_dump_ads ads(0x100+SP, 0x100+SP+7);
  rom->dump(0, /*0x100+SP, 0x100+SP+7*/&ads, 8, con);
  con->dd_color("answer");
  
  if (!ojaj)
    print_disass(PC, con);
  else
    {
      con->dd_printf(" ? 0x%04x ", PC);
      {
	int i, j, code= rom->read(PC), l= inst_length(PC);
	struct dis_entry *dt= dis_tbl();
	for (i=0;i<3;i++)
	  if (i<l)
	    con->dd_printf("%02x ",rom->get(PC+i));
	  else
	    con->dd_printf("   ");
	i= 0;
	while (((code & dt[i].mask) != dt[i].code) &&
	       dt[i].mnemonic)
	  i++;
	if (dt[i].mnemonic)
	  {
	    for (j=0;j<3;j++)
	      con->dd_printf("%c", dt[i].mnemonic[j]);
	  }
      }
      con->dd_printf("\n");
    }
  jaj= ojaj;
}

int
cl_mos6502::inst_length(t_addr addr)
{
  struct dis_entry *de= get_dis_entry(addr);
  if (!de)
    return 0;
  return de->length;
}

bool
cl_mos6502::is_call(t_addr addr)
{
  struct dis_entry *de= get_dis_entry(addr);
  if (!de)
    return false;
  return de->branch == 's';
}

int
cl_mos6502::exec_inst(void)
{
  int res;

  set_b= false;
  if ((res= exec_inst_tab(itab)) != resNOT_DONE)
    return res;

  inst_unknown(rom->read(instPC));
  return(resINV_INST);
}

int
cl_mos6502::accept_it(class it_level *il)
{
  class cl_it_src *is= il->source;

  tick(2);
  push_addr(PC);
  rom->write(0x0100 + rSP, rF|0x20);
  if (set_b)
    rF&= ~flagB;
  cSP.W(rSP-1);
  tick(1);
  vc.wr++;
  
  t_addr a= read_addr(rom, is->addr);
  tick(2);
  vc.rd+= 2;
  PC= a;

  rF|= flagI;
  is->clear();
  it_levels->push(il);
  
  return resGO;
}

bool
cl_mos6502::it_enabled(void)
{
  return !(rF & flagI);
}

void
cl_mos6502::push_addr(t_addr a)
{
  rom->write(0x0100 + rSP, (a>>8));
  cSP.W(rSP-1);
  rom->write(0x0100 + rSP, (a));
  cSP.W(rSP-1);
  tick(2);
  vc.wr+= 2;
}

t_addr
cl_mos6502::pop_addr(void)
{
  u8_t h, l;
  cSP.W(rSP+1);
  l= rom->read(0x0100 + rSP);
  cSP.W(rSP+1);
  h= rom->read(0x0100 + rSP);
  tick(2);
  vc.rd+= 2;
  return h*256+l;
}

void
cl_mos6502::stack_check_overflow(class cl_stack_op *op)
{
  if (op)
    {
      if (op->get_op() & stack_write_operation)
	{
	  u8_t a= op->get_before();
	  if (rSP > a)
	    {
	      class cl_error_stack_overflow *e=
		new cl_error_stack_overflow(op);
	      e->init();
	      error(e);
	    }
	}
    }
}


/* End of mos6502.src/mos6502.cc */
