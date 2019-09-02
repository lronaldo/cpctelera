#ifndef FUIOCL_HEADER
#define FUIOCL_HEADER


#include <termios.h>
#include <unistd.h>

#include "fiocl.h"

class cl_io: public cl_f
{
 public:
  cl_io();
  cl_io(chars fn, chars mode);
  cl_io(int the_server_port);
  virtual ~cl_io(void);
 protected:
  struct termios saved_attributes;
 public:
  virtual enum file_type determine_type(void);
  virtual int close(void);
  virtual void changed(void);
  virtual int check_dev(void);

  virtual void prepare_terminal();
  virtual void save_attributes();
  virtual void restore_attributes();
};


#endif

/* End of fuiocl.h */
