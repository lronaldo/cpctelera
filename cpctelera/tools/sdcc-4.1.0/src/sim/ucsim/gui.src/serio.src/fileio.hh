/******************************************************************************
 * to emulate the serial input and output of an 8051 controller               *
 * fileio.hh - file input and output                                          *
 ******************************************************************************/
#ifndef FILEIO_HEADER
#define FILEIO_HEADER

#include "config.h"

class FileIO
{
public:
  FileIO();
  FileIO(const char *infile, const char *outfile);
  virtual ~FileIO();
  
  virtual int SendByte(char b);
  virtual int RecvByte(char *b);
  virtual int SendStr(char *str);
  virtual int RecvStr(char *str);

  virtual int infile_id() { return fdin; }
  
private:
  int fdin;
  int fdout;
};

#endif
