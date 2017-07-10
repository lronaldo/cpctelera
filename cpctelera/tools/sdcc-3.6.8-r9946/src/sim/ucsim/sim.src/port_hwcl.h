/* $Id: port_hwcl.h 494 2016-11-10 20:52:08Z drdani $ */

#ifndef PORT_HW_HEADER
#define PORT_HW_HEADER

#include "newcmdposixcl.h"

#include "hwcl.h"


extern const char *keysets[7];


class cl_port_io: public cl_hw_io
{
 public:
  cl_port_io(class cl_hw *ihw);
  virtual int init(void);
  //virtual bool input_avail(void);  
};

class cl_port_data: public cl_base
{
 public:
  class cl_memory_cell *cell_p, *cell_in, *cell_dir;
  t_mem cache_p, cache_in, cache_dir, cache_value;
  char *keyset;
  int basx, basy;
};

enum { NUOF_PORT_UIS= 16 };

class cl_port_ui: public cl_hw
{
 public:
  class cl_port_data pd[16];
  int act_port;
 public:
  cl_port_ui(class cl_uc *auc, int aid, chars aid_string);

  virtual bool add_port(class cl_port_data *p, int nr);
  
  virtual void make_io(void);
  virtual void new_io(class cl_f *f_in, class cl_f *f_out);
  virtual bool proc_input(void);
  virtual bool handle_input(int c);
  virtual void refresh_display(bool force);
  virtual void draw_display(void);
};


#endif

/* End of sim.src/port_hwcl.h */
