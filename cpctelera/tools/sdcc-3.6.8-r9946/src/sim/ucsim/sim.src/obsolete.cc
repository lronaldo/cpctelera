/*
 * Memory location handled specially by a hw element
 */

/*cl_memloc::cl_memloc(t_addr addr):
  cl_base()
{
  address= addr;
  hws= new cl_list(2, 2);
  hws->init();
}*/

/*cl_memloc::~cl_memloc(void)
{
  hws->disconn_all();
  delete hws;
}*/

/*ulong
cl_memloc::read(class cl_mem *mem)
{
  uchar ret= 0;
  class cl_hw *hw;

  if (!hws ||
      hws->count == 0)
    return(ret);
  if ((hw= (class cl_hw *)(hws->at(0))))
    ret= hw->read(mem, address);
  return(ret);
}*/

/*void
cl_memloc::write(class cl_mem *mem, t_addr addr, t_mem *val)
{
  class cl_hw *hw;
  int i;

  if (!hws)
    return;
  for (i= 0; i < hws->count; i++)
    {
      hw= (class cl_hw *)hws->at(0);
      hw->write(mem, addr, val);
    }
}*/


/* Sorted collection of memory locations */

/*cl_memloc_coll::cl_memloc_coll(void):
  cl_sorted_list(2, 2)
{
  Duplicates= DD_FALSE;
}*/

/*void *
cl_memloc_coll::key_of(void *item)
{
  return(&(((class cl_memloc *)item)->address));
}*/

/*int
cl_memloc_coll::compare(void *key1, void *key2)
{
  if (*(long*)key1 > *(long*)key2)
    return(1);
  else
    if (*(long*)key1 < *(long*)key2)
      return(-1);
    else
      return(0);
}*/

/*class cl_memloc *
cl_memloc_coll::get_loc(t_addr address)
{
  t_index i;

  if (search(&address, i))
    return((class cl_memloc*)(at(i)));
  return(0);
}*/


/*
 * Memory
 ******************************************************************************
 */

/*
 * Bitmap
 */

/*cl_bitmap::cl_bitmap(t_addr asize):
  cl_base()
{
  map= (uchar*)malloc(size= asize/(8*SIZEOF_CHAR));
  memset(map, 0, size);
}

cl_bitmap::~cl_bitmap(void)
{
  free(map);
}

void
cl_bitmap::set(t_addr pos)
{
  int i;

  if ((i= pos/(8*SIZEOF_CHAR)) < size)
    map[i]|= (1 << (pos & ((8*SIZEOF_CHAR)-1)));
}

void
cl_bitmap::clear(t_addr pos)
{
  int i;

  if ((i= pos/(8*SIZEOF_CHAR)) < size)
    map[i]&= ~(1 << (pos & ((8*SIZEOF_CHAR)-1)));
}

bool
cl_bitmap::get(t_addr pos)
{
  return(map[pos/(8*SIZEOF_CHAR)] & (1 << (pos & ((8*SIZEOF_CHAR)-1))));
}

bool
cl_bitmap::empty(void)
{
  int i;

  for (i= 0; i < size && map[i] == 0; i++) ;
  return(i == size);
}*/

/*
 * Special memory for code (ROM)
 */

/*cl_rom::cl_rom(t_addr asize, int awidth, class cl_uc *auc):
  cl_mem(MEM_ROM, get_id_string(mem_classes, MEM_ROM), asize, awidth, auc)
{
  bp_map= new cl_bitmap(asize);
  inst_map= new cl_bitmap(asize);
}

cl_rom::~cl_rom(void)
{
  delete bp_map;
  delete inst_map;
}*/


cl_mem::cl_mem(enum mem_class atype, char *aclass_name,
	       t_addr asize, int awidth, class cl_uc *auc):
  cl_guiobj()
{
  int i;
  
  uc= auc;
  type= atype;
  class_name= aclass_name;
  width= awidth;
  size= asize;
  mem= 0;
  for (i= width, mask= 0; i; i--)
    mask= (mask<<1) | 1;
  if (width == 0 ||
      size == 0)
    mem= 0;
  else if (width <= 8)
    mem= (TYPE_UBYTE *)malloc(size);
  else if (width <= 16)
    mem= (TYPE_UWORD *)malloc(size*sizeof(TYPE_WORD));
  else
    mem= (TYPE_UDWORD *)malloc(size*sizeof(TYPE_DWORD));
  //read_locs= new cl_memloc_coll();
  //write_locs= new cl_memloc_coll();
  dump_finished= 0;
  addr_format= data_format= 0;
}

cl_mem::~cl_mem(void)
{
  if (mem)
    free(mem);
  if (addr_format)
    free(addr_format);
  if (data_format)
    free(data_format);
  //delete read_locs;
  //delete write_locs;
}

int
cl_mem::init(void)
{
  t_addr i;

  addr_format= (char *)malloc(10);
  sprintf(addr_format, "0x%%0%dx",
	  size-1<=0xf?1:
	  (size-1<=0xff?2:
	   (size-1<=0xfff?3:
	    (size-1<=0xffff?4:
	     (size-1<=0xfffff?5:
	      (size-1<=0xffffff?6:12))))));
  data_format= (char *)malloc(10);
  sprintf(data_format, "%%0%dx", width/4+((width%4)?1:0));

  for (i= 0; i < size; i++)
    set(i, (type==MEM_ROM)?(-1):0);
  return(0);
}

char *
cl_mem::id_string(void)
{
  char *s= get_id_string(mem_ids, type);

  return(s?s:(char*)"NONE");
}

t_mem
cl_mem::read(t_addr addr)
{
  //class cl_memloc *loc;

  if (addr >= size)
    {
      //FIXME
      fprintf(stderr, "Address 0x%06x is over 0x%06x\n",
	      (int)addr, (int)size);
      return(0);
    }
  /*if ((loc= read_locs->get_loc(addr)))
    return(loc->read(this));*/
  if (width <= 8)
    return((((TYPE_UBYTE*)mem)[addr])&mask);
  else if (width <= 16)
    return((((TYPE_UWORD*)mem)[addr])&mask);
  else
    return((((TYPE_UDWORD*)mem)[addr])&mask);
}

t_mem
cl_mem::get(t_addr addr)
{
  if (addr >= size)
    return(0);
  if (width <= 8)
    return((((TYPE_UBYTE*)mem)[addr])&mask);
  else if (width <= 16)
    return((((TYPE_UWORD*)mem)[addr])&mask);
  else
    return((((TYPE_UDWORD*)mem)[addr])&mask);
}


/*
 * Modify memory location
 */

/* Write calls callbacks of HW elements */

t_mem
cl_mem::write(t_addr addr, t_mem val)
{
  /*  class cl_memloc *loc;

  if (addr >= size)
    return;
  if ((loc= write_locs->get_loc(addr)))
    loc->write(this, addr, val);
  if (width <= 8)
    ((TYPE_UBYTE*)mem)[addr]= (*val)&mask;
  else if (width <= 16)
    ((TYPE_UWORD*)mem)[addr]= (*val)&mask;
  else
  ((TYPE_UDWORD*)mem)[addr]= (*val)&mask;*/
  fprintf(stderr, "FIXME cl_mem::write(0x%06x, 0x%04x)\n",
	  (int)addr, (int)val);
  return(0);
}

/* Set doesn't call callbacks */

void
cl_mem::set(t_addr addr, t_mem val)
{
  if (addr >= size)
    return;
  if (width <= 8)
    ((TYPE_UBYTE*)mem)[addr]= val&mask;
  else if (width <= 16)
    ((TYPE_UWORD*)mem)[addr]= val&mask;
  else
    ((TYPE_UDWORD*)mem)[addr]= val&mask;
}

t_mem
cl_mem::add(t_addr addr, long what)
{
  if (addr >= size)
    return(0);
  if (width <= 8)
    {
      ((TYPE_UBYTE*)mem)[addr]= ((TYPE_UBYTE*)mem)[addr] + what;
      return(((TYPE_UBYTE*)mem)[addr]);
    }
  else if (width <= 16)
    {
      ((TYPE_UWORD*)mem)[addr]= ((TYPE_UWORD*)mem)[addr] + what;
      return(((TYPE_UWORD*)mem)[addr]);
    }
  else
    {
      ((TYPE_UDWORD*)mem)[addr]= ((TYPE_UDWORD*)mem)[addr] + what;
      return(((TYPE_UDWORD*)mem)[addr]);
    }
}

t_addr
cl_mem::dump(t_addr start, t_addr stop, int bpl, class cl_console *con)
{
  int i;

  while ((start <= stop) &&
	 (start < size))
    {
      con->dd_printf(addr_format, start); con->dd_printf(" ");
      for (i= 0;
	   (i < bpl) &&
	     (start+i < size) &&
	     (start+i <= stop);
	   i++)
	{
	  con->dd_printf(data_format, /*read*/get(start+i)); con->dd_printf(" ");
	}
      while (i < bpl)
	{
	  int j;
	  j= width/4 + ((width%4)?1:0) + 1;
	  while (j)
	    {
	      con->dd_printf(" ");
	      j--;
	    }
	  i++;
	}
      for (i= 0; (i < bpl) &&
	     (start+i < size) &&
	     (start+i <= stop);
	   i++)
	{
	  long c= get(start+i);
	  con->dd_printf("%c", isprint(255&c)?(255&c):'.');
	  if (width > 8)
	    con->dd_printf("%c", isprint(255&(c>>8))?(255&(c>>8)):'.');
	  if (width > 16)
	    con->dd_printf("%c", isprint(255&(c>>16))?(255&(c>>16)):'.');
	  if (width > 24)
	    con->dd_printf("%c", isprint(255&(c>>24))?(255&(c>>24)):'.');
	}
      con->dd_printf("\n");
      dump_finished= start+i;
      start+= bpl;
    }
  return(dump_finished);
}

t_addr
cl_mem::dump(class cl_console *con)
{
  return(dump(dump_finished, dump_finished+10*8-1, 8, con));
}



/*
 */
/*
cl_mapped_cell::cl_mapped_cell(class cl_cell *realcell)
{
  real_cell= realcell;
}

cl_mapped_cell::~cl_mapped_cell(void)
{}

t_mem
cl_mapped_cell::read(void)
{
  return(real_cell->read());
}

t_mem
cl_mapped_cell::read(enum hw_cath skip)
{
  return(real_cell->read(skip));
}

t_mem
cl_mapped_cell::get(void)
{
  return(real_cell->get());
}

t_mem
cl_mapped_cell::write(t_mem val)
{
  return(real_cell->write(val));
}

t_mem
cl_mapped_cell::set(t_mem val)
{
  return(real_cell->set(val));
}

t_mem
cl_mapped_cell::add(long what)
{
  return(real_cell->add(what));
}

t_mem
cl_mapped_cell::wadd(long what)
{
  return(real_cell->wadd(what));
}

void
cl_mapped_cell::set_bit1(t_mem bits)
{
  return(real_cell->set_bit1(bits));
}

void
cl_mapped_cell::set_bit0(t_mem bits)
{
  return(real_cell->set_bit0(bits));
}

class cl_cell *
cl_mapped_cell::add_hw(class cl_hw *hw, int *ith)
{
  return(real_cell->add_hw(hw, ith));
}

class cl_hw *
cl_mapped_cell::get_hw(int ith)
{
  return(real_cell->get_hw(ith));
}

class cl_event_handler *
cl_mapped_cell::get_event_handler(void)
{
  return(real_cell->get_event_handler());
}
*/

/*
 */

cl_m::cl_m(enum mem_class atype, char *aclass_name, t_addr asize, int awidth,
	   class cl_uc *auc):
  cl_memory(aclass_name, asize, awidth)
  //cl_mem(atype, aclass_name, 0, awidth, auc)
{
  t_addr a;

  //size= asize;
  width= awidth;
  set_name(aclass_name);
  uc= auc;
  type= atype;

  array= (class cl_cell **)calloc(size, sizeof(class cl_cell *));
  for (a= 0; a < size; a++)
    array[a]= new cl_normal_cell(width);
  bus_mask= 0;
  t_addr i;
  for (i= 1; i < size; i<<=1)
    bus_mask= (bus_mask<<1)|1;
  dummy= new cl_normal_cell(width);
  //mk_cell(size, 0);
}

cl_m::~cl_m(void)
{
  t_addr a;

  for (a= 0; a < size; a++)
    delete array[a];
  free(array);
  delete dummy;
}

int
cl_m::init(void)
{
  t_addr i;

  cl_memory::init();

  for (i= 0; i < size; i++)
    set(i, (type==MEM_ROM)?(-1):0);
  return(0);
}

char *
cl_m::id_string(void)
{
  char *s= get_id_string(mem_ids, type);

  return(s?s:(char*)"NONE");
}

/*void
cl_m::mk_cell(t_addr addr, class cl_cell *cell)
{
  if (!cell)
    cell= new cl_cell(width);
  class cl_cell *p;
  if (addr >= size)
    p= dummy;
  else
    p= array[addr];
  if (p == 0)
    {
      p= (class cl_cell *)calloc(1, sizeof(*cell));
    }
  else
    {
      p->destroy();
      p= (class cl_cell *)realloc(p, sizeof(cell));
    }
  memcpy(p, cell, sizeof(*cell));
  cell->destroy();
  delete cell;
}*/

t_mem
cl_m::read(t_addr addr)
{
  //addr&= bus_mask;
  if (addr >= size)
    {
      err_inv_addr(addr);
      return(dummy->read());
    }
  return(array[addr]->read());
}

t_mem
cl_m::read(t_addr addr, enum hw_cath skip)
{
  //addr&= bus_mask;
  if (addr >= size)
    {
      err_inv_addr(addr);
      return(dummy->read(skip));
    }
  return(array[addr]->read(skip));
}

t_mem
cl_m::get(t_addr addr)
{
  addr&= bus_mask;
  if (addr >= size)
    {
      err_inv_addr(addr);
      return(dummy->get());
    }
  return(array[addr]->get());
}

t_mem
cl_m::write(t_addr addr, t_mem val)
{
  //addr&= bus_mask;
  if (addr >= size)
    {
      err_inv_addr(addr);
      return(dummy->write(val));
    }
  return(array[addr]->write(val));
}

void
cl_m::set(t_addr addr, t_mem val)
{
  if (addr >= size)
    {
      err_inv_addr(addr);
      //addr&= bus_mask;
      dummy->set(val);
      return;
    }
  //addr&= bus_mask;
  array[addr]->set(val);
}

class cl_cell *
cl_m::get_cell(t_addr addr)
{
  //addr&= bus_mask;
  if (addr >= size)
    {
      err_inv_addr(addr);
      return(dummy);
    }
  return(array[addr]);
}


/* Set or clear bits, without callbacks */

void
cl_m::set_bit1(t_addr addr, t_mem bits)
{
  class cl_cell *cell;

  addr&= bus_mask;
  if (addr >= size)
    {
      err_inv_addr(addr);
      cell= dummy;
    }
  else
    cell= array[addr];
  bits&= cell->get_mask();
  cell->set(cell->get() | bits);
}

void
cl_m::write_bit1(t_addr addr, t_mem bits)
{
  class cl_cell *cell;

  addr&= bus_mask;
  if (addr >= size)
    {
      err_inv_addr(addr);
      cell= dummy;
    }
  else
    cell= array[addr];
  bits&= cell->get_mask();
  cell->write(cell->get() | bits);
}

void
cl_m::set_bit0(t_addr addr, t_mem bits)
{
  class cl_cell *cell;

  addr&= bus_mask;
  if (addr >= size)
    {
      err_inv_addr(addr);
      cell= dummy;
    }
  else
    cell= array[addr];
  bits&= cell->get_mask();
  cell->set(cell->get() & ~bits);
}

void
cl_m::write_bit0(t_addr addr, t_mem bits)
{
  class cl_cell *cell;

  addr&= bus_mask;
  if (addr >= size)
    {
      err_inv_addr(addr);
      cell =dummy;
    }
  else
    cell= array[addr];
  bits&= cell->get_mask();
  cell->write(cell->get() & ~bits);
}

t_mem
cl_m::add(t_addr addr, long what)
{
  addr&= bus_mask;
  if (addr >= size)
    {
      err_inv_addr(addr);
      return(dummy->add(what));
    }
  return(array[addr]->add(what));
}

t_mem
cl_m::wadd(t_addr addr, long what)
{
  addr&= bus_mask;
  if (addr >= size)
    {
      err_inv_addr(addr);
      return(dummy->wadd(what));
    }
  return(array[addr]->wadd(what));
}

bool
cl_m::search_next(bool case_sensitive, t_mem *array, int len, t_addr *addr)
{
  t_addr a;
  int i;
  bool found;

  if (addr == NULL)
    a= 0;
  else
    a= *addr;
  
  if (a+len > size)
    return(DD_FALSE);

  found= DD_FALSE;
  while (!found &&
	 a+len <= size)
    {
      bool match= DD_TRUE;
      for (i= 0; i < len && match; i++)
	{
	  t_mem d1, d2;
	  d1= get(a+i);
	  d2= array[i];
	  if (!case_sensitive)
	    {
	      if (/*d1 < 128*/isalpha(d1))
		d1= toupper(d1);
	      if (/*d2 < 128*/isalpha(d2))
		d2= toupper(d2);
	    }
	  match= d1 == d2;
	}
      found= match;
      if (!found)
	a++;
    }

  if (addr)
    *addr= a;
  return(found);
}

class cl_cell *
cl_m::register_hw(t_addr addr, class cl_hw *hw, int *ith, bool announce)
{
  class cl_cell *cell, *nc;

  addr&= bus_mask;
  if (addr >= size)
    cell= dummy;
  else
    cell= array[addr];

  if (cell->get_type() & (CELL_HW_READ | CELL_HW_WRITE))
    {
      /* Already registered */
      return(cell->add_hw(hw, ith));
    }
  else if (cell->get_type() & (CELL_READ_BRK | CELL_WRITE_BRK))
    {
      /* Event break is set on it, now register hw */
      nc= new cl_ev_reg_cell(width, uc);
      nc->set(cell->get());
      nc->set_type(nc->get_type() &
		   ~(CELL_GENERAL|CELL_READ_BRK|CELL_WRITE_BRK));
      nc->set_type(nc->get_type() | (cell->get_type() & CELL_GENERAL));
      class cl_event_handler *eh= nc->get_event_handler();
      if (eh)
	nc->set_type(nc->get_type() | eh->copy_from(cell->get_event_handler()));
      nc->add_hw(hw, ith);
    }
  else
    {
      /* Normal cell, register hw */
      nc= new cl_registered_cell(width);
      nc->set(cell->get());
      nc->set_type(nc->get_type() & ~CELL_GENERAL);
      nc->set_type(nc->get_type() | (cell->get_type() & CELL_GENERAL));
      nc->add_hw(hw, ith);
    }

  if (addr >= size)
    {
      delete dummy;
      dummy= nc;  
    }
  else
    {
      delete array[addr];
      array[addr]= nc;
    }
  if (announce)
    uc->sim->/*app->*/mem_cell_changed(this, addr);
  return(nc);
}

void
cl_m::set_brk(t_addr addr, class cl_brk *brk)
{
  class cl_cell *cell, *nc;
  char e= '_';

  addr&= bus_mask;
  if (addr >= size)
    cell= dummy;
  else
    cell= array[addr];

  switch (brk->get_event())
    {
    case brkWRITE: case brkWXRAM: case brkWIRAM: case brkWSFR:
      e= 'W';
      break;
    case brkREAD: case brkRXRAM: case brkRCODE: case brkRIRAM: case brkRSFR:
      e= 'R';
      break;
    case brkNONE:
      set_cell_flag(addr, DD_TRUE, CELL_FETCH_BRK);
      return;
      break;
    default: e= '.'; break;	  
    }
  
  if (cell->get_type() & (CELL_HW_READ | CELL_HW_WRITE))
    {
      /* Hw is registered on it, now set event break */
      nc= new cl_ev_reg_cell(width, uc);
      nc->set(cell->get());
      nc->set_type(nc->get_type() & ~CELL_GENERAL);
      nc->set_type(nc->get_type() | (cell->get_type() & CELL_GENERAL));
      int i= 0;
      class cl_hw *hw;
      while ((hw= cell->get_hw(i)) != 0)
	{
	  nc->add_hw(hw, 0);
	  i++;
	}
      if (((class cl_registered_cell *)cell)->hardwares)
	{
	  free(((class cl_registered_cell *)cell)->hardwares);
	  ((class cl_registered_cell *)cell)->hardwares= 0;
	}
      class cl_event_handler *eh;
      if ((eh= nc->get_event_handler()))
	nc->set_type(nc->get_type() | eh->add_bp(brk));
    }
  else if (cell->get_type() & (CELL_READ_BRK | CELL_WRITE_BRK))
    {
      /* Break is already set on it */
      class cl_event_handler *eh;
      if ((eh= cell->get_event_handler()))
	cell->set_type(cell->get_type() | eh->add_bp(brk));
      return;
    }
  else
    {
      /* Normal cell, set event break */
      nc= new cl_event_cell(width, uc);
      nc->set(cell->get());
      nc->set_type(nc->get_type() & ~CELL_GENERAL);
      nc->set_type(nc->get_type() | (cell->get_type() & CELL_GENERAL));
      class cl_event_handler *eh;
      if ((eh= nc->get_event_handler()))
	nc->set_type(nc->get_type() | eh->add_bp(brk));
    }

  if (addr >= size)
    {
      delete dummy;
      dummy= nc;
    }
  else
    {
      delete array[addr];
      array[addr]= nc;
    }
  uc->sim->/*app->*/mem_cell_changed(this, addr);
}

void
cl_m::del_brk(t_addr addr, class cl_brk *brk)
{
  class cl_cell *cell, *nc;
  char e= '_';

  addr&= bus_mask;
  if (addr >= size)
    cell= dummy;
  else
    cell= array[addr];

  switch (brk->get_event())
    {
    case brkWRITE: case brkWXRAM: case brkWIRAM: case brkWSFR: e= 'W'; break;
    case brkREAD: case brkRXRAM: case brkRCODE: case brkRIRAM: case brkRSFR:
      e= 'R';
      break;
    case brkNONE:
      set_cell_flag(addr, DD_FALSE, CELL_FETCH_BRK);
      return;
      break;
    default: e= '.'; break;
    }
  
  if (cell->get_type() & (CELL_HW_READ | CELL_HW_WRITE))
    {
      /* Hw is registered on it, delete event break */
      class cl_event_handler *eh;
      int t= CELL_NORMAL;
      if ((eh= cell->get_event_handler()))
	t= eh->del_bp(brk);
      if (t & (CELL_READ_BRK|CELL_WRITE_BRK))
	{
	  cell->set_type(cell->get_type() & ~(CELL_READ_BRK|CELL_WRITE_BRK));
	  cell->set_type(cell->get_type() | t);
	  return;
	}
      nc= new cl_registered_cell(width);
      nc->set(cell->get());
      nc->set_type(cell->get_type() & ~CELL_GENERAL);
      nc->set_type(cell->get_type() | (cell->get_type() & CELL_GENERAL));
      int i= 0;
      class cl_hw *hw;
      while ((hw= cell->get_hw(i)) != 0)
	{
	  nc->add_hw(hw, 0);
	  i++;
	}
      if (((class cl_registered_cell *)cell)->hardwares)
	free(((class cl_registered_cell *)cell)->hardwares);
    }
  else if (cell->get_type() & (CELL_READ_BRK | CELL_WRITE_BRK))
    {
      /* Break already set on it, delete brk */
      class cl_event_handler *eh;
      int t= CELL_NORMAL;
      if ((eh= cell->get_event_handler()))
	t= eh->del_bp(brk);
      if (t & (CELL_READ_BRK|CELL_WRITE_BRK))
	{
	  cell->set_type(cell->get_type() & ~(CELL_READ_BRK|CELL_WRITE_BRK));
	  cell->set_type(cell->get_type() | t);
	  return;
	}
      nc= new cl_normal_cell(width);
      nc->set(cell->get());
      nc->set_type(cell->get_type() & ~CELL_GENERAL);
      nc->set_type(cell->get_type() | (cell->get_type() & CELL_GENERAL));
      return;
    }
  else
    {
      /* Normal cell */
      return;
    }

  if (addr >= size)
    {
      delete dummy;
      dummy= nc;
    }
  else
    {
      delete array[addr];
      array[addr]= nc;
    }
  uc->sim->/*app->*/mem_cell_changed(this, addr);
}


#ifdef STATISTIC
unsigned long
cl_m::get_nuof_reads(void)
{
  unsigned long res= 0;
  t_addr i;
  for (i= 0; i < size; i++)
    res+= array[i]->nuof_reads;
  return(res);
}

unsigned long
cl_m::get_nuof_writes(void)
{
  unsigned long res= 0;
  t_addr i;
  for (i= 0; i < size; i++)
    res+= array[i]->nuof_writes;
  return(res);
}

void
cl_m::set_nuof_reads(unsigned long value)
{
  t_addr i;
  for (i= 0; i < size; i++)
    array[i]->nuof_reads= value;
  dummy->nuof_reads= value;
}

void
cl_m::set_nuof_writes(unsigned long value)
{
  t_addr i;
  for (i= 0; i < size; i++)
    array[i]->nuof_writes= value;
  dummy->nuof_writes= value;
}
#endif


cl_normal_cell::cl_normal_cell(uchar awidth):
  cl_cell()
{
  type= CELL_NORMAL;
  data= 0;
  mask= 1;
  width= awidth;
  for (--awidth; awidth; awidth--)
    {
      mask<<= 1;
      mask|= 1;
    }
}

t_mem
cl_normal_cell::add(long what)
{
  t_mem d;
  
  if (width <= 8)
    d= /*TYPE_BYTE*/i8_t(data) + what;
  else if (width <= 16)
    d= /*TYPE_WORD*/i16_t(data) + what;
  else
    d= /*TYPE_DWORD*/i32_t(data) + what;
  return(data= d & mask);
}

t_mem
cl_normal_cell::wadd(long what)
{
  t_mem d;
  
  if (width <= 8)
    d= TYPE_BYTE(data) + what;
  else if (width <= 16)
    d= TYPE_WORD(data) + what;
  else
    d= TYPE_DWORD(data) + what;
  return(write(d));
}

void
cl_normal_cell::set_bit1(t_mem bits)
{
  bits&= mask;
  data|= bits;
}

void
cl_normal_cell::set_bit0(t_mem bits)
{
  bits&= mask;
  data&= ~bits;
}


/*
 */

cl_registered_cell::cl_registered_cell(uchar awidth):
  cl_normal_cell(awidth)
{
  type= CELL_HW_READ | CELL_HW_WRITE;
  //hws= new cl_list(1, 1);
  hardwares= 0;
  nuof_hws= 0;
}

cl_registered_cell::~cl_registered_cell(void)
{
  if (hardwares)
    free(hardwares);
}

/*void
cl_registered_cell::destroy(void)
{
  hardwares= 0;
  nuof_hws= 0;
}*/

t_mem
cl_registered_cell::read(void)
{
  int i;
  t_mem d= data;

  if (nuof_hws)
    for (i= 0; i < nuof_hws; i++)
      {
	d= hardwares[i]->read(this);
	;
      }
#ifdef STATISTIC
  nuof_reads++;
#endif
  return(d & mask);
}

t_mem
cl_registered_cell::read(enum hw_cath skip)
{
  int i;
  t_mem d= data;

  if (nuof_hws)
    for (i= 0; i < nuof_hws; i++)
      {
	if ((skip & hardwares[i]->cathegory) == 0)
	  d= hardwares[i]->read(this);
	;
      }
#ifdef STATISTIC
  nuof_reads++;
#endif
  return(d & mask);
}

t_mem
cl_registered_cell::write(t_mem val)
{
  int i;

  val&= mask;
  if (nuof_hws)
    for (i= 0; i < nuof_hws; i++)
      {
	hardwares[i]->write(this, &val);
	;
      }
#ifdef STATISTIC
  nuof_writes++;
#endif
  return(data= val & mask);
}

class cl_cell *
cl_registered_cell::add_hw(class cl_hw *hw, int *ith)
{
  if (!hw)
    {
      /* Whatta hell!? */
      return(0);
    }
  if (!hardwares)
    hardwares= (class cl_hw **)malloc(sizeof(class cl_hw *));
  else
    hardwares= (class cl_hw **)realloc(hardwares,
				       sizeof(class c_hw *) * (nuof_hws+1));
  hardwares[nuof_hws]= hw;
  nuof_hws++;
  if (ith)
    *ith= nuof_hws-1;
  return(this);
}

class cl_hw *
cl_registered_cell::get_hw(int ith)
{
  if (ith >= nuof_hws)
    return(0);
  return(hardwares[ith]);
}


/*
 */

cl_event_cell::cl_event_cell(uchar awidth, class cl_uc *auc):
  cl_normal_cell(awidth)
{
  eh= new cl_event_handler(auc);
}

cl_event_cell::~cl_event_cell(void)
{
  delete eh;
}

t_mem
cl_event_cell::read(void)
{
  if (type & CELL_READ_BRK)
    eh->read();
  return(cl_normal_cell::read());
}

t_mem
cl_event_cell::write(t_mem val)
{
  if (type & CELL_WRITE_BRK)
    eh->write();
  return(cl_normal_cell::write(val));
}


/*
 */

cl_ev_reg_cell::cl_ev_reg_cell(uchar awidth, class cl_uc *auc):
  cl_registered_cell(awidth)
{
  eh= new cl_event_handler(auc);
}

cl_ev_reg_cell::~cl_ev_reg_cell(void)
{}

t_mem
cl_ev_reg_cell::read(void)
{
  if (type & CELL_READ_BRK)
    eh->read();
  return(cl_registered_cell::read());
}

t_mem
cl_ev_reg_cell::write(t_mem val)
{
  if (type & CELL_WRITE_BRK)
    eh->write();
  return(cl_registered_cell::write(val));
}


/*
 */

cl_event_handler::cl_event_handler(class cl_uc *auc):
  cl_base()
{
  uc= auc;
  read_bps= new cl_list(1, 1);
  write_bps= new cl_list(1, 1);
}

cl_event_handler::~cl_event_handler(void)
{
  read_bps->disconn_all();
  write_bps->disconn_all();
  delete read_bps;
  delete write_bps;
}

void
cl_event_handler::write(void)
{
  int i;

  for (i= 0; i < write_bps->count; i++)
    {
      class cl_brk *bp= (class cl_brk *)(write_bps->at(i));
      uc->events->add(bp);
    }
}

void
cl_event_handler::read(void)
{
  int i;

  for (i= 0; i < read_bps->count; i++)
    {
      class cl_brk *bp= (class cl_brk *)(read_bps->at(i));
      uc->events->add(bp);
    }
}

int
cl_event_handler::add_bp(class cl_brk *bp)
{
  int t= CELL_NORMAL;

  if (!bp)
    return(CELL_NORMAL);
  switch (bp->get_event())
    {
    case brkWRITE: case brkWXRAM: case brkWIRAM: case brkWSFR:
      t|= CELL_WRITE_BRK;
      write_bps->add(bp);
      break;
    case brkREAD: case brkRXRAM: case brkRCODE: case brkRIRAM: case brkRSFR:
      t|= CELL_READ_BRK;
      read_bps->add(bp);
      break;
    default:
      t|= CELL_READ_BRK | CELL_WRITE_BRK;
      read_bps->add(bp);
      write_bps->add(bp);
      break;
    }
  return(t);
}

int
cl_event_handler::copy_from(class cl_event_handler *eh)
{
  int i, t= CELL_NORMAL;
  
  if (!eh)
    return(t);
  for (i= 0; i < eh->read_bps->count; i++)
    {
      class cl_brk *bp= (class cl_brk *)(eh->read_bps->at(i));
      t|= add_bp(bp);
    }
  for (i= 0; i < eh->write_bps->count; i++)
    {
      class cl_brk *bp= (class cl_brk *)(eh->write_bps->at(i));
      t|= add_bp(bp);
    }
  return(t);
}

int
cl_event_handler::del_bp(class cl_brk *bp)
{
  int t= CELL_NORMAL;

  write_bps->disconn(bp);
  read_bps->disconn(bp);
  if (write_bps->count)
    t|= CELL_WRITE_BRK;
  if (read_bps->count)
    t|= CELL_READ_BRK;
  return(t);
}


