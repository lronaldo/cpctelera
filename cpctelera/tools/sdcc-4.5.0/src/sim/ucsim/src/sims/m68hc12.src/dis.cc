/*
 * Simulator of microcontrollers (dis.cc)
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

#include <ctype.h>

#include "glob12.h"

#include "m68hc12cl.h"


struct dis_entry *
cl_m68hc12::dis_tbl(void)
{
  return disass12p0;
}

struct dis_entry *
cl_m68hc12::get_dis_entry(t_addr addr)
{
  struct dis_entry *dt;
  int i= 0;
  t_mem code= rom->get(addr);

  if (code == 0x18)
    {
      code= rom->get(addr+1);
      dt= disass12p18;
      while (((code & dt[i].mask) != dt[i].code) &&
	     dt[i].mnemonic)
	i++;
      return &dt[i];
    }
  
  dt= disass12p0;
  while (((code & dt[i].mask) != dt[i].code) &&
	 dt[i].mnemonic)
    i++;
  return &dt[i];

}



char *
cl_m68hc12::disassc(t_addr addr, chars *comment)
{
  chars work= chars(), temp= chars(), fmt= chars();
  const char *b;
  //t_mem code= rom->get(addr);
  struct dis_entry *dis_e;
  int i;
  bool first;

  if (rom->read(addr) == 0x04)
    {
      addr++;
      return disass_loop(&addr, &work, comment);
    }
  
  if ((dis_e= get_dis_entry(addr)) == NULL)
    return NULL;
  if (dis_e->mnemonic == NULL)
    return strdup("-- UNKNOWN/INVALID");
  b= dis_e->mnemonic;

  if (rom->read(addr) == 0x18)
    addr++;

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
	  if (strcmp(fmt.c_str(), "ep") == 0)
	    {
	      u8_t h, l, p;
	      h= rom->read(addr+1);
	      l= rom->read(addr+2);
	      p= rom->read(addr+3);
	      work.appendf("$%04x", h*256+l);
	      work.append(",");
	      work.appendf("$%02x", p);
	    }
	  if (strcmp(fmt.c_str(), "ip") == 0)
	    {
	      t_addr a= addr+1;
	      u8_t xb= rom->read(a);
	      disass_xb(&a, &work, comment, 3);
	      if (!xb_indirect(xb))
		{
		  u8_t p= rom->read(a);
		  work.append(",");
		  work.appendf("$%02x", p);
		}
	    }
	  if (strcmp(fmt.c_str(), "IMID") == 0)
	    {
	      t_addr a= (addr+=2);
	      u8_t h, l, xb= rom->read(a);
	      addr++;
	      h= rom->read(addr++);
	      l= rom->read(addr++);
	      work.appendf("#$%04x", h*256+l);
	      work.append(", ");
	      disass_xb(&a, &work, comment, 2, xb_PC(xb)?2:0);
	    }
	  if (strcmp(fmt.c_str(), "imid") == 0)
	    {
	      t_addr a= (addr+=2);
	      u8_t h, xb= rom->read(a);
	      addr++;
	      h= rom->read(addr++);
	      work.appendf("#$%02x", h);
	      work.append(", ");
	      disass_xb(&a, &work, comment, 1, xb_PC(xb)?1:0);
	    }
	  if (strcmp(fmt.c_str(), "EXID") == 0)
	    {
	      t_addr aof_xb= (addr+=2), asrc;
	      u8_t h, l, xb= rom->read(aof_xb);
	      addr++;
	      h= rom->read(addr++);
	      l= rom->read(addr++);
	      asrc= h*256+l;
	      work.appendf("$%04x", asrc);
	      work.append(", ");
	      comment->appendf("; [%04x]=%04x",asrc,rom->read(asrc)*256+rom->read(asrc+1));
	      disass_xb(&aof_xb, &work, comment, 2, xb_PC(xb)?2:0);
	    }
	  if (strcmp(fmt.c_str(), "exid") == 0)
	    {
	      t_addr aof_xb= (addr+=2), asrc;
	      u8_t h, l, xb= rom->read(aof_xb);
	      addr++;
	      h= rom->read(addr++);
	      l= rom->read(addr++);
	      asrc= h*256+l;
	      work.appendf("$%04x", asrc);
	      work.append(", ");
	      comment->appendf("; [%04x]=%02x",asrc,rom->read(asrc));
	      disass_xb(&aof_xb, &work, comment, 1, xb_PC(xb)?2:0);
	    }
	  if (strcmp(fmt.c_str(), "IDID") == 0)
	    {
	      t_addr aof_xbsrc= (addr+=2);
	      t_addr aof_xbdst= (addr+=1);
	      u8_t xbsrc, xbdst;
	      xbsrc= rom->read(aof_xbsrc);
	      xbdst= rom->read(aof_xbdst);
	      disass_xb(&aof_xbsrc, &work, comment, 2, xb_PC(xbsrc)?-1:0);
	      work.append(", ");
	      disass_xb(&aof_xbdst, &work, comment, 2, xb_PC(xbdst)?1:0);
	    }
	  if (strcmp(fmt.c_str(), "idid") == 0)
	    {
	      t_addr aof_xbsrc= (addr+=2);
	      t_addr aof_xbdst= (addr+=1);
	      u8_t xbsrc, xbdst;
	      xbsrc= rom->read(aof_xbsrc);
	      xbdst= rom->read(aof_xbdst);
	      disass_xb(&aof_xbsrc, &work, comment, 1, xb_PC(xbsrc)?-1:0);
	      work.append(", ");
	      disass_xb(&aof_xbdst, &work, comment, 1, xb_PC(xbdst)?1:0);
	    }
	  if (strcmp(fmt.c_str(), "IMEX") == 0)
	    {
	      u16_t i= read_addr(rom, addr+2);
	      u16_t e= read_addr(rom, addr+4);
	      work.appendf("#$%04x", i);
	      work.append(", ");
	      work.appendf("$%04x", e);
	      comment->appendf("; [%04x]=%04x", e, read_addr(rom, e));
	    }
	  if (strcmp(fmt.c_str(), "imex") == 0)
	    {
	      u8_t i= rom->read(addr+2);
	      u16_t e= read_addr(rom, addr+3);
	      work.appendf("#$%02x", i);
	      work.append(", ");
	      work.appendf("$%04x", e);
	      comment->appendf("; [%04x]=%02x", e, rom->read(e));
	    }
	  if (strcmp(fmt.c_str(), "EXEX") == 0)
	    {
	      u16_t s= read_addr(rom, addr+2);
	      u16_t d= read_addr(rom, addr+4);
	      work.appendf("$%04x", s);
	      work.append(", ");
	      work.appendf("$%04x", d);
	      comment->appendf("; [%04x]=%04x [%04x]=%04x",
			       s, read_addr(rom, s),
			       d, read_addr(rom, d));
	    }
	  if (strcmp(fmt.c_str(), "exex") == 0)
	    {
	      u16_t s= read_addr(rom, addr+2);
	      u16_t d= read_addr(rom, addr+4);
	      work.appendf("$%04x", s);
	      work.append(", ");
	      work.appendf("$%04x", d);
	      comment->appendf("; [%04x]=%02x [%04x]=%02x",
			       s, rom->read(s),
			       d, rom->read(d));
	    }
	  if (strcmp(fmt.c_str(), "IDEX") == 0)
	    {
	      t_addr aof_xb= (addr+=2), adst;
	      u8_t h, l, xb= rom->read(aof_xb);
	      addr++;
	      h= rom->read(addr++);
	      l= rom->read(addr++);
	      disass_xb(&aof_xb, &work, comment, 2, xb_PC(xb)?-2:0);
	      work.append(", ");
	      work.appendf("$%04x", adst= h*256+l);
	      comment->appendf(" [%04x]=%04x", adst, read_addr(rom,adst));
	    }
	  if (strcmp(fmt.c_str(), "idex") == 0)
	    {
	      t_addr aof_xb= (addr+=2), adst;
	      u8_t h, l, xb= rom->read(aof_xb);
	      addr++;
	      h= rom->read(addr++);
	      l= rom->read(addr++);
	      disass_xb(&aof_xb, &work, comment, 1, xb_PC(xb)?-2:0);
	      work.append(", ");
	      work.appendf("$%04x", adst= h*256+l);
	      comment->appendf(" [%04x]=%02x", adst, rom->read(adst));
	    }
	  if (strcmp(fmt.c_str(), "em") == 0) // EMACS
	    {
	      u16_t a= read_addr(rom, addr+2);
	      u32_t m= (read_addr(rom, a)<<16) + (read_addr(rom, a+2));
	      work.appendf("$%04x", a);
	      comment->appendf("; [X=%04x]=%04x,%+d [Y=%04x]=%04x,%+d [%04x]=%08x,%+d",
			       rX, read_addr(rom, rX), (int)read_addr(rom, rX),
			       rY, read_addr(rom, rY), (int)read_addr(rom, rY),
			       a, m, int(m));
	    }
	  continue;
	}
      if (b[i] == '%')
	{
	  t_addr a= addr;
	  u8_t h, l;
	  i++;
	  temp= "";
	  switch (b[i])
	    {
	    case 'x': case 'X': // indexed
	      h= rom->read(addr+1);
	      a= rX+h;
	      work.appendf("$%02x,X", h);
	      //add_spaces(&work, 20);
	      if (b[i]=='x')
		temp.appendf("; [$%04x]=$%02x", a, rom->read(a));
	      else
		temp.appendf("; [$%04x]=$%04x", a, read_addr(rom, a));
	      break;
	    case 'e': case 'E': // extended
	      h= rom->read(addr+1);
	      l= rom->read(addr+2);
	      a= h*256 + l;
	      work.appendf("$%04x", a);
	      //add_spaces(&work, 20);
	      if (b[i]=='e')
		temp.appendf("; [$%04x]=$%02x", a, rom->read(a));
	      else
		temp.appendf("; [$%04x]=$%04x", a,
			     read_addr(rom, a));
	      break;
	    case 'd': case 'D': // direct
	      h= a= rom->read(addr+1);
	      work.appendf("$00%02x", h);
	      //add_spaces(&work, 20);
	      if (b[i]=='d')
		temp.appendf("; [$%04x]=$%02x", a, rom->read(a));
	      else
	        temp.appendf("; [$%04x]=$%04x", a,
			     read_addr(rom, a));
	      break;
	    case 'b': // immediate 8 bit
	      work.appendf("#$%02x",
			   rom->read(addr+1));
	      break;
	    case 'B': // immediate 16 bit
	      work.appendf("#$%04x",
			   read_addr(rom, addr+1));
	      break;
	    case 'r': // relative 8 bit offset
	      work.appendf("$%04x",
			   (addr+2+(i8_t)(rom->read(addr+1))) & 0xffff );
	      break;
	    case 'R': // relative 16 bit offset
	      {
		u16_t a= addr+4;
		i16_t r= rom->read(addr+2)*256;
		r+= rom->read(addr+3);
		a+= r;
		work.appendf("$%04x", a);
		break;
	      }
	    case 'p': case 'P':// xb postbyte for 8/16 bit operand
	      {
		t_addr a= addr+1;
		disass_xb(&a, &work, comment, isupper(b[i])?2:1);
		addr= a;
		break;
	      }
	    case 'T': // TFR/EXG
	      disass_b7(&a, &work, comment);
	    }
	  //work+= temp;
	  if (comment && temp.nempty())
	    comment->append(temp);
	}
      else
	work+= b[i];
    }

  return(strdup(work.c_str()));
}

const char *rr_names[4]= { "X", "Y", "SP", "PC" };

void
CL12::disass_xb(t_addr *addr, chars *work, chars *comment, int len, int corr, u32_t use_PC)
{
  u8_t p, h, l;
  i8_t n;
  int rr= -1;
  i16_t offset= 0;
  t_addr aof_xb= *addr;
  u16_t a;
  
  p= rom->read(aof_xb);
  (*addr)++;
  
  switch (xb_type(p))
    {
    case 1: // 1. rr0n nnnn n5,r rr={X,Y,SP,PC}
      rr= (p>>6)&3;
      offset= p&0x1f;
      if (p&0x10) offset|= 0xffe0;
      if (offset)
	work->appendf("%+d", offset);
      work->appendf(",%s", rr_names[rr]);
      break;
  
    case 6: // 6. 111r r111 [D,r] rr={X,Y,SP,PC}
      work->appendf("[D,%s]", rr_names[(p&0x18)>>3]);
      break;
  
    case 5: // 5. 111r r011 [n16,r] rr={X,Y,SP,PC}
      h= rom->read(aof_xb+1);
      l= rom->read(aof_xb+2);
      work->appendf("[$%04x,%s]", h*256+l,rr_names[(p&0x18)>>3]);
      break;

    case 3: // 3. rr1p nnnn n4,+-r+- rr={X,Y,SP}
      n= p&0xf;
      if (n&0x08) n|= 0xf0; else n++;
      if (p&0x10)
	{
	  // post +-
	  work->appendf("%+d,%s%c",
			(int)n,
			rr_names[(p&0xc0)>>6],
			(n<0)?'-':'+');
	}
      else
	{
	  // pre +-
	  work->appendf("%+d,%c%s",
			(int)n,
			(n<0)?'-':'+',
			rr_names[(p&0xc0)>>6]);
	}
      break;
  
    case 2: // 2. 111r r0zs n9/16,r rr={X,Y,SP,PC}
      if ((p&0x02) == 0x00)
	{
	  // 9 bit
	  offset= rom->read(aof_xb+1);
	  if (p&0x01) offset|= 0xff00;
	}
      else
	{
	  // 16 bit
	  h= rom->read(aof_xb+1);
	  l= rom->read(aof_xb+2);
	  offset= h*256+l;
	}
      work->appendf("$%+d,%s", offset, rr_names[(p&0x18)>>3]);
      break;
  
    default: // 4. 111r r1aa {A,B,D},r rr={X,Y,SP,PC}
      switch (p&0x03)
	{
	case 0x00: work->append("A,"); break;
	case 0x01: work->append("B,"); break;
	case 0x02: work->append("D,"); break;
	}
      work->appendf("%s", rr_names[(p&0x18)>>3]);
      break;
    }

  a= naddr(&aof_xb, NULL, use_PC);
  if (comment)
    {
      bool b= false;
      comment->append("; [");
      if (rr >= 0)
	comment->appendf("%s", rr_names[rr]), b= true;
      if (offset)
	comment->appendf("%+d", offset), b= true;
      if (b)
	comment->append("=");
      comment->appendf("%04x", a);
      if (corr)
	comment->appendf("%+d", corr);
      a+= corr;
      comment->appendf("]=$%02x%02x%02x",
		       rom->read(a), rom->read(a+1), rom->read(a+2));
    }
  *addr= aof_xb;
}

void
CL12::disass_b7(t_addr *addr, chars *work, chars *comment)
{
  bool spec= false;
  (*addr)++;
  u8_t pb= rom->read(*addr);
  if (pb & 0x08)
    work->append("TFR/EXG INVALID");
  else
    {
      if (pb == 0x02) work->append("TAP"), spec= true;
      if (pb == 0x20) work->append("TPA"), spec= true;
      if (pb == 0x75) work->append("TSX"), spec= true;
      if (pb == 0x76) work->append("TSY"), spec= true;
      if (pb == 0x57) work->append("TXS"), spec= true;
      if (pb == 0x67) work->append("TYS"), spec= true;
      if (pb == 0xc5) work->append("XGDX"), spec= true;
      if (pb == 0xc6) work->append("XGDY"), spec= true;
      if (spec) return;
      if (!(pb & 0x80))
	work->append("TFR");
      else
	work->append("EXG");
      while (work->len() < 7) work->append(' ');
      u8_t ls= pb&7, ms= (pb>>4)&7;
      const char *nd;
      nd= tex_names[ms];
      if (ms==3)
	nd= "TMP2";
      work->appendf("%s", nd);
      work->append(",");
      work->appendf("%s", tex_names[ls]);
    }
}

char *
CL12::disass_loop(t_addr *addr, chars *work, chars *comment)
{
  u8_t code;
  i16_t r;

  code= rom->read(*addr);
  *addr= *addr + 1;
  r= rom->read(*addr);
  *addr= *addr + 1;
  if ((code & 0xc0) == 0) work->append("D");
  else if ((code & 0xc0) == 0x40) work->append("T");
  else if ((code & 0xc0) == 0x80) work->append("I");
  else
    return strdup("-- invalid");
  work->append("B");
  if (code & 0x20)
    work->append("NE");
  else
    work->append("EQ");
  while (work->len() < 7) work->append(' ');

  work->append(loop_names[code & 0x7]);
  work->append(",");
  if (code & 0x10)
    r|= 0xff00;
  
  u16_t a= *addr + r;
  work->appendf("$%04x", a);
  
  return strdup(work->c_str());
}

int
CL12::inst_length(t_addr addr)
{
  u8_t code= rom->read(addr);
  if (code == 0x04)
    return 3;
  struct dis_entry *di= get_dis_entry(addr);
  if (di && di->mnemonic)
    {
      if (di->length >= 0)
	return di->length;
      int l= -(di->length);
      u16_t a= (u16_t)addr+l-1;
      u8_t p= rom->read(a);
      int corr= 0;
      if ((di->code == 0x4b)
	  &&
	  (!xb_indirect(p)))
	corr= 1;
      if ((p & 0x20) == 0)
	{
	  // 1. rr0n nnnn n5,r rr={X,Y,SP,PC}
	  return l+corr;
	}
      else if ((p&0xe7) == 0xe7)
	{
	  // 6. 111r r111 [D,r] rr={X,Y,SP,PC}
	  return l+corr;
	}
      else if ((p&0xe7) == 0xe3)
	{
	  // 5. 111r r011 [n16,r] rr={X,Y,SP,PC}
	  return l+2+corr;
	}
      else if ((p&0xc0) != 0xc0)
	{
	  // 3. rr1p nnnn n4,+-r+- rr={X,Y,SP}
	  return l+corr;
	}
      else if ((p&0xe4) == 0xe0)
	{
	  // 2. 111r r0zs n9/16,r rr={X,Y,SP,PC}
	  if ((p & 0x02) == 0)
	    return l+1+corr;
	  return l+2+corr;
	}
      else // if ((p&0xe4) == 0xe4)
	{
	  // 4. 111r r1aa {A,B,D},r rr={X,Y,SP,PC}
	  return l+corr;
	}
    }
  return 1;
}


/* End of m68hc12.src/dis.cc */
