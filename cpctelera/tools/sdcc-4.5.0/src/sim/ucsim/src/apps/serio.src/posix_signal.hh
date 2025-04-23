/******************************************************************************
 * posix_signal.hh - A signal handling class for linux + solaris             *
 * to convert posix into something easier to use                               *
 * Tim Hurman - t.hurman@virgin.net                                           *
 * Last edited on 01th Oct 1999                                               *
 ******************************************************************************/
#ifndef POSIX_SIGNAL_HEADER
#define POSIX_SIGNAL_HEADER

typedef void(*SIG_PF)(int);
class SigHandler
{
public:
  SigHandler();
  virtual ~SigHandler();
  
  virtual int SetSignal(int SIGNAL, SIG_PF ACTION);
  virtual int BlockSignal(int SIGNAL);
  virtual int UnBlockSignal(int SIGNAL);
};

#endif
