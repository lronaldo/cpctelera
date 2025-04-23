/*
 * Simulator of microcontrollers (m6809.cc)
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

#include "globals.h"
#include "utils.h"

#include "dregcl.h"
#include "ciacl.h"
#include "piacl.h"

#include "glob.h"
#include "irqcl.h"

#include "m6809cl.h"


cl_m6809::cl_m6809(class cl_sim *asim):
  cl_uc(asim)
{
}

int
cl_m6809::init(void)
{
  cl_uc::init();

  //set_xtal(1000000);
  
  reg8_ptr[0]= &A;
  reg8_ptr[1]= &B;
  reg8_ptr[2]= &(reg.CC);
  reg8_ptr[3]= &(reg.DP);
  reg8_ptr[4]= &A;
  reg8_ptr[5]= &A;
  reg8_ptr[6]= &A;
  reg8_ptr[7]= &A;

  reg16_ptr[0]= &D;
  reg16_ptr[1]= &(reg.X);
  reg16_ptr[2]= &(reg.Y);
  reg16_ptr[3]= &(reg.U);
  reg16_ptr[4]= &(reg.S);
  reg16_ptr[5]= NULL;
  reg16_ptr[6]= &D;
  reg16_ptr[7]= &D;
  
  return 0;
}

const char *
cl_m6809::id_string(void)
{
  return "M6809";
}

void
cl_m6809::reset(void)
{
  cl_uc::reset();

  reg.DP= 0;
  en_nmi= false;
  cwai= false;
  reg.CC= flagI | flagF;
  PC= rom->read(0xfffe)*256 + rom->read(0xffff);
  tick(6);
}
  
void
cl_m6809::set_PC(t_addr addr)
{
  PC= addr;
}

void
cl_m6809::mk_hw_elements(void)
{
  class cl_hw *h;
  class cl_option *o;
  cl_uc::mk_hw_elements();

  if ((o= application->options->get_option("serial0_in_file")) == NULL)
    {
      o= new cl_string_option(this, "serial0_in_file",
			      "Input file for serial line uart0 (-S)");
      application->options->new_option(o);
      o->init();
      o->hide();
    }
  if ((o= application->options->get_option("serial0_out_file")) == NULL)
    {
      o= new cl_string_option(this, "serial0_out_file",
			      "Output file for serial line uart0 (-S)");
      application->options->new_option(o);
      o->init();
      o->hide();
    }


  if ((o= application->options->get_option("serial1_in_file")) == NULL)
    {
      o= new cl_string_option(this, "serial1_in_file",
			      "Input file for serial line uart1 (-S)");
      application->options->new_option(o);
      o->init();
      o->hide();
    }
  if ((o= application->options->get_option("serial1_out_file")) == NULL)
    {
      o= new cl_string_option(this, "serial1_out_file",
			      "Output file for serial line uart1 (-S)");
      application->options->new_option(o);
      o->init();
      o->hide();
    }

  add_hw(h= new cl_dreg(this, 0, "dreg"));
  h->init();
  
  add_hw(h= new cl_m6809_irq(this));
  h->init();

  src_irq= new cl_it_src(this,
			 irq_irq,
			 regs8->get_cell(3), flagI,
			 h->cfg_cell(cpu_irq), 1,
			 IRQ_AT,
			 false,
			 true,
			 "Interrupt request",
			 0);
  src_irq->set_cid('i');
  src_irq->set_ie_value(0);
  src_irq->init();
  it_sources->add(src_irq);
  
  src_firq= new cl_it_src(this,
			  irq_firq,
			  regs8->get_cell(3), flagF,
			  h->cfg_cell(cpu_firq), 1,
			  FIRQ_AT,
			  false,
			  true,
			  "Fast interrupt request",
			  0);
  src_firq->set_cid('f');
  src_firq->init();
  it_sources->add(src_firq);
  
  src_nmi= new cl_it_src(this,
			 irq_nmi,
			 h->cfg_cell(cpu_nmi_en), 1,
			 h->cfg_cell(cpu_nmi), 1,
			 NMI_AT,
			 false,
			 true,
			 "Non-maskable interrupt request",
			 0);
  src_nmi->set_cid('n');
  src_nmi->set_nmi(true);
  src_nmi->init();
  it_sources->add(src_nmi);
  
  add_hw(h= new cl_cia(this, 0, 0xc000));
  h->init();

  add_hw(h= new cl_cia(this, 1, 0xc008));
  h->init();

  class cl_pia *p0, *p1;
  
  add_hw(p0= new cl_pia(this, 0, 0xc010));
  p0->init();
  add_hw(p1= new cl_pia(this, 1, 0xc020));
  p1->init();

  class cl_port_ui *d;
  add_hw(d= new cl_port_ui(this, 0, "dport"));
  d->init();

  class cl_port_data pd;
  pd.init();
  pd.set_name("P0A");
  pd.cell_dir= p0->ddra;
  pd.cell_p  = p0->ora;
  pd.cell_in = p0->ina;
  pd.keyset  = keysets[0];
  pd.basx    = 1;
  pd.basy    = 5;
  d->add_port(&pd, 0);

  pd.set_name("P0B");
  pd.cell_dir= p0->ddrb;
  pd.cell_p  = p0->orb;
  pd.cell_in = p0->inb;
  pd.keyset  = keysets[1];
  pd.basx    = 20;
  pd.basy    = 5;
  d->add_port(&pd, 1);

  pd.set_name("P0CA");
  pd.cell_dir= p0->ddca;
  pd.cell_p  = p0->oca;
  pd.cell_in = p0->inca;
  pd.cell_dir= p0->ddca;
  pd.keyset  = keysets[2];
  pd.basx    = 40;
  pd.basy    = 5;
  pd.width   = 2;
  d->add_port(&pd, 2);

  pd.set_name("P0CB");
  pd.cell_dir= p0->ddcb;
  pd.cell_p  = p0->ocb;
  pd.cell_in = p0->incb;
  pd.cell_dir= p0->ddcb;
  pd.keyset  = keysets[3];
  pd.basx    = 54;
  pd.basy    = 5;
  pd.width   = 2;
  d->add_port(&pd, 3);

  // Port #1
  pd.init();
  pd.set_name("P1A");
  pd.cell_dir= p1->ddra;
  pd.cell_p  = p1->ora;
  pd.cell_in = p1->ina;
  pd.keyset  = keysets[4];
  pd.basx    = 1;
  pd.basy    = 11;
  d->add_port(&pd, 4);

  pd.set_name("P1B");
  pd.cell_dir= p1->ddrb;
  pd.cell_p  = p1->orb;
  pd.cell_in = p1->inb;
  pd.keyset  = keysets[5];
  pd.basx    = 20;
  pd.basy    = 11;
  d->add_port(&pd, 5);

  pd.set_name("P1CA");
  pd.cell_dir= p1->ddca;
  pd.cell_p  = p1->oca;
  pd.cell_in = p1->inca;
  pd.cell_dir= p1->ddca;
  pd.keyset  = keysets[6];
  pd.basx    = 40;
  pd.basy    = 11;
  pd.width   = 2;
  d->add_port(&pd, 6);

  pd.set_name("P1CB");
  pd.cell_dir= p1->ddcb;
  pd.cell_p  = p1->ocb;
  pd.cell_in = p1->incb;
  pd.cell_dir= p1->ddcb;
  pd.keyset  = keysets[7];
  pd.basx    = 54;
  pd.basy    = 11;
  pd.width   = 2;
  d->add_port(&pd, 7);
}

void
cl_m6809::make_cpu_hw(void)
{
}

void
cl_m6809::make_memories(void)
{
  class cl_address_space *as;
  
  rom= as= new cl_address_space("rom"/*MEM_ROM_ID*/, 0, 0x10000, 8);
  as->init();
  address_spaces->add(as);

  class cl_address_decoder *ad;
  class cl_memory_chip *chip;

  chip= new cl_chip8("rom_chip", 0x10000, 8);
  chip->init();
  memchips->add(chip);
  ad= new cl_address_decoder(as= rom/*address_space(MEM_ROM_ID)*/,
			     chip, 0, 0xffff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

  regs8= new cl_address_space("regs8", 0, 4, 8);
  regs8->init();
  address_spaces->add(regs8);
  regs8->get_cell(0)->decode((t_mem*)&(reg.acc.a8.rA));
  regs8->get_cell(1)->decode((t_mem*)&(reg.acc.a8.rB));
  regs8->get_cell(2)->decode((t_mem*)&(reg.DP));
  regs8->get_cell(3)->decode((t_mem*)&(reg.CC));

  regs16= new cl_address_space("regs16", 0, 5, 16);
  regs16->init();
  address_spaces->add(regs16);
  regs16->get_cell(0)->decode((t_mem*)&(reg.U));
  regs16->get_cell(1)->decode((t_mem*)&(reg.S));
  regs16->get_cell(2)->decode((t_mem*)&(reg.X));
  regs16->get_cell(3)->decode((t_mem*)&(reg.Y));
  regs16->get_cell(4)->decode((t_mem*)&(reg.acc.rD));

  vars->add("A", regs8, 0, 7, 0, "CPU register A");
  vars->add("B", regs8, 1, 7, 0, "CPU register B");
  vars->add("DP", regs8, 2, 7, 0, "CPU register DP");
  vars->add("CC", regs8, 3, 7, 0, "CPU register CC");

  vars->add("U", regs16, 0, 15, 0, "CPU register U");
  vars->add("S", regs16, 1, 15, 0, "CPU register S");
  vars->add("X", regs16, 2, 15, 0, "CPU register X");
  vars->add("Y", regs16, 3, 15, 0, "CPU register Y");
  vars->add("D", regs16, 4, 15, 0, "CPU register D");
}


void
cl_m6809::analyze_start(void)
{
  struct {
    const char *name;
    t_addr vector, addr;
  } vectors[] = {
    { ".reset", 0xfffe },
    { ".nmi",   0xfffc },
    { ".swi",   0xfffa },
    { ".irq",   0xfff8 },
    { ".firq",  0xfff6 },
    { ".swi2",  0xfff4 },
    { ".swi3",  0xfff2 },
  };

  for (size_t i = 0; i < sizeof(vectors) / sizeof(vectors[0]); i++)
    {
      vectors[i].addr = rom->read(vectors[i].vector) * 256 + rom->read(vectors[i].vector + 1);
      class cl_var *v = new cl_var(vectors[i].name, rom, vectors[i].addr, chars("Auto-generated by analyze"), -1, -1);
      v->init();
      vars->add(v);
    }

  //for (size_t i = 0; i < sizeof(vectors) / sizeof(vectors[0]); i++)
  analyze(vectors[0].addr);
}


void
cl_m6809::analyze(t_addr addr)
{
  struct dis_entry *tt, *di;
  t_addr code;

  code= rom->get(addr);
  if (code == 0x10)
    {
      tt= disass_m6809_10;
      code= rom->get(++addr);
    }
  else if (code == 0x11)
    {
      tt= disass_m6809_11;
      code= rom->get(++addr);
    }
  else
    tt= disass_m6809;
  addr++;

  for (di= tt; (code & di->mask) != di->code && di->mnemonic; di++);

  while (!inst_at(addr) && di && (di->mnemonic!=NULL))
    {
      set_inst_at(addr);

      if (di->branch != ' ')
        {
          t_addr target= addr;
          char branch = di->branch;

          switch (di->branch)
            {
              case 'x': // Returns or indirect jumps that end this execution path immediately
                return;

              case 'j': // Unconditional jumps
              case 's': // Subroutine calls
                    switch (code & 0x30)
                      {
                      case 0x00: // relative (BSR)
                        target= rom->validate_address(addr + (i8_t)rom->get(addr));
                        break;
                      case 0x10: // direct
                        target= rom->get(addr);
                        break;
                      case 0x20: // indexed
                        return;
                      case 0x30: // extended
                        target= rom->get(addr)*256 + rom->get(addr+1);
			addr+= 2;
                        break;
                      }
                    break;

              case 'b': // Conditional branches
              case 'V':
              case 'v':
              case 'B': // Unconditional branches
                if (tt == disass_m6809)
                  {
		    target= rom->validate_address(addr + (i8_t)rom->get(addr));
		    addr++;
		  }
                else
                  {
		    target= rom->validate_address(addr + (i16_t)(rom->get(addr)*256 + rom->get(addr+1)));
		    branch = 'b';
		    addr+= 2;
		  }
                break;

              default:
                return;
            }

          analyze_jump(addr, target, branch);

          // Unconditional jumps end this execution path
          if (di->branch == 'j' || di->branch == 'B')
            break;
        }

      addr= rom->validate_address(addr + di->length);
    }
}


struct dis_entry *
cl_m6809::dis_tbl(void)
{
  return(disass_m6809);
}

const char *idx_reg_name[4]=
  {
   "X",
   "Y",
   "U",
   "S"
  };

void
cl_m6809::disass_indexed(t_addr *addr, chars *work, int siz)
{
  t_addr a= *addr;
  u8_t idx= rom->get(a++);
  int r= (idx&0x60) >> 5;
  chars w= "";
  i8_t i8;
  i16_t off;
  u16_t u16;
  
  if ((idx & 0x80) == 0)
    {
      off= idx & 0x1f;
      if (idx & 0x10) off|= 0xfff0;
      work->appendf("%+d,%s", off, idx_reg_name[r]);		    
    }
  else
    {
      switch (idx & 0x0f)
	{
	case 0x00:
	  work->appendf(",%s+", idx_reg_name[r]);
	  break;
	case 0x01:
	  w.format(",%s++", idx_reg_name[r]);
	  break;
	case 0x02:
	  work->appendf(",-%s", idx_reg_name[r]);
	  break;
	case 0x03:
	  w.format(",--%s", idx_reg_name[r]);
	  break;
	case 0x04:
	  w.format(",%s", idx_reg_name[r]);
	  break;
	case 0x05:
	  w.format("B,%s", idx_reg_name[r]);
	  break;
	case 0x06:
	  w.format("A,%s", idx_reg_name[r]);
	  break;
	case 0x08:
	  i8= rom->get(a++);
	  off= i8;
	  w.format("%+d,%s", off, idx_reg_name[r]);
	  break;
	case 0x09:
	  i8= rom->get(a++);
	  off= i8*256 + rom->get(a++);
	  w.format("%+d,%s", off, idx_reg_name[r]);
	  break;
	case 0x0b:
	  w.format("D,%s", idx_reg_name[r]);
	  break;
	case 0x0c:
	  i8= rom->get(a++);
	  off= i8;
	  w.format("%+d,PC", off);
	  break;
	case 0x0d:
	  i8= rom->get(a++);
	  off= i8*256 + rom->get(a++);
	  w.format("%+d,PC", off);
	  break;
	case 0x0f:
	  u16= rom->get(a++)*256;
	  u16+= rom->get(a++);
	  if ((idx & 0x10) == 0)
	    work->append("??");
	  else
	    w.format("$%04x", u16);
	  break;
	default:
	  work->append("??");
	  break;
	}
      if (w.nempty())
	{
	  if (idx & 0x10)
	    work->appendf("[%s]", w.c_str());
	  else
	    work->append(w);
	}
    }
  *addr= a;
}

void
cl_m6809::disass_immediate(t_addr *addr, chars *work, int siz)
{
  t_addr a= *addr;
  //work+= 'm';
  u8_t op8= rom->get(a++);
  if (siz==1)
    work->appendf("#$%02x", op8);
  else
    {
      u16_t op16= op8*256 + rom->get(a++);
      work->appendf("#$%04x", op16);
    }
  *addr= a;
}

const char *reg_names[16]=
  {
   /* 0*/ "D",
   /* 1*/ "X",
   /* 2*/ "Y",
   /* 3*/ "U",
   /* 4*/ "S",
   /* 5*/ "PC",
   /* 6*/ "?",
   /* 7*/ "?",
   /* 8*/ "A",
   /* 9*/ "B",
   /*10*/ "CC",
   /*11*/ "DP",
   /*12*/ "?",
   /*13*/ "?",
   /*14*/ "?",
   /*15*/ "?"
  };
   
char *
cl_m6809::disass(t_addr addr)
{
  chars work= chars(), temp= chars();
  const char *b;
  t_mem code;
  int i, j;
  struct dis_entry *tt;
  u8_t op8;
  i8_t i8;
  i16_t i16;
  u16_t op16;
  //t_addr ea;
  bool first;
  int siz;

  //work= "";
  //p= (char*)work;

  code= rom->get(addr);
  if (code == 0x10)
    {
      tt= disass_m6809_10;
      code= rom->get(++addr);
    }
  else if (code == 0x11)
    {
      tt= disass_m6809_11;
      code= rom->get(++addr);
    }
  else
    tt= disass_m6809;
  addr++;
  
  i= 0;
  while ((code & tt[i].mask) != tt[i].code &&
	 tt[i].mnemonic)
    {
      //printf("code=%02x mask=%02x MSKED=%02x ? tt[code]=%02x\n",code,tt[i].mask,code&tt[i].mask,tt[i].code);
      i++;
    }
  if (tt[i].mnemonic == NULL)
    {
      return strdup("-- UNKNOWN/INVALID");
    }
  b= tt[i].mnemonic;

  work= "";
  first= true;
  for (j= 0; b[j]; j++)
    {
      if (b[j] == ' ')
	{
	  if (first)
	    {
	      first= false;
	      while (work.len() < 7) work.append(' ');
	    }
	  else
	    work+= ' ';
	}
      else if (b[j] == '%')
	{
	  j++;
	  switch (b[j])
	    {
	    case 'u': case 'U': case 'n': case 'N':
	      {
		u8_t mode= code & 0x30;
		siz= (islower(b[j]))?1:2;
		switch (mode)
		  {
		  case 0x00: // immed
		    if (toupper(b[j])=='N')
		      work.append("??");
		    else
		      disass_immediate(&addr, &work, siz);
		    break;
		  case 0x10: // direct
		    //work+= 'd';
		    op8= rom->get(addr++);
		    work.appendf("DP:$%02x", op8);
		    break;
		  case 0x20: // index
		    //work+= 'i';
		    disass_indexed(&addr, &work, siz);
		    break;
		  case 0x30: // extend
		    //work+= 'e';
		    op16= rom->get(addr++)*256;
		    op16+= rom->get(addr++);
		    work.appendf("$%04x", op16);
		    addr_name(op16, rom, &work);
		    break;
		  }
		break;
	      }
	    case 'j': case 'J':
	      {
		u8_t mode= code & 0x30;
		switch (mode)
		  {
		  case 0x00: // direct (JMP)
		  case 0x10: // direct (JSR)
		    op8= rom->get(addr++);
		    work.appendf("DP:$%02x", op8);
		    break;
		  case 0x20: // indexed
		    disass_indexed(&addr, &work, 1/*siz*/);
		    break;
		  case 0x30: // extended
		    op16= rom->get(addr++)*256;
		    op16+= rom->get(addr++);
		    work.appendf("$%04x", op16);
		    addr_name(op16, rom, &work);
		    break;
		  }
		break;
	      }
	    case 'x': case 'X':
	      {
		siz= (islower(b[j]))?1:2;
		disass_indexed(&addr, &work, siz);
		break;
	      }
	    case 'i':
	      {
		siz= (islower(b[j]))?1:2;
		disass_immediate(&addr, &work, siz);
		break;
	      }
	    case 'b': case 'B':
	      {
		siz= (islower(b[j]))?1:2;
		if (siz==1)
		  {
		    i8= rom->get(addr++);
		    i16= i8;
		  }
		else
		  {
		    i16= rom->get(addr++)*256;
		    i16+= rom->get(addr++);
		  }
		i16= (i16 + addr) & 0xffff;
		work.appendf("$%04x", i16);
		addr_name(i16, rom, &work);
		break;
	      }
	    case 'p': case 'P':
	      { 
		chars r= "";
		op8= rom->get(addr++);
		if (op8 & 0x80) (r.nempty()?(r+=','):r),r+="PC";
		if (op8 & 0x40) (r.nempty()?(r+=','):r),r+=((b[j]=='p')?"U":"S");
		if (op8 & 0x20) (r.nempty()?(r+=','):r),r+="Y";
		if (op8 & 0x10) (r.nempty()?(r+=','):r),r+="X";
		if (op8 & 0x08) (r.nempty()?(r+=','):r),r+="DP";
		if (op8 & 0x04) (r.nempty()?(r+=','):r),r+="B";
		if (op8 & 0x02) (r.nempty()?(r+=','):r),r+="A";
		if (op8 & 0x01) (r.nempty()?(r+=','):r),r+="CC";
		work.append(r);
		break;
	      }
	    case 'r':
	      {
		op8= rom->get(addr++);
		u8_t s= op8>>4;
		u8_t d= op8&0xf;
		work.appendf("%s,%s", reg_names[s], reg_names[d]);
		if ((s^d)&0x8)
		  work+= "-??";
		break;
	      }
	    case 'e': case 'E':
	      op16= rom->get(addr++);
	      if (op16 & 0x80)
		op16|= 0xff00;
	      if (b[j]=='E')
		op16= op16*256 + rom->get(addr++);
	      op16= (addr + op16)&0xffff;
	      work.appendf("$%04x", op16);
	      addr_name(op16, rom, &work);
	      break;
	    }
	}
      else
	{
	  work+= b[j];
	}
    }
  
  return strdup(work.c_str());
}

void
cl_m6809::print_regs(class cl_console_base *con)
{
  con->dd_color("answer");
  con->dd_printf("A= $%02x %3d %+4d %c  ", A, A, (i8_t)A, isprint(A)?A:'.');
  con->dd_printf("B= $%02x %3d %+4d %c  ", B, B, (i8_t)B, isprint(B)?B:'.');
  con->dd_printf("D= $%04x %5d %+6d\n", D, D, (i16_t)D);
  con->dd_printf("CC= "); con->print_bin(reg.CC, 8); con->dd_printf("\n");
  con->dd_printf("    EFHINZVC\n");

  con->dd_printf("DP= $%02x\n", reg.DP);

  con->dd_printf("X= ");
  class cl_dump_ads ads(reg.X, reg.X+7);
  rom->dump(0, /*reg.X, reg.X+7*/&ads, 8, con);
  con->dd_color("answer");
  
  con->dd_printf("Y= ");
  ads._ss(reg.Y, reg.Y+7);
  rom->dump(0, /*reg.Y, reg.Y+7*/&ads, 8, con);
  con->dd_color("answer");
  
  con->dd_printf("S= ");
  ads._ss(reg.S, reg.S+7);
  rom->dump(0, /*reg.S, reg.S+7*/&ads, 8, con);
  con->dd_color("answer");
  
  con->dd_printf("U= ");
  ads._ss(reg.U, reg.U+7);
  rom->dump(0, /*reg.U, reg.U+7*/&ads, 8, con);
  con->dd_color("answer");
  
  print_disass(PC, con);
}

int
cl_m6809::indexed_length(t_addr addr)
{
  u8_t idx= rom->get(addr+1);
  if ((idx&0x80)!=0)
    {
      u8_t il= idx&0xf;
      if (il==8||il==12)
	return 3;
      if (il==9||il==13||il==15)
	return 4;
    }
  return 2;
}

int
cl_m6809::inst_length(t_addr addr)
{
  u8_t code= rom->get(addr);
  u8_t ch, cl;
  int ret= 1;
  ch= code>>4;
  cl= code&0xf;
  if (code == 0x10)
    {
      ret= 2;
      code= rom->get(addr+1);
      ch= code>>4;
      cl= code&0xf;
      int aml= 1;
      switch (code & 0x30)
	{
	case 0x00: aml= 1+1; break; // immed
	case 0x10: aml= 1; break; // direct
	case 0x20: aml= indexed_length(addr+1); break;// index
	case 0x30: aml= 2; // extend
	}
      ret+= aml;
    }
  else if (code == 0x11)
    {
      ret= 2;
      code= rom->get(addr+1);
      if (code==0x3f) return ret;
      int aml= 1;
      switch (code & 0x30)
	{
	case 0x00: aml= 1+1; break; // immed
	case 0x10: aml= 1; break; // direct
	case 0x20: aml= indexed_length(addr+1); break;// index
	case 0x30: aml= 2; // extend
	}
      ret+= aml;
    }
  else if (code & 0x80)
    {
      // HIGH HALF
      switch (code & 0x30)
	{
	case 0x00: // immed
	  if (cl==3||cl==12||cl==14)
	    return 3;
	  return 2;
	  break;
	case 0x10: // direct
	  return 2;
	  break;
	case 0x20: // index
	  return indexed_length(addr);
	  break;
	case 0x30: // extend
	  return 3;
	  break;
	}
    }
  else
    {
      // LOW HALF
      if (ch==0) return 2; // direct
      if (ch==1)
	{
	  if (cl==6||cl==7) return 3;
	  if (cl==10||cl==12||cl==13||cl==15) return 2;
	}
      if (ch==2) return 2;
      if (ch==3)
	{
	  if (cl < 4) return indexed_length(addr);
	  if (cl < 8) return 2;
	}
      if (ch==6) return indexed_length(addr);
      if (ch==7) return 3; // extend
    }
  return ret;
}

int
cl_m6809::index2ea(u8_t idx, t_addr *res_ea)
{
  u16_t iv;
  i16_t off;
  u16_t *ir= &reg.X;
  t_addr ea= 0;

  switch (idx & 0x60)
    {
    case 0x00: ir= &reg.X; break;
    case 0x20: ir= &reg.Y; break;
    case 0x40: ir= &reg.U; break;
    case 0x60: ir= &reg.S; break;
    }
  iv= *ir;
  if (!(idx & 0x80))
    {
      idx&= 0x1f;
      off= (idx & 0x10)? (0xffe0 | idx) : (idx);
      ea= iv + off;
    }
  else
    {
      i8_t i8;
      bool ind= true;
      switch (idx & 0xf)
	{
	case 0x00:
	  if (idx & 0x10) return resINV_INST;
	  off= 0;
	  ea= iv;
	  (*ir)++;
	  ind= false;
	  break;
	case 0x01:
	  off= 0;
	  ea= iv;
	  (*ir)+= 2;
	  break;
	case 0x02:
	  if (idx & 0x10) return resINV_INST;
	  off= 0;
	  (*ir)--;
	  iv= *ir;
	  ea= iv;
	  ind= false;
	  break;
	case 0x03:
	  off= 0;
	  (*ir)-= 2;
	  iv= *ir;
	  ea= iv;
	  break;
	case 0x04:
	  off= 0;
	  ea= iv;
	  break;
	case 0x05:
	  off= (i8_t)B;
	  ea= iv + off;
	  break;
	case 0x06:
	  off= (i8_t)A;
	  ea= iv + off;
	  break;
	case 0x07:
	  return resINV_INST;
	  break;
	case 0x08:
	  i8= fetch();
	  tick(1);
	  off= i8;
	  ea= iv + off;
	  break;
	case 0x09:
	  off= fetch()*256 + fetch();
	  tick(2);
	  ea= iv + off;
	  break;
	case 0x0a:
	  return resINV_INST;
	  break;
	case 0x0b:
	  off= D;
	  ea= iv + off;
	  break;
	case 0x0c:
	  i8= fetch();
	  tick(1);
	  off= i8;
	  iv= PC;
	  ea= iv + off;
	  break;
	case 0x0d:
	  off= fetch()*256 + fetch();
	  tick(2);
	  iv= PC;
	  ea= iv + off;
	  break;
	case 0x0e:
	  return resINV_INST;
	  break;
	case 0x0f:
	  if ((idx & 0x10) == 0) return resINV_INST;
	  if ((idx & 0x60) != 0) return resINV_INST;
	  off= 0;
	  iv= fetch()*256 + fetch();
	  tick(2);
	  ea= iv;
	  break;
	}
      if (ind && (idx & 0x10))
	{
	  u16_t a;
	  a= rom->read(ea)*256 + rom->read(ea+1);
	  ea= a;
	  tick(2);
	}
    }

  if (res_ea)
    *res_ea= ea;
  return resGO;
}


void
cl_m6809::push_regs(bool do_cc)
{
  rom->write(--reg.S, PC&0xff);
  rom->write(--reg.S, PC>>8);
  tick(2);
  if (reg.CC & flagE)
    {
      rom->write(--reg.S, reg.U&0xff);
      rom->write(--reg.S, reg.U>>8);
      rom->write(--reg.S, reg.Y&0xff);
      rom->write(--reg.S, reg.Y>>8);
      rom->write(--reg.S, reg.X&0xff);
      rom->write(--reg.S, reg.X>>8);
      rom->write(--reg.S, reg.DP);
      rom->write(--reg.S, B);
      rom->write(--reg.S, A);
      tick(9);
      if (do_cc)
	rom->write(--reg.S, reg.CC), tick(1);
    }
}

void
cl_m6809::pull_regs(bool do_cc)
{
  u8_t l,h;
  if(do_cc)
    {
      reg.CC= rom->read(reg.S++), vc.rd++;
      tick(1);
    }
  if (reg.CC & flagE)
    {
      A= rom->read(reg.S++);
      B= rom->read(reg.S++);
      reg.DP= rom->read(reg.S++);
      h= rom->read(reg.S++);
      l= rom->read(reg.S++);
      reg.X= h*256 + l;
      h= rom->read(reg.S++);
      l= rom->read(reg.S++);
      reg.Y= h*256 + l;
      h= rom->read(reg.S++);
      l= rom->read(reg.S++);
      reg.U= h*256 + l;
      vc.rd+= 9;
      tick(9);
    }
  h= rom->read(reg.S++);
  l= rom->read(reg.S++);
  tick(2);
  vc.rd+= 2;
  PC= h*256 + l;
}

int
cl_m6809::inst_add8(t_mem code, u8_t *acc, u8_t op, bool c, bool store, bool invert_c)
{
  u8_t r;
  unsigned int d= *acc;
  unsigned int o= op;
  signed int res= (signed char)d + (signed char)o;

  if (c) { ++res, ++o; }
  
  reg.CC= ~(flagH|flagV|flagS|flagZ|flagC);
  if ((d & 0xf) + (o & 0xf) > 0xf)  reg.CC|= flagH;
  if ((res < -128) || (res > +127)) reg.CC|= flagV;
  if (d + o > 0xff)                 reg.CC|= flagC;
  if (invert_c)
    reg.CC^= flagC;

  r= res & 0xff;
  if (r == 0)   reg.CC|= flagZ;
  if (r & 0x80) reg.CC|= flagS;

  if (store)
    *acc= r;
  
  return resGO;
}


int
cl_m6809::inst_add16(t_mem code, u16_t *acc, u16_t op, bool c,
		     bool store, bool invert_c, bool is_sub)
{
  u16_t r;
  unsigned int d= *acc;
  unsigned int o= op;
  u32_t res= d + o;
  if (c)
    res++;
  r= res;
  
  reg.CC= ~(flagV|flagS|flagZ|flagC);
  //if ((res < (int)(0x8000)) || (res > (int)(0x7fff)))
  if (is_sub)
    {
      o= ~o;
      if (0x8000 & ((d&~o&~r) | (~d&o&r)))
	reg.CC|= flagV;
    }
  else
    {
      if (0x8000 & ((d&o&~r) | (~d&~o&r)))
	reg.CC|= flagV;
    }
  if (res > 0xffff)
    reg.CC|= flagC;
  if (invert_c)
    reg.CC^= flagC;

  //r= res & 0xffff;
  if (r == 0)     reg.CC|= flagZ;
  if (r & 0x8000) reg.CC|= flagS;

  if (store)
    {
      *acc= r;
      if (acc == &reg.S)
	en_nmi= true;
    }
  
  return resGO;
}

int
cl_m6809::inst_bool(t_mem code, char bop, u8_t *acc, u8_t op, bool store)
{
  u8_t r;

  switch (bop)
    {
    case '&': r= *acc & op; break;
    case '|': r= *acc | op; break;
    case '^': r= *acc ^ op; break;
    default: r= *acc; break;
    }

  if (store)
    *acc= r;

  SET_O(0);
  SET_Z(r);
  SET_S(r&0x80);
  
  return resGO;
}

int
cl_m6809::inst_ld8(t_mem code, u8_t *acc, u8_t op)
{
  *acc= op;

  SET_O(0);
  SET_Z(op);
  SET_S(op&0x80);
  
  return resGO;
}

int
cl_m6809::inst_ld16(t_mem code, u16_t *acc, u16_t op)
{
  *acc= op;

  SET_O(0);
  SET_Z(op);
  SET_S(op&0x8000);

  if (acc == &reg.S)
    en_nmi= true;
  
  return resGO;
}

int
cl_m6809::inst_st8(t_mem code, u8_t src, t_addr ea)
{
  rom->write(ea, src);
  tick(1);
  vc.wr++;
  
  SET_O(0);
  SET_Z(src);
  SET_S(src & 0x80);

  return resGO;
}

int
cl_m6809::inst_st16(t_mem code, u16_t src, t_addr ea)
{
  rom->write(ea  , (src)>>8);
  rom->write(ea+1, (src)&0xff);
  tick(2);
  vc.wr+= 2;
  
  SET_O(0);
  SET_Z(src);
  SET_S(src & 0x8000);

  return resGO;
}

int
cl_m6809::inst_alu(t_mem code)
{
  u8_t *acc, op8, idx;
  u16_t op16;
  t_addr ea;
  
  if (code == 0x87 ||
      code == 0xc7 ||
      code == 0xcd ||
      code == 0x8f ||
      code == 0xcf)
    return resINV_INST;
  
  acc= (code & 0x40)? (&B) : (&A);

  switch (code & 0x30)
    {
    case 0x00: // immed
      ea= PC;
      op8= fetch();
      tick(1);
      break;
    case 0x10: // direct
      ea= reg.DP*256 + fetch();
      tick(2);
      op8= rom->read(ea);
      tick(1);
      vc.rd++;
      break;
    case 0x20: // index
      {
	int r;
	idx= fetch();
	tick(1);
	if ((r= index2ea(idx, &ea)) != resGO)
	  return r;
	op8= rom->read(ea);
	tick(1);
	vc.rd++;
	break;
      }
    case 0x30: // extend
      ea= fetch()*256 + fetch();
      tick(3);
      op8= rom->read(ea);
      tick(1);
      vc.rd++;
      break;
    default: op8= 0; break;
    }

  switch (code & 0x0f)
    {
      //          8    9    A    B    C    D    E    F
    case 0x00: // SUB  SUB  SUB  SUB  SUB  SUB  SUB  SUB
      return inst_add8(code, acc, ~op8, 1, true, true);
      break;
    case 0x01: // CMP  CMP  CMP  CMP  CMP  CMP  CMP  CMP
      return inst_add8(code, acc, ~op8, 1, false, true);
      break;
    case 0x02: // SBC  SBC  SBC  SBC  SBC  SBC  SBC  SBC
      return inst_add8(code, acc, ~op8, (reg.CC&flagC)?0:1, true, true);
      break;
    case 0x03: // SUBD SUBD SUBD SUBD ADDD ADDD ADDD ADDD
      {
	int c= 0;
	int inv= false;
	bool is_sub= false;
	if ((code & 0x30) == 0)
	  {
	    op16= op8*256 + fetch();
	    tick(1);
	  }
	else
	  {
	    op16= op8*256 + rom->read(ea+1);
	    tick(1);
	    vc.rd++;
	  }
	if ((code & 0x40) == 0)
	  op16= ~op16, c= 1, inv= true, is_sub= true;
	return inst_add16(code, &D, op16, c, true, inv, is_sub);
	break;
      }
    case 0x04: // AND  AND  AND  AND  AND  AND  AND  AND
      return inst_bool(code, '&', acc, op8, true);
      break;
    case 0x05: // BIT  BIT  BIT  BIT  BIT  BIT  BIT  BIT
      return inst_bool(code, '&', acc, op8, false);
      break;
    case 0x06: // LD   LD   LD   LD   LD   LD   LD   LD
      return inst_ld8(code, acc, op8);
      break;
    case 0x07: // --   STA  STA  STA  --   STB  STB  STB
      return inst_st8(code, *acc, ea);
      break;
    case 0x08: // EOR  EOR  EOR  EOR  EOR  EOR  EOR  EOR
      return inst_bool(code, '^', acc, op8, true);
      break;
    case 0x09: // ADC  ADC  ADC  ADC  ADC  ADC  ADC  ADC
      return inst_add8(code, acc, op8, (reg.CC&flagC)?1:0, true, false);
      break;
    case 0x0a: // OR   OR   OR   OR   OR   OR   OR   OR
      return inst_bool(code, '|', acc, op8, true);
      break;
    case 0x0b: // ADD  ADD  ADD  ADD  ADD  ADD  ADD  ADD
      return inst_add8(code, acc, op8, 0, true, false);
      break;
    case 0x0c: // CMPX CMPX CMPX CMPX LDD  LDD  LDD  LDD
      if ((code & 0x30) == 0)
	{
	  op16= op8*256 + fetch();
	  tick(1);
	}
      else
	{
	  op16= op8*256 + rom->read(ea+1);
	  tick(1);
	  vc.rd++;
	}
      if ((code & 0x40) == 0)
	return inst_add16(code, &(reg.X), ~op16, 1, false, true, true);
      else
	return inst_ld16(code, &D, op16);
      break;
    case 0x0d: // BSR  JSR  JSR  JSR  --   STD  STD  STD
      if ((code & 0x40) == 0)
	{
	  if ((code & 0x30) == 0)
	    { //BSR
	      i8_t i8= op8;
	      ea= (u16_t)((i16_t)PC + (i16_t)i8);
	      rom->write(--reg.S, PC&0xff);
	      rom->write(--reg.S, (PC>>8)&0xff);
	      tick(2);
	      vc.wr+= 2;
	    }
	  else
	    {
	      //JSR
	      rom->write(--reg.S, PC & 0xff);
	      rom->write(--reg.S, (PC>>8)&0xff);
	      tick(2);
	      vc.wr+= 2;
	    }
	  PC= ea;
	}
      else
	return inst_st16(code, D, ea);
      break;
    case 0x0e: // LDX  LDX  LDX  LDX  LDU  LDU  LDU  LDU
      if ((code & 0x30) == 0)
	{
	  op16= op8*256 + fetch();
	  tick(1);
	}
      else
	{
	  op16= op8*256 + rom->read(ea+1);
	  tick(1);
	  vc.rd++;
	}
      if ((code & 0x40) == 0)
	return inst_ld16(code, &reg.X, op16);
      else
	return inst_ld16(code, &reg.U, op16);
      break;
    case 0x0f: // --   STX  STX  STX  --   STU  STU  STU
      if ((code & 0x40) == 0)
	return inst_st16(code, reg.X, ea);
      else
	return inst_st16(code, reg.U, ea);
      break;
    }
  
  return resGO;
}


int
cl_m6809::inst_10(t_mem code)
{
  u8_t op8;
    
  switch (code & 0x0f)
    {
    case 0x00: // pg1
      break;
    case 0x01: // pg2
      break;
    case 0x02: // NOP
      break;
    case 0x03: // SYNC
      state= stIDLE;
      break;
    case 0x04: // --
      break;
    case 0x05: // --
      break;
    case 0x06: // LBRA
      {
	u16_t u= fetch()*256 + fetch();
	i16_t i= u;
	PC= PC + i;
	tick(2);
	break;
      }
    case 0x07: // LBSR
      {
	u16_t u= fetch()*256 + fetch();
	i16_t i= u;
	tick(2);
	rom->write(--reg.S, PC & 0xff);
	rom->write(--reg.S, (PC>>8)&0xff);
	tick(2);
	vc.wr+= 2;
	PC= PC + i;
	break;
      }
    case 0x08: // --
      break;
    case 0x09: // DAA
      {
	u8_t cf= 0;
	int r;
	if ((reg.CC & flagH) || ((A&0x0f) > 9))
	  cf|= 0x06;
	if ((reg.CC & flagC) ||
	    ((A&0xf0) > 0x90) ||
	    (
	     ((A&0xf0) > 0x80) &&
	     ((A&0x0f) > 0x09)
	     )
	    )
	  cf|= 0x60;
	r= A + cf;
	A= r;
	SET_Z(A);
	SET_S(A & 0x80);
	if (r>0xff)
	  reg.CC|= flagC;
	tick(1);
	break;
      }
    case 0x0a: // ORCC
      op8= fetch();
      reg.CC|= op8;
      tick(1);
      break;
    case 0x0b: // --
      break;
    case 0x0c: // ANDCC
      op8= fetch();
      reg.CC&= op8;
      tick(1);
      break;
    case 0x0d: // SEX
      A= (B & 0x80)?0xff:0;
      SET_Z(D);
      SET_S(A & 0x80);
      tick(1);
      break;
    case 0x0e: // EXG
      {
	u8_t r1, r2;
	op8= fetch();
	tick(1);
	r1= op8>>4;
	r2= op8&0xf;
	if (((r1^r2)&0x08)!=0)
	  return resINV_INST;
	if (r1<8)
	  {
	    u16_t *R1= reg16_ptr[r1&7];
	    u16_t *R2= reg16_ptr[r2&7];
	    u16_t t;
	    t= (R1)?*R1:PC;
	    *R1= (R2)?*R2:PC;
	    *R2= t;
	  }
	else
	  {
	    u8_t *R1= reg8_ptr[r1&7];
	    u8_t *R2= reg8_ptr[r2&7];
	    u8_t t;
	    t= *R1;
	    *R1= *R2;
	    *R2= t;
	  }
	tick(6);
	break;
      }
    case 0x0f: // TFR
      {
	op8= fetch();
	tick(1);
	u8_t rs= op8>>4;
	u8_t rd= op8&0xf;
	if (((rd^rs)&8)!=0)
	  return resINV_INST;
	if (rs<8)
	  {
	    u16_t *RS= reg16_ptr[rs&7];
	    u16_t *RD= reg16_ptr[rd&7];
	    if (RD)
	      *RD= (RS)?*RS:PC;
	    else
	      PC= (RS)?*RS:PC;
	  }
	else
	  {
	    u8_t *RS= reg8_ptr[rs&7];
	    u8_t *RD= reg8_ptr[rd&7];
	    *RD= *RS;
	  }
	tick(4);
	break;
      }
    }

  return resGO;
}

int
cl_m6809::inst_branch(t_mem code, bool l)
{
  bool c= reg.CC & flagC;
  bool z= reg.CC & flagZ;
  bool n= reg.CC & flagN;
  bool v= reg.CC & flagV;
  bool t= 0;
  
  switch (code & 0x0f)
    {
    case 0x00: // BRA 1
      if (l)
	return resINV_INST;
      t= true;
      break;
    case 0x01: // BRN 0
      t= false;
      break;
    case 0x02: // BHI ~C * ~Z
      t= !c && !z;
      break;
    case 0x03: // BLS C + Z
      t= c || z;
      break;
    case 0x04: // BHS ~C
      t= !c;
      break;
    case 0x05: // BLO C
      t= c;
      break;
    case 0x06: // BNE ~Z
      t= !z;
      break;
    case 0x07: // BEQ Z
      t= z;
      break;
    case 0x08: // BVC ~V
      t= !v;
      break;
    case 0x09: // BVS V
      t= v;
      break;
    case 0x0a: // BPL ~N
      t= !n;
      break;
    case 0x0b: // BMI N
      t= n;
      break;
    case 0x0c: // BGE N*V + ~N*~V
      t= (n&&v) || (!n&&!v);
      break;
    case 0x0d: // BLT N*~V + ~N*V
      t= (n&&!v) || (!n&&v);
      break;
    case 0x0e: // BGT N*V*~Z + ~N*~V*~Z
      t= (n&&v&&!z) || (!n&&!v&&!z);
      break;
    case 0x0f: // BLE Z + N*~V + ~N*V
      t= z || (n&&!v) || (!n&&v);
      break;
    }

  i16_t i= fetch();
  tick(1);
  if (i&0x80) i|= 0xff00;
  if (l)
    {
      i= i*256 + fetch();
      tick(1);
    }
  if (t)
    {
      PC= PC + i;
      tick(1);
    }
      
  return resGO;
}

int
cl_m6809::inst_30(t_mem code)
{
  t_addr ea;
  int r;
  u8_t op8, l, h;    
  
  switch (code & 0x0f)
    {
    case 0x00: // LEAX
      r= index2ea(fetch(), &ea);
      tick(1);
      if (r!=resGO)
	return r;
      reg.X= ea;
      SET_Z(reg.X);
      tick(1);
      break;
    case 0x01: // LEAY
      r= index2ea(fetch(), &ea);
      tick(1);
      if (r!=resGO)
	return r;
      reg.Y= ea;
      SET_Z(reg.Y);
      tick(1);
      break;
    case 0x02: // LEAS
      r= index2ea(fetch(), &ea);
      tick(1);
      if (r!=resGO)
	return r;
      reg.S= ea;
      tick(1);
      break;
    case 0x03: // LEAU
      r= index2ea(fetch(), &ea);
      tick(1);
      if (r!=resGO)
	return r;
      reg.U= ea;
      tick(1);
      break;
    case 0x04: // PSHS
      op8= fetch();
      tick(1);
      if (op8 & 0x80)
	rom->write(--reg.S, PC&0xff),
	  rom->write(--reg.S, PC>>8),
	  vc.wr+= 2,
	  tick(2);
      if (op8 & 0x40)
	rom->write(--reg.S, reg.U&0xff),
	  rom->write(--reg.S, reg.U>>8),
	  vc.wr+= 2,
	  tick(2);
      if (op8 & 0x20)
	rom->write(--reg.S, reg.Y&0xff),
	  rom->write(--reg.S, reg.Y>>8),
	  vc.wr+= 2,
	  tick(2);
      if (op8 & 0x10)
	rom->write(--reg.S, reg.X&0xff),
	  rom->write(--reg.S, reg.X>>8),
	  vc.wr+= 2,
	  tick(2);
      if (op8 & 0x08)
	rom->write(--reg.S, reg.DP), vc.wr++, tick(1);
      if (op8 & 0x04)
	rom->write(--reg.S, B), vc.wr++, tick(1);
      if (op8 & 0x02)
	rom->write(--reg.S, A), vc.wr++, tick(1);
      if (op8 & 0x01)
	rom->write(--reg.S, reg.CC), vc.wr++, tick(1);
      break;
    case 0x05: // PULS
      op8= fetch();
      if(op8 & 0x01)
	reg.CC= rom->read(reg.S++), vc.rd++, tick(1);
      if (op8 & 0x02)
	A= rom->read(reg.S++), vc.rd++, tick(1);
      if (op8 & 0x04)
	B= rom->read(reg.S++), vc.rd++, tick(1);
      if (op8 & 0x08)
	reg.DP= rom->read(reg.S++), vc.rd++, tick(1);
      if (op8 & 0x10)
	h= rom->read(reg.S++),
	  l= rom->read(reg.S++),
	  reg.X= h*256 + l,
	  vc.rd+= 2,
	  tick(2);
      if (op8 & 0x20)
	h= rom->read(reg.S++),
	  l= rom->read(reg.S++),
	  reg.Y= h*256 + l,
	  vc.rd+= 2,
	  tick(2);
      if (op8 & 0x40)
	h= rom->read(reg.S++),
	  l= rom->read(reg.S++),
	  reg.U= h*256 + l,
	  vc.rd+= 2,
	  tick(2);
      if (op8 & 0x80)
	h= rom->read(reg.S++),
	  l= rom->read(reg.S++),
	  PC= h*256 + l,
	  vc.rd+= 2,
	  tick(2);
      break;
    case 0x06: // PSHU
      op8= fetch();
      if (op8 & 0x80)
	rom->write(--reg.U, PC&0xff),
	  rom->write(--reg.U, PC>>8),
	  vc.wr+= 2,
	  tick(2);
      if (op8 & 0x40)
	rom->write(--reg.U, reg.S&0xff),
	  rom->write(--reg.U, reg.S>>8),
	  vc.wr+= 2,
	  tick(2);
      if (op8 & 0x20)
	rom->write(--reg.U, reg.Y&0xff),
	  rom->write(--reg.U, reg.Y>>8),
	  vc.wr+= 2,
	  tick(2);
      if (op8 & 0x10)
	rom->write(--reg.U, reg.X&0xff),
	  rom->write(--reg.U, reg.X>>8),
	  vc.wr+= 2,
	  tick(2);
      if (op8 & 0x08)
	rom->write(--reg.U, reg.DP), vc.wr++, tick(1);
      if (op8 & 0x04)
	rom->write(--reg.U, B), vc.wr++, tick(1);
      if (op8 & 0x02)
	rom->write(--reg.U, A), vc.wr++, tick(1);
      if (op8 & 0x01)
	rom->write(--reg.U, reg.CC), vc.wr++, tick(1);
      break;
    case 0x07: // PULU
      op8= fetch();
      if(op8 & 0x01)
	reg.CC= rom->read(reg.U++), vc.rd++, tick(1);
      if (op8 & 0x02)
	A= rom->read(reg.U++), vc.rd++, tick(1);
      if (op8 & 0x04)
	B= rom->read(reg.U++), vc.rd++, tick(1);
      if (op8 & 0x08)
	reg.DP= rom->read(reg.U++), vc.rd++, tick(1);
      if (op8 & 0x10)
	h= rom->read(reg.U++),
	  l= rom->read(reg.U++),
	  reg.X= h*256 + l,
	  vc.rd+= 2,
	  tick(2);
      if (op8 & 0x20)
	h= rom->read(reg.U++),
	  l= rom->read(reg.U++),
	  reg.Y= h*256 + l,
	  vc.rd+= 2,
	  tick(2);
      if (op8 & 0x40)
	h= rom->read(reg.U++),
	  l= rom->read(reg.U++),
	  reg.S= h*256 + l,
	  vc.rd+= 2,
	  tick(2);
      if (op8 & 0x80)
	h= rom->read(reg.U++),
	  l= rom->read(reg.U++),
	  PC= h*256 + l,
	  vc.rd+= 2,
	  tick(2);
      break;
    case 0x08: // --
      break;
    case 0x09: // RTS
      ea= rom->read(reg.S++) * 256;
      ea+= rom->read(reg.S++);
      tick(2);
      vc.rd+= 2;
      PC= ea;
      tick(1);
      break;
    case 0x0a: // ABX
      reg.X+= B;
      tick(2);
      break;
    case 0x0b: // RTI
      pull_regs(true);
      tick(1);
      break;
    case 0x0c: // CWAI
      op8= fetch();
      tick(1);
      reg.CC&= op8;
      reg.CC|= flagE;
      push_regs(true);
      cwai= true;
      state= stIDLE;
      break;
    case 0x0d: // MUL
      D= A * B;
      SET_Z(D);
      SET_C(B & 0x80);
      tick(9);
      break;
    case 0x0e: // RESET
      reset();
      break;
    case 0x0f: // SWI
      reg.CC|= flagE;
      tick(2);
      push_regs(true);
      reg.CC|= flagF|flagI;
      PC= rom->read(0xfffa)*256 + rom->read(0xfffb);
      tick(2);
      tick(1);
      break;
    }

  return resGO;
}


int
cl_m6809::inst_neg(t_mem code, u8_t *acc, t_addr ea, u8_t op8)
{
  if (acc)
    {
      *acc= ~(*acc);
      return inst_add8(code, acc, 0, 1, true, true);
    }
  op8= rom->read(ea);
  tick(1);
  vc.rd++;
  u8_t t= A;
  A= ~op8;
  inst_add8(code, &A, 0, 1, true, true);
  rom->write(ea, A);
  tick(1);
  vc.wr++;
  A= t;
  return resGO;
}

int
cl_m6809::inst_com(t_mem code, u8_t *acc, t_addr ea, u8_t op8)
{
  if (acc)
    {
      op8= *acc= ~(*acc);
    }
  else
    {
      op8= ~(rom->read(ea));
      tick(1);
      vc.rd++;
      rom->write(ea, op8);
      tick(1);
      vc.wr++;
    }
  
  SET_C(1);
  SET_O(0);
  SET_Z(op8);
  SET_S(op8 & 0x80);
  
  return resGO;
}

int
cl_m6809::inst_lsr(t_mem code, u8_t *acc, t_addr ea, u8_t op8)
{
  if (acc)
    {
      SET_C(*acc & 1);
      op8= *acc= (*acc) >> 1;
    }
  else
    {
      op8= rom->read(ea);
      tick(1);
      vc.rd++;
      SET_C(op8 & 1);
      op8>>= 1;
      rom->write(ea, op8);
      tick(1);
      vc.wr++;
    }
  
  SET_Z(op8);
  SET_S(op8 & 0x80);
  
  return resGO;
}

int
cl_m6809::inst_ror(t_mem code, u8_t *acc, t_addr ea, u8_t op8)
{
  u8_t oldc, newc;

  if (acc)
    op8= *acc;
  else
    {
      op8= rom->read(ea);
      tick(1);
      vc.rd++;
    }
  oldc= reg.CC & flagC;
  newc= op8 & 1;
  
  op8>>= 1;
  if (oldc)
    op8|= 0x80;

  if (acc)
    *acc= op8;
  else
    rom->write(ea, op8), vc.wr++, tick(1);
  
  SET_C(newc);
  SET_Z(op8);
  SET_S(op8 & 0x80);
  
  return resGO;
}

int
cl_m6809::inst_asr(t_mem code, u8_t *acc, t_addr ea, u8_t op8)
{
  u8_t old8, newc;

  if (acc)
    op8= *acc;
  else
    {
      op8= rom->read(ea);
      tick(1);
      vc.rd++;
    }
  old8= op8 & 0x80;
  newc= op8 & 1;
  
  op8>>= 1;
  if (old8)
    op8|= 0x80;

  if (acc)
    *acc= op8;
  else
    rom->write(ea, op8), vc.wr++, tick(1);
  
  SET_C(newc);
  SET_Z(op8);
  SET_S(op8 & 0x80);
  
  return resGO;
}

int
cl_m6809::inst_asl(t_mem code, u8_t *acc, t_addr ea, u8_t op8)
{
  u8_t newc;
  
  op8= (acc)?(*acc):(vc.rd++, tick(1), rom->read(ea));
  newc= op8 & 0x80;
  
  op8<<= 1;

  if (acc)
    *acc= op8;
  else
    rom->write(ea, op8), vc.wr++, tick(1);
  
  SET_C(newc);
  SET_Z(op8);
  SET_S(op8 & 0x80);
  
  return resGO;
}

int
cl_m6809::inst_rol(t_mem code, u8_t *acc, t_addr ea, u8_t op8)
{
  u8_t oldc, newc;
  
  op8= (acc)?(*acc):(vc.rd++, tick(1), rom->read(ea));
  oldc= reg.CC & flagC;
  newc= op8 & 0x80;

  SET_O( (op8&0x80) ^ ((op8<<1)&0x80) );
  
  op8<<= 1;
  if (oldc)
    op8|= 1;

  if (acc)
    *acc= op8;
  else
    rom->write(ea, op8), vc.wr++, tick(1);
  
  SET_C(newc);
  SET_Z(op8);
  SET_S(op8 & 0x80);
  
  return resGO;
}

int
cl_m6809::inst_dec(t_mem code, u8_t *acc, t_addr ea, u8_t op8)
{
  op8= (acc)?(*acc):(vc.rd++, tick(1), rom->read(ea));
  SET_O(op8==0x80);

  op8--;
  
  if (acc)
    *acc= op8;
  else
    rom->write(ea, op8), vc.wr++, tick(1);
  
  SET_Z(op8);
  SET_S(op8 & 0x80);

  return resGO;
}

int
cl_m6809::inst_inc(t_mem code, u8_t *acc, t_addr ea, u8_t op8)
{
  op8= (acc)?(*acc):(vc.rd++, tick(1), rom->read(ea));
  SET_O(op8==0x7f);

  op8++;
  
  if (acc)
    *acc= op8;
  else
    rom->write(ea, op8), vc.wr++, tick(1);
  
  SET_Z(op8);
  SET_S(op8 & 0x80);

  return resGO;
}

int
cl_m6809::inst_tst(t_mem code, u8_t *acc, t_addr ea, u8_t op8)
{
  op8= (acc)?(*acc):(vc.rd++, tick(1), rom->read(ea));
  SET_O(0);
  
  SET_Z(op8);
  SET_S(op8 & 0x80);

  return resGO;
}

int
cl_m6809::inst_clr(t_mem code, u8_t *acc, t_addr ea, u8_t op8)
{
  if (!acc)
    {
      volatile u8_t u= rom->read(ea);
      op8= u;
      vc.rd++;
      tick(1);
    }
  
  if (acc)
    *acc= 0;
  else
    rom->write(ea, 0), vc.wr++, tick(1);
  
  SET_O(0);
  SET_Z(1);
  SET_S(0);
  SET_C(0);

  return resGO;
}

const u8_t low_illegals[]=
  {
   0x01, 0x41, 0x51, 0x61, 0x71,
   0x02, 0x42, 0x52, 0x62, 0x72,
   0x14,
   0x05, 0x15, 0x45, 0x55, 0x65, 0x75,
   0x18, 0x38,
   0x0b, 0x1b, 0x4b, 0x5b, 0x6b, 0x7b,
   0x4e, 0x5e,
   0
  };

int
cl_m6809::inst_low(t_mem code)
{
  t_addr ea= 0;
  u8_t op8= 0, *acc, idx;
  
  for (idx= 0; low_illegals[idx]; idx++)
    if (low_illegals[idx] == code)
      return resINV_INST;

  if ((code & 0xf0) == 0x10)
    return inst_10(code);

  if ((code & 0xf0) == 0x20)
    return inst_branch(code, false);

  if ((code & 0xf0) == 0x30)
    return inst_30(code);
  
  switch (code & 0xf0)
    {
    case 0x00: // direct
      ea= reg.DP*256 + fetch();
      op8= rom->read(ea);
      tick(2);
      vc.rd++;
      break;
    case 0x60: // index
      {
	int r;
	idx= fetch();
	tick(1);
	if ((r= index2ea(idx, &ea)) != resGO)
	  return r;
	op8= rom->read(ea);
	tick(1);
	vc.rd++;
	break;
      }
    case 0x70: // extend
      ea= fetch()*256 + fetch();
      op8= rom->read(ea);
      tick(3);
      vc.rd++;
      break;
    }

  if ((code & 0xf0) == 0x40)
    acc= &A;
  else if ((code & 0xf0) == 0x50)
    acc= &B;
  else
    acc= NULL;
  
  switch (code & 0x0f)
    {
      //          0     1     2     3     4     5     6     7
    case 0x00: // NEG   pg1   BRA   LEAX  NEGA  NEGB  NEG   NEG
      return inst_neg(code, acc, ea, op8);
      break;
    case 0x01: // --    pg2   BRN   LEAY  --    --    --    --
      break;
    case 0x02: // --    NOP   BHI   LEAS  --    --    --    --
      break;
    case 0x03: // COM   SYNC  BLS   LEAU  COMA  COMB  COM   COM
      return inst_com(code, acc, ea, op8);
      break;
    case 0x04: // LSR   --    BHS   PSHS  LSRA  LSRB  LSR   LSR
      return inst_lsr(code, acc, ea, op8);
      break;
    case 0x05: // --    --    BLO   PULS  --    --    --    --
      break;
    case 0x06: // ROR   LBRA  BNE   PSHU  RORA  RORB  ROR   ROR
      return inst_ror(code, acc, ea, op8);
      break;
    case 0x07: // ASR   LBSR  BEQ   PULU  ASRA  ASRB  ASR   ASR
      return inst_asr(code, acc, ea, op8);
      break;
    case 0x08: // ASL   --    BVC   --    ASLA  ASLB  ASL   ASL
      return inst_asl(code, acc, ea, op8);
      break;
    case 0x09: // ROL   DAA   BVS   RTS   ROLA  ROLB  ROL   ROL
      return inst_rol(code, acc, ea, op8);
      break;
    case 0x0a: // DEC   ORCC  BPL   ABX   DECA  DECB  DEC   DEC
      return inst_dec(code, acc, ea, op8);
      break;
    case 0x0b: // --    --    BMI   RTI   --    --    --    --
      break;
    case 0x0c: // INC   ANDCC BGE   CWAI  INCA  INCB  INC   INC
      return inst_inc(code, acc, ea, op8);
      break;
    case 0x0d: // TST   SEX   BLT   MUL   TSTA  TSTB  TST   TST
      return inst_tst(code, acc, ea, op8);
      break;
    case 0x0e: // JMP   EXG   BGT   RESET --    --    JMP   JMP
      PC= ea;
      break;
    case 0x0f: // CLR   TFR   BLE   SWI   CLRA  CLRB  CLR   CLR
      return inst_clr(code, acc, ea, op8);
      break;
    }
  
  return resGO;
}


int
cl_m6809::inst_page1(t_mem code)
{
  t_addr ea;
  u8_t op8, idx;
  u16_t op16;

  if ((code & 0xf0) == 0x20)
    return inst_branch(code, true);
  if (code == 0x3f)
    {
      // SWI2
      tick(2);
      reg.CC|= flagE;
      push_regs(true);
      tick(1);
      PC= rom->read(0xfff4)*256 + rom->read(0xfff5);
      tick(2);
      tick(1);
      vc.rd+= 2;
    }
  if ((code & 0x80) == 0)
    return resINV_INST;
  
  /*
    1000 immed   3 c e
    1100             e

    1001 direct  3 c e f
    1101             e f

    1010 index   3 c e f
    1110             e f

    1011 extend  3 c e f
    1111             e f
  */
  u8_t cl= code & 0xf;
  u8_t ch= code >> 4;
  switch (code & 0x30)
    {
    case 0x00: // immed
      if ((ch==8)&&(cl!=3)&&(cl!=12)&&(cl!=14)) return resINV_INST;
      if ((ch==12)&&(cl!=14)) return resINV_INST;
      ea= PC;
      op8= fetch();
      tick(1);
      break;
    case 0x10: // direct
      if ((ch==9)&&(cl!=3)&&(cl!=12)&&(cl!=14)&&(cl!=15)) return resINV_INST;
      if ((ch==13)&&(cl!=14)&&(cl!=15)) return resINV_INST;
      ea= reg.DP*256 + fetch();
      tick(1);
      op8= rom->read(ea);
      tick(1);
      vc.rd++;
      break;
    case 0x20: // index
      {
	if ((ch==10)&&(cl!=3)&&(cl!=12)&&(cl!=14)&&(cl!=15)) return resINV_INST;
	if ((ch==14)&&(cl!=14)&&(cl!=15)) return resINV_INST;
	int r;
	idx= fetch();
	tick(1);
	if ((r= index2ea(idx, &ea)) != resGO)
	  return r;
	op8= rom->read(ea);
	tick(1);
	vc.rd++;
	break;
      }
    case 0x30: // extend
      if ((ch==11)&&(cl!=3)&&(cl!=12)&&(cl!=14)&&(cl!=15)) return resINV_INST;
      if ((ch==15)&&(cl!=14)&&(cl!=15)) return resINV_INST;
      ea= fetch()*256 + fetch();
      tick(2);
      op8= rom->read(ea);
      tick(1);
      vc.rd++;
      break;
    default: op8= 0; break;
    }

  if ((code & 0x30) == 0)
    {
      op16= op8*256 + fetch();
      tick(1);
    }
  else
    {
      op16= op8*256 + rom->read(ea+1);
      tick(1);
      vc.rd++;
    }
  
  switch (cl)
    {
    case 3: // CMPD
      inst_add16(code, &(D), ~op16, 1, false, true, true);
      break;
    case 0xc: // CMPY
      inst_add16(code, &(reg.Y), ~op16, 1, false, true, true);
      break;
    case 0xe: // LDY, LDS
      if ((code & 0x40) == 0)
	  inst_ld16(code, &(reg.Y), op16);
      else
	  inst_ld16(code, &(reg.S), op16);
      break;
    case 0xf: // STY, STS
      if (code & 0x40)
	inst_st16(code, reg.Y, ea);
      else
	inst_st16(code, reg.S, ea);
      break;
    }
  
  return resGO;
}


int
cl_m6809::inst_page2(t_mem code)
{
  t_addr ea;
  u8_t op8= 0, idx;
  u16_t op16;
  
  if (code == 0x3f)
    {
      // SWI3
      tick(1);
      reg.CC|= flagE;
      push_regs(true);
      tick(1);
      PC= rom->read(0xfff2)*256 + rom->read(0xfff3);
      tick(2);
      tick(1);
      vc.rd+= 2;
    }
  if ((code!=0x83)&&(code!=0x8c)&&
      (code!=0x93)&&(code!=0x9c)&&
      (code!=0xa3)&&(code!=0xac)&&
      (code!=0xb3)&&(code!=0xbc))
    return resINV_INST;

  switch (code & 0x30)
    {
    case 0x00: //immediate
      ea= PC;
      op8= fetch();
      tick(1);
      break;
    case 0x10: // direct
      ea= reg.DP*256 + fetch();
      tick(1);
      op8= rom->read(ea);
      tick(1);
      vc.rd++;
      break;
    case 0x20: // index
      {
	int r;
	idx= fetch();
	tick(1);
	if ((r= index2ea(idx, &ea)) != resGO)
	  return r;
	op8= rom->read(ea);
	tick(1);
	vc.rd++;
	break;
      }
    case 0x30: // extend
      ea= fetch()*256 + fetch();
      tick(2);
      op8= rom->read(ea);
      tick(1);
      vc.rd++;
      break;
    }

  if ((code & 0x30) == 0)
    {
      op16= op8*256 + fetch();
      tick(1);
    }
  else
    {
      op16= op8*256 + rom->read(ea+1);
      tick(1);
      vc.rd++;
    }
  
  if ((code & 0x0f) == 0x03)
    {
      // CMPU
      inst_add16(code, &(reg.U), ~op16, 1, false, true, true);
      tick(1);
    }
  if ((code & 0x0f) == 0x0c)
    {
      // CMPS
      inst_add16(code, &(reg.S), ~op16, 1, false, true, true);
      tick(1);
    }

  return resGO;
}


int
cl_m6809::exec_inst(void)
{
  t_mem code;
  bool fe;

  fe= fetch(&code);
  tick(1);
  if (fe)
    return(resBREAKPOINT);

  if (code & 0x80)
    return inst_alu(code);
  else
    {
      if (code == 0x10)
	return tick(1), inst_page1(fetch());
      if (code == 0x11)
	return tick(1), inst_page2(fetch());
      return inst_low(code);
    }
  
  return resGO;
}

int
cl_m6809::accept_it(class it_level *il)
{
  class cl_it_src *is= il->source;
  //class cl_m6809_src_base *parent= NULL;
  /*
  if (is)
    {
      if ((parent= is->get_parent()) != NULL)
	{
	  is= parent;
	  il->source= is;
	}
    }
  */
  tick(3);
  reg.CC&= ~flagE;
  if (is == src_irq || is == src_nmi)
    reg.CC|= flagE;//is->Evalue;
    
  if (!cwai)
    push_regs(true);
  cwai= false;
  reg.CC|= flagI;//is->IFvalue;
  if (is == src_firq || is == src_nmi)
    reg.CC|= flagF;
  
  t_addr a= rom->read(is->addr) * 256 + rom->read(is->addr+1);
  tick(2);
  vc.rd+= 2;
  PC= a;

  is->clear();
  
  it_levels->push(il);
  tick(2);
  return resGO;
}


/* End of m6809.src/m6809.cc */
