/*
 * Simulator of microcontrollers (sim.src/port_hw.cc)
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
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

#include <stdio.h>
#include <ctype.h>

#include "globals.h"

#include "port_hwcl.h"


const char *keysets[8]= {
  "12345678",
  "qwertyui",
  "asdfghjk",
  "zxcvbnm,",
  "QWERTYUI",
  "ASDFGHJK",
  "ZXCVBNM,",
  "9opl.OPL"
};


cl_port_ui::cl_port_ui(class cl_uc *auc, int aid, chars aid_string):
  cl_hw(auc, HW_DUMMY, aid, aid_string)
{
  int i;
  
  for (i= 0; i < NUOF_PORT_UIS; i++)
    {
      pd[i].init();
      pd[i].cell_p= NULL;
      pd[i].cell_in= NULL;
      pd[i].cell_dir= NULL;
      pd[i].cache_p= 0;
      pd[i].cache_in= 0;
      pd[i].cache_dir= 0;
      pd[i].keyset= NULL;
    }
  act_port= -1;
}


bool
cl_port_ui::add_port(class cl_port_data *p, int nr)
{
  if (nr >= NUOF_PORT_UIS)
    return false;
  if (pd[nr].cell_p)
    return false;

  if ((pd[nr].cell_p= p->cell_p))
    pd[nr].cache_p= pd[nr].cell_p->get();
  if ((pd[nr].cell_in= p->cell_in))
    pd[nr].cache_in= pd[nr].cell_in->read();
  if ((pd[nr].cell_dir= p->cell_dir))
    pd[nr].cache_dir= pd[nr].cell_dir->read();
  pd[nr].keyset  = p->keyset;
  pd[nr].basx    = p->basx;
  pd[nr].basy    = p->basy;

  pd[nr].set_name(p->get_name());

  if (act_port < 0)
    act_port= nr;
  
  return true;
}


void
cl_port_ui::make_io()
{
  if (!io)
    {
      io= new cl_port_io(this);
      io->init();
      application->get_commander()->add_console(io);
    }
}


void
cl_port_ui::new_io(class cl_f *f_in, class cl_f *f_out)
{
  cl_hw::new_io(f_in, f_out);
  io->tu_mouse_on();
  io->dd_printf("\033[2 q");
  if (f_in)
    f_in->set_escape(true);
}


bool
cl_port_ui::proc_input(void)
{
  return cl_hw::proc_input();
}

bool
cl_port_ui::handle_input(int c)
{
  class cl_port_io *pio= (class cl_port_io *)io;
  int i;
  i8_t i8= c;

  if (i8 < 0)
    {
      //fprintf(stderr, "Port: spec key= %d\n", i8);
    }
  else
    {
      for (i= 0; i < NUOF_PORT_UIS; i++)
	{
	  if (pd[i].cell_p == NULL)
	    continue;
	  
	  if (pd[i].keyset != NULL)
	    {
	      int bit;
	      for (bit= 0; pd[i].keyset[bit]; bit++)
		if (pd[i].keyset[bit] == c)
		  {
		    t_mem m= pd[i].cell_in->read();
		    pd[i].cell_in->write(m ^ (1<<(7-bit)));
		    pio->tu_go(1,1);
		    return true;
		  }
	    }
	}
    }
  pio->tu_go(1,24);
  pio->tu_cll();
  int ret= cl_hw::handle_input(c); // handle default keys
  pio->tu_go(1,1);
  //pio->tu_cll();
  if (!ret)
    {
      //u8_t u= c;
      //fprintf(stderr, "Unknown command: %c (%d,0x%x)\n", isprint(u)?u:'?', i8, c);
    }
  return ret;
}

void
cl_port_ui::refresh_display(bool force)
{
  class cl_port_io *pio= (class cl_port_io *)io;
  if (!io)
    return;

  int i, m;
  bool pc= false, ic= false;
  pio->tu_hide();
  for (i= 0; i < NUOF_PORT_UIS; i++)
    {
      if (pd[i].cell_p == NULL)
	continue;
      // name
      pio->tu_go(pd[i].basx, pd[i].basy);
      pio->dd_printf("\033[%dm", (act_port == i)?7:0);
      if (pd[i].have_name())
	pio->dd_printf(pd[i].get_name());
      else
	pio->dd_printf("port_%d", i);

      if (pd[i].cell_dir)
	{
	  t_mem d= pd[i].cell_dir->get();
	  if (pd[i].cache_dir != d)
	    force= true;
	  pd[i].cache_dir= d;
	}
      
      pio->dd_printf("\033[0m");
      if (force ||
	  (pd[i].cell_p->get() != pd[i].cache_p))
	{
	  // Out
	  pd[i].cache_p= pd[i].cell_p->get();
	  pio->tu_go(pd[i].basx+4, pd[i].basy+1);
	  m= 0x80;
	  for ( ; m; m>>= 1)
	    {
	      char v= (pd[i].cache_p&m)?'*':'-';
	      if (pd[i].cell_dir != NULL)
		{
		  if ((pd[i].cache_dir & m) == 0)
		    v= '.';
		}
	      pio->dd_printf("%c", v);		
	    }
	  pio->tu_go(pd[i].basx+4+8+1, pd[i].basy+1);
	  pio->dd_printf("%02x", pd[i].cache_p);
	  pc= true;
	}
      if (force ||
	  (pd[i].cell_in &&
	   (pd[i].cell_in->get() != pd[i].cache_in)))
	{
	  // In
	  pd[i].cache_in= pd[i].cell_in->get();
	  pio->tu_go(pd[i].basx+4, pd[i].basy+3);
	  m= 0x80;
	  for ( ; m; m>>= 1)
	    pio->dd_printf("%c", (pd[i].cache_in&m)?'*':'-');
	  pio->tu_go(pd[i].basx+4+8+1, pd[i].basy+3);
	  pio->dd_printf("%02x", pd[i].cache_in);
	  ic= true;
	}
      if (force ||
	  ((pc || ic)/* &&
			pd[i].cell_dir == NULL*/))
	{
	  // port value on "Bits" line
	  int b, val, pval= pd[i].cache_in;
	  pio->tu_go(pd[i].basx+4, pd[i].basy+2);
	  for (b= 7; b>=0; b--)
	    {
	      m= 1<<b;
	      val= pd[i].cache_in & m;
	      if (pd[i].cell_dir == NULL)
		val&= (pd[i].cache_p & m);
	      //if (val)
		pio->dd_printf("\033[%dm", val?7:0);
		//else
		//pio->dd_printf("\033[0m");
	      pio->dd_printf("%d", b);
	    }
	  if (!pd[i].cell_dir)
	    pval&= pd[i].cache_p;
	  pio->dd_printf("\033[0m %02x", pval);
	}
    }
  pio->tu_show();
  cl_hw::refresh_display(force);
  if (act_port >= 0)
    pio->tu_go(pd[act_port].basx+4, pd[act_port].basy+1);
  else
    pio->tu_go(1, 1);
}

void
cl_port_ui::draw_display(void)
{
  class cl_port_io *pio= (class cl_port_io *)io;
  if (!io)
    return;
  pio->tu_cls();

  cl_hw::draw_display();
  
  int i;
  for (i= 0; i < NUOF_PORT_UIS; i++)
    {
      if (pd[i].cell_p == NULL)
	continue;
     
      pio->tu_go(pd[i].basx, pd[i].basy+1);
      pio->dd_printf("Out ");
      pio->tu_go(pd[i].basx, pd[i].basy+2);
      pio->dd_printf("Bit 76543210");
      pio->tu_go(pd[i].basx, pd[i].basy+3);
      pio->dd_printf("In  ");
      pio->tu_go(pd[i].basx, pd[i].basy+4);
      if (pd[i].keyset)
	pio->dd_printf("Key %s", pd[i].keyset);

      pd[i].cache_p= pd[i].cell_p->get();
      pd[i].cache_in= pd[i].cell_in->read();
    }
  
  refresh_display(true);
}


/* IO console for port display */

cl_port_io::cl_port_io(class cl_hw *ihw):
  cl_hw_io(ihw)
{
}

int
cl_port_io::init(void)
{
  cl_hw_io::init();
  return 0;
}

/*
bool
cl_port_io::input_avail(void)
{
  if (hw)
    hw->refresh_display(false);
  return cl_console::input_avail();
}
*/

/* End of sim.src/port_hw.cc */
