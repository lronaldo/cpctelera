/******************************************************************************
 * to emulate the serial input and output of an 8051 controller               *
 * frontend.hh - ncurses frontend                                             *
 ******************************************************************************/
#ifndef FRONTEND_HEADER
#define FRONTEND_HEADER

#include <sys/types.h>
#include <curses.h>

#include "config.h"

struct COORDS_S
{
  int min_x;
  int max_x;
  int min_y;
  int max_y;
};
typedef struct COORDS_S COORDS;

struct COORDINATES_S
{
  int x;
  int y;
};
typedef struct COORDINATES_S COORDINATES;

enum filter_t
  {
   flt_none,
   flt_hex
  };

class Viewer
{
public:
  Viewer();
  virtual ~Viewer();
  virtual void DrawBox(void);
  virtual void AddStrOutWin(char *string);
  virtual void GetStrInWin(char *string);
  virtual void AddChOutWin(char b);
  virtual int  GetChInWin(char *res);

  virtual void iflt_mode(enum filter_t iflt);
  virtual void oflt_mode(enum filter_t oflt);
  virtual void set_length(int l);
  
private:
  WINDOW *inp, *outp;
  COORDS win_c, inp_c, outp_c;
  COORDINATES topleft, bottomright;//, current;
  int middle_y, middle_x;
  enum filter_t flt_in, flt_out;
  unsigned int ocnt, icnt;
  int line_length;
  
  unsigned char ohex_buf[16];//, ihex_buf[16];
  int ohex_ptr, ihex_ptr, ihex_high, ihex_val;
};

#endif
