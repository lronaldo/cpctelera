/*
 * Simulator of microcontrollers (disass.cc)
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

#include "f8cl.h"


struct dis_entry *
cl_f8::dis_tbl(void)
{
  return(disass_f8);
}

struct dis_entry *
cl_f8::get_dis_entry(t_addr addr)
{
  t_mem code= rom->get(addr);
  int i= 0;
  while (code == PREF_SWAPOP || code == PREF_ALT1 || code == PREF_ALT2 || code == PREF_ALT3 || code == PREF_ALT4 || code == PREF_ALT5)
    {
      i++;
      code= rom->get(addr+i);
    }
  for (struct dis_entry *de = disass_f8; de && de->mnemonic; de++)
    {
      if ((code & de->mask) == de->code)
        return de;
    }

  return NULL;
}

int
cl_f8::inst_length(t_addr addr)
{
  t_addr a= addr;
  int s= 0;
  u8_t c= rom->get(a);
  while (c == PREF_SWAPOP || c == PREF_ALT1 || c == PREF_ALT2 || c == PREF_ALT3 || c == PREF_ALT4 || c == PREF_ALT5)
    {
      s++;
      c= rom->get(++a);
    }
  struct dis_entry *de= get_dis_entry(a);
  if (de == NULL)
    return 1 + s;
  return de->length + s;
}

u8_t
cl_f8::a8(u8_t prefs)
{
  if (prefs & P_ALT1)
    return rXH;
  else if (prefs & P_ALT2)
    return rYL;
  else if (prefs & P_ALT3)
    return rZL;
  else if (prefs & P_ALT4)
    return rYH;
  else if (prefs & P_ALT5)
    return rZH;
  return rXL;
}

u16_t
cl_f8::a16(u8_t prefs)
{
  if (prefs & P_ALT3)
    return rX;
  else if (prefs & (P_ALT2 | P_ALT4))
    return rZ;
  return rY;
}

const char *
cl_f8::a8_name(u8_t prefs)
{
  if (prefs & P_ALT1)
    return "xh";
  else if (prefs & P_ALT2)
    return "yl";
  else if (prefs & P_ALT3)
    return "zl";
  else if (prefs & P_ALT4)
    return "yh";
  else if (prefs & P_ALT5)
    return "zh";
  return "xl";
}

const char *
cl_f8::a16_name(u8_t prefs)
{
  if (prefs & (P_ALT2 | P_ALT4))
    return "z";
  else if (prefs & P_ALT3)
    return "x";
  return "y";
}

const char *
cl_f8::r16_name(u8_t prefs)
{
  if (prefs & (P_ALT2))
    return "y";
  else if (prefs & (P_ALT3 | P_ALT5))
    return "z";
  return "x";
}

const char *
cl_f8::a16h_name(u8_t prefs)
{
  if (prefs & (P_ALT2 | P_ALT4))
    return "zh";
  else if (prefs & P_ALT3)
    return "xh";
  return "yh";
}

const char *
cl_f8::a16l_name(u8_t prefs)
{
  if (prefs & (P_ALT2 | P_ALT4))
    return "zl";
  else if (prefs & P_ALT3)
    return "xl";
  return "yl";
}

char *
cl_f8::disassc(t_addr addr, chars *comment)
{
  chars work= chars(), temp= chars(), fmt= chars();
  chars words[10];
  chars *word= NULL;
  int wi= 0;
  const char *b;
  struct dis_entry *de;
  int i, prefs= P_NONE;
  bool first;
  u8_t h, l, /*r,*/ code;
  u16_t a, nn;
  i16_t d;
  
  code= rom->read(addr);
  while (code == PREF_SWAPOP || code == PREF_ALT1 || code == PREF_ALT2 || code == PREF_ALT3 || code == PREF_ALT4 || code == PREF_ALT5)
    {
      switch (code) {
      case PREF_SWAPOP:
        prefs = P_SWAP;
        break;
      case PREF_ALT1:
        prefs = P_ALT1;
        break;
      case PREF_ALT2:
        prefs = P_ALT2;
        break;
      case PREF_ALT3:
        prefs = P_ALT3;
        break;
      case PREF_ALT4:
        prefs = P_ALT4;
        break;
      case PREF_ALT5:
        prefs = P_ALT5;
        break;
      }
      code= rom->read(++addr);
    }
  de= get_dis_entry(addr);
  //code= rom->read(addr);
  
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
	  word= &words[wi=1];
	}
      else if (b[i] == ',')
	{
	  word= &words[++wi];
	}
      else if (b[i] == '\'')
	{
	  fmt= "";
	  i++;
	  while (b[i] && (b[i]!='\''))
	    fmt.append(b[i++]);
	  if (!b[i]) i--;
	  if (fmt.empty())
	    word->append("'");
	  if (strcmp(fmt.c_str(), "i8") == 0)
	    {
	      word->appendf("0x%02x", rom->read(addr+1));
	    }
	  if (strcmp(fmt.c_str(), "i16") == 0)
	    {
	      word->appendf("0x%04x", read_addr(rom, addr+1));
	    }
	  if (strcmp(fmt.c_str(), "a16") == 0)
	    {
	      a= read_addr(rom, addr+1);
	      word->appendf("0x%04x", a);
	    }
	  if (strcmp(fmt.c_str(), "a16_8") == 0)
	    {
	      a= read_addr(rom, addr+1);
	      word->appendf("0x%04x", a);
	      comment->appendf("; [0x%04x]= 0x%02x", a, rom->read(a));
	    }
	  if (strcmp(fmt.c_str(), "a16_16") == 0)
	    {
	      a= read_addr(rom, addr+1);
	      word->appendf("0x%04x", a);
	      comment->appendf("; [0x%04x]= 0x%04x", a, read_addr(rom, a));
	    }
	  if (strstr(fmt.c_str(), "a16_b") == fmt.c_str())
	    {
	      char bc= fmt.c_str()[5];
	      int b= 0;
	      if (bc && bc>='0' && bc<='7') b= bc-'0';
	      a= read_addr(rom, addr+1);
	      word->appendf("0x%04x", a);
	      l= rom->read(a);
	      h= l&(1<<b);
	      comment->appendf("; [0x%04x]= 0x%02x.%d=%d", a, l, b, h?1:0);
	    }
	  if (strcmp(fmt.c_str(), "nsp_8") == 0)
	    {
	      l= rom->read(addr+1);
	      a= rSP+l;
	      a&= 0xffff;
	      word->appendf("0x%02x,sp", l);
	      comment->appendf("; [0x%04x]= 0x%02x", a, rom->read(a));
	    }
	  if (strcmp(fmt.c_str(), "nsp_16") == 0)
	    {
	      l= rom->read(addr+1);
	      a= rSP+l;
	      a&= 0xffff;
	      word->appendf("0x%02x,sp", l);
	      comment->appendf("; [0x%04x]= 0x%04x", a, read_addr(rom,a));
	    }
	  if (strcmp(fmt.c_str(), "nnz_8") == 0)
	    {
	      nn= read_addr(rom, addr+1);
	      a= nn+rZ;
	      a&= 0xffff;
	      word->appendf("0x%04x,z", nn);
	      comment->appendf("; [0x%04x]= 0x%02x", a, rom->read(a));
	    }
	  if (strcmp(fmt.c_str(), "nnz_16") == 0)
	    {
	      nn= read_addr(rom, addr+1);
	      a= nn+rZ;
	      a&= 0xffff;
	      word->appendf("0x%04x,z", nn);
	      comment->appendf("; [0x%04x]= 0x%04x", a, read_addr(rom,a));
	    }
	  if (strcmp(fmt.c_str(), "z_8") == 0)
	    {
	      a= rZ;
	      a&= 0xffff;
	      word->appendf("z");
	      comment->appendf("; [0x%04x]= 0x%02x", a, rom->read(a));
	    }
	  if (strcmp(fmt.c_str(), "z_16") == 0)
	    {
	      a= rZ;
	      a&= 0xffff;
	      word->appendf("z");
	      comment->appendf("; [0x%04x]= 0x%04x", a, read_addr(rom,a));
	    }
	  if (strcmp(fmt.c_str(), "y_8") == 0)
	    {
	      a= rY;
	      word->appendf("y");
	      comment->appendf("; [0x%04x]= 0x%02x", a, rom->read(a));
	    }
	  if (strcmp(fmt.c_str(), "y_16") == 0)
	    {
	      a= rY;
	      word->appendf("y");
	      comment->appendf("; [0x%04x]= 0x%04x", a, read_addr(rom,a));
	    }
	  if (strcmp(fmt.c_str(), "ny_8") == 0)
	    {
	      l= rom->read(addr+1);
	      a= rY+l;
	      a&= 0xffff;
	      word->appendf("0x%02x,y", l);
	      comment->appendf("; [0x%04x]= 0x%02x", a, rom->read(a));
	    }
	  if (strcmp(fmt.c_str(), "ny_16") == 0)
	    {
	      l= rom->read(addr+1);
	      a= rY+l;
	      a&= 0xffff;
	      word->appendf("0x%02x,sp", l);
	      comment->appendf("; [0x%04x]= 0x%04x", a, read_addr(rom,a));
	    }
	  if (strcmp(fmt.c_str(), "nA_16") == 0)
	    {
	      l= rom->read(addr+1);
	      a= a16(prefs)+l;
	      a&= 0xffff;
	      word->appendf("0x%02x,%s", l, a16_name(prefs));
	      comment->appendf("; [0x%04x]= 0x%04x", a, read_addr(rom,a));
	    }
	  if (strcmp(fmt.c_str(), "nnA_16") == 0)
	    {
	      nn= read_addr(rom, addr+1);
	      a= a16(prefs)+nn;
	      a&= 0xffff;
	      word->appendf("0x%04x,%s", nn, a16_name(prefs));
	      comment->appendf("; [0x%04x]= 0x%04x", a, read_addr(rom,a));
	    }
	  if (strcmp(fmt.c_str(), "dsp_16") == 0)
	    {
	      d= rom->read(addr+1);
	      if (d&0x80) d|= 0xff00;
	      a= rSP+d;
	      a&= 0xffff;
	      word->appendf("+%d,sp", d);
	      comment->appendf("; [0x%04x]= 0x%04x", a, read_addr(rom,a));
	    }

	  continue;
	}
      else if (b[i] == '%')
	{
	  i++;
	  temp= "";
	  switch (b[i])
	    {
	    case 'a': // 8 bit accumulator, selected by prefix
	      word->append(a8_name(prefs));
	      break;

	    case 'A': // 16 bit accumulator, selected by prefix
	      word->append(a16_name(prefs));
	      break;

            case 'R': // 16 bit register, corresponding to 8 bit accumulator, selected by prefix
	      word->append(r16_name(prefs));
	      break;

	    case 'H': // upper half of 16 bit accumulator, selected by prefix
	      word->append(a16h_name(prefs));
	      break;

	    case 'L': // lower half of 16 bit accumulator, selected by prefix
	      word->append(a16l_name(prefs));
	      break;

	    case 'd':
	      d= rom->read(addr+1);
	      if (d&0x80) d|= 0xff00;
	      word->appendf("%+d", d);
	      break;

	    case 'r':
	      d= rom->read(addr+1);
	      if (d&0x80) d|= 0xff00;
	      a= addr+2+d;
	      word->appendf("0x%04x", a);
	      break;
	      
	    }
	  if (comment && temp.nempty())
	    comment->append(temp);
	}
      else
	{
	  if (word == NULL)
	      work+= b[i];
	  else
	    word->append(b[i]);
	}
    }

  if (prefs & ~allowed_prefs[code])
    work+= '!';
  prefs&= allowed_prefs[code];
  if (prefs & P_SWAP)
    {
      chars t;
      t= words[1];
      words[1]= words[2];
      words[2]= t;
    }
  for (l= 1; l<=wi; l++)
    {
      if (l>1)
	work.append(",");
      work.append(words[l]);
    }
  return(strdup(work.c_str()));
}


/* End of f8.src/disass.cc */
