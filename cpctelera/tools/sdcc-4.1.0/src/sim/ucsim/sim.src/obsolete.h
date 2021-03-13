/*
class cl_mem: public cl_guiobj
{
public:
  char *addr_format, *data_format;
  ulong mask;
  enum mem_class type;
  char *class_name;
  union {
    void *mem;
    uchar *umem8;
  };
  //class cl_memloc_coll *read_locs, *write_locs;
  t_addr size;
  int width; // in bits
  class cl_uc *uc;
  t_addr dump_finished;

public:
  cl_mem(enum mem_class atype, char *aclass_name, t_addr asize, int awidth,
	 class cl_uc *auc);
  virtual ~cl_mem(void);
  virtual int init(void);
  virtual const char *id_string(void);
  virtual int get_cell_flag(t_addr //addr
			    ) { return(CELL_NORMAL); }
  virtual bool get_cell_flag(t_addr //addr
			     , int //flag
			     )
  { return(DD_FALSE); }
  virtual void set_cell_flag(t_addr addr, bool set_to, int flag) {}

  virtual t_mem read(t_addr addr);
  virtual t_mem read(t_addr addr, enum hw_cath //skip
		     ) {return(read(addr));}
  virtual t_mem get(t_addr addr);
  virtual t_mem write(t_addr addr, t_mem val);
  virtual void set(t_addr addr, t_mem val);
  virtual void set_bit1(t_addr addr, t_mem bits);
  virtual void set_bit0(t_addr addr, t_mem bits);
  virtual void write_bit1(t_addr addr, t_mem bits) { set_bit1(addr, bits); }
  virtual void write_bit0(t_addr addr, t_mem bits) { set_bit0(addr, bits); }
  virtual t_mem add(t_addr addr, long what);
  virtual t_mem wadd(t_addr addr, long what) { return(add(addr, what)); }
  virtual t_addr dump(t_addr start, t_addr stop, int bpl,
		      class cl_console *con);
  virtual t_addr dump(class cl_console *con);
  virtual bool search_next(bool case_sensitive,
			   t_mem *array, int len, t_addr *addr);

  virtual class cl_cell *get_cell(t_addr addr) {return(0);}
  virtual class cl_cell *register_hw(t_addr addr, class cl_hw *hw, int *ith,
				     bool announce)
  { return(0); }
  virtual void set_brk(t_addr //addr
		       , class cl_brk *//brk
		       ) {}
  virtual void del_brk(t_addr addr, class cl_brk *brk) {}
#ifdef STATISTIC
  virtual unsigned long get_nuof_reads(void) {return(0);}
  virtual unsigned long get_nuof_writes(void) {return(0);}
  virtual void set_nuof_reads(unsigned long value) {}
  virtual void set_nuof_writes(unsigned long value) {}
#endif
};
*/

/*
class cl_mapped_cell: public cl_cell
{
protected:
  class cl_cell *real_cell;
public:
  cl_mapped_cell(class cl_cell *realcell);
  virtual ~cl_mapped_cell(void);

  virtual t_mem read(void);
  virtual t_mem read(enum hw_cath skip);
  virtual t_mem get(void);
  virtual t_mem write(t_mem val);
  virtual t_mem set(t_mem val);
  virtual t_mem add(long what);
  virtual t_mem wadd(long what);

  virtual void set_bit1(t_mem bits);
  virtual void set_bit0(t_mem bits);

  virtual class cl_cell *add_hw(class cl_hw *hw, int *ith);
  virtual class cl_hw *get_hw(int ith);
  virtual class cl_event_handler *get_event_handler(void);
};
*/


class cl_m: public cl_memory
{
protected:
  class cl_cell **array;
  class cl_cell *dummy;
  t_addr bus_mask;
public:
  //t_addr size;
  enum mem_class type;

public:
  cl_m(enum mem_class atype, char *aclass_name, t_addr asize, int awidth,
       class cl_uc *auc);
  cl_m(t_addr asize, int awidth);
  virtual ~cl_m(void);
  virtual int init(void);
  virtual const char *id_string(void);

  virtual int get_cell_flag(t_addr addr);
  virtual bool get_cell_flag(t_addr addr, int flag);
  virtual void set_cell_flag(t_addr addr, bool set_to, int flag);

  virtual t_mem read(t_addr addr);
  virtual t_mem read(t_addr addr, enum hw_cath skip);
  virtual t_mem get(t_addr addr);
  virtual t_mem write(t_addr addr, t_mem val);
  virtual void set(t_addr addr, t_mem val);
  virtual class cl_cell *get_cell(t_addr addr);

  virtual void set_bit1(t_addr addr, t_mem bits);
  virtual void set_bit0(t_addr addr, t_mem bits);
  virtual void write_bit1(t_addr addr, t_mem bits);
  virtual void write_bit0(t_addr addr, t_mem bits);
  virtual t_mem add(t_addr addr, long what);
  virtual t_mem wadd(t_addr addr, long what);

  virtual bool search_next(bool case_sensitive,
			   t_mem *array, int len, t_addr *addr);

  virtual class cl_cell *register_hw(t_addr addr, class cl_hw *hw, int *ith,
				     bool announce);
  virtual void set_brk(t_addr addr, class cl_brk *brk);
  virtual void del_brk(t_addr addr, class cl_brk *brk);

#ifdef STATISTIC
  virtual unsigned long get_nuof_reads(void);
  virtual unsigned long get_nuof_writes(void);
  virtual void set_nuof_reads(unsigned long value);
  virtual void set_nuof_writes(unsigned long value);
#endif
};


class cl_normal_cell: public cl_cell
{
public:
  t_mem data;
  TYPE_UBYTE type;	// See CELL_XXXX
  //protected:

public:
  cl_normal_cell(uchar awidth);
  //virtual void destroy(void) {}

  virtual TYPE_UBYTE get_type(void) { return(type); }
  virtual void set_type(TYPE_UBYTE what) { type= what; }

  virtual t_mem read(void) {
#ifdef STATISTIC
    nuof_reads++;
#endif
    return(data);
  }
  virtual t_mem read(enum hw_cath skip) { return(data); }
  virtual t_mem get(void)  { return(data); }
  virtual t_mem write(t_mem val) {
    data= val & mask;
#ifdef STATISTIC
    nuof_writes++;
#endif
    return(data);
  }
  virtual t_mem set(t_mem val) { return(data= val & mask); }
  virtual t_mem add(long what);
  virtual t_mem wadd(long what);

  virtual void set_bit1(t_mem bits);
  virtual void set_bit0(t_mem bits);

  virtual class cl_cell *add_hw(class cl_hw *hw, int *ith)
  { return(0); }
  virtual class cl_hw *get_hw(int ith) { return(0); }
  //virtual class cl_brk *get_brk(void) { return(0); }
  virtual class cl_event_handler *get_event_handler(void) { return(0); }
};

class cl_registered_cell: public cl_memory_cell
{
public:
  //class cl_list *hws;
  class cl_hw **hardwares;
  int nuof_hws;
public:
  cl_registered_cell(uchar awidth);
  virtual ~cl_registered_cell(void);
  //virtual void destroy(void);

  virtual t_mem read(void);
  virtual t_mem read(enum hw_cath skip);
  virtual t_mem write(t_mem val);

  virtual class cl_cell *add_hw(class cl_hw *hw, int *ith);
  virtual class cl_hw *get_hw(int ith);
};

class cl_event_cell: public cl_normal_cell
{
protected:
  class cl_event_handler *eh;
public:
  cl_event_cell(uchar awidth, class cl_uc *auc);
  virtual ~cl_event_cell(void);

  virtual t_mem read(void);
  virtual t_mem write(t_mem val);
  //virtual void event(void);

  //virtual class cl_brk *get_brk(void) { return(brk); }
  virtual class cl_event_handler *get_event_handler(void) { return(eh); }
};

class cl_ev_reg_cell: public cl_registered_cell
{
protected:
  class cl_event_handler *eh;
public:
  cl_ev_reg_cell(uchar awidth, class cl_uc *auc);
  virtual ~cl_ev_reg_cell(void);
 
  virtual t_mem read(void);
  virtual t_mem write(t_mem val);
  //virtual void event(void);
  
  //virtual class cl_brk *get_brk(void) { return(brk); }
  virtual class cl_event_handler *get_event_handler(void) { return(eh); }
};

/*
 * 2nd version memory system
 */
class cl_cell: public cl_base
{
public:
  cl_cell(void);
public:

  virtual t_mem read(void)= 0;
  virtual t_mem read(enum hw_cath skip)=0;
  virtual t_mem get(void)=0;
  virtual t_mem write(t_mem val)=0;
  virtual t_mem set(t_mem val)=0;
  virtual t_mem add(long what)=0;
  virtual t_mem wadd(long what)=0;

  virtual void set_bit1(t_mem bits)=0;
  virtual void set_bit0(t_mem bits)=0;

  virtual class cl_cell *add_hw(class cl_hw *hw, int *ith)=0;
  virtual class cl_hw *get_hw(int ith)=0;
  virtual class cl_event_handler *get_event_handler(void)=0;
};

/*
 */
class cl_event_handler: public cl_base
{
public:
  class cl_list *read_bps, *write_bps;
  class cl_uc *uc;
public:
  cl_event_handler(class cl_uc *auc);
  virtual ~cl_event_handler(void);

  virtual void write(void);
  virtual void read(void);

  virtual int add_bp(class cl_brk *bp);
  virtual int copy_from(class cl_event_handler *eh);
  virtual bool del_bp(class cl_brk *bp);
};


