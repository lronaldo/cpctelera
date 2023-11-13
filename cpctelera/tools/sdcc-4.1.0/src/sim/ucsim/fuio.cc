
#include "ddconfig.h"

#include <stdio.h>
#if defined HAVE_SYS_SOCKET_H
# include <sys/socket.h>
# include <sys/select.h>
# include <netinet/in.h>
# include <arpa/inet.h>
#endif
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <termios.h>

#include "utils.h"

#include "fuiocl.h"


cl_f *dd= NULL;

void deb(const char *format, ...)
{
  return;
  if (dd==NULL)
    {
      dd= mk_io(/*"/dev/pts/2", "w"*/"", "");
      dd->file_id= open("/dev/pts/4", O_WRONLY);
      //dd->init();
    }
  va_list ap;
  va_start(ap, format);
  //dd->vprintf(format, ap);
  //vdprintf(dd->file_id, format, ap);
  {
    char *buf= vformat_string(format, ap);
    /*dd->*/write(dd->file_id, buf, strlen(buf));
    free(buf);
  }
  va_end(ap);
}


cl_io::cl_io(): cl_f()
{
}

cl_io::cl_io(chars fn, chars mode): cl_f(fn, mode)
{
}

cl_io::cl_io(int the_server_port): cl_f(the_server_port)
{
}

int
cl_io::close(void)
{
  int i= 0;

  deb("fuio close fid=%d\n", file_id);
  if ((type == F_SOCKET) ||
      (type == F_LISTENER))
    {
      restore_attributes();
      shutdown(file_id, 2/*SHUT_RDWR*/);
    }
  /*
  if (file_f)
    {
      restore_attributes();
      i= fclose(file_f);
    }
    else*/ if (file_id >= 0)
    {
      restore_attributes();
      i= ::close(file_id);
    }

  //file_f= NULL;
  file_id= -1;
  own= false;
  file_name= 0;
  file_mode= 0;

  changed();
  return i;
}

cl_io::~cl_io(void)
{
  deb("~cl_uio fid=%d\n", file_id);
  restore_attributes();
  if (echo_of != NULL)
    echo_of->echo(NULL);
  if (/*file_f*/file_id>=0)
    {
      if (own)
	close();
      else
	stop_use();
    }
}

void
cl_io::changed(void)
{
  //printf("fuio changed fid=%d\n", file_id);
  if (file_id < 0)
    {
      type= F_UNKNOWN;
    }
  else
    {
      type= determine_type();
      if (type == F_SOCKET) tty= true;
    }
}

enum file_type
cl_io::determine_type(void)
{
  int i;
  struct stat s;
  
  if (file_id < 0)
    return F_UNKNOWN;
  i= fstat(file_id, &s);
  if (i < 0)
    return F_UNKNOWN;

  if (S_ISDIR(s.st_mode) ||
      S_ISLNK(s.st_mode))
    return F_UNKNOWN;
  if (S_ISCHR(s.st_mode))
    return F_CHAR;
  if (S_ISFIFO(s.st_mode))
    return F_PIPE;
  if (S_ISBLK(s.st_mode) ||
      S_ISREG(s.st_mode))
    return F_FILE;
  if (S_ISSOCK(s.st_mode))
    return F_SOCKET;
  return F_UNKNOWN;
}

int
cl_io::check_dev(void)
{
  struct timeval tv= { 0, 0 };
  fd_set s;
  int i;

  if (file_id<0)
    {
      return 0;
    }
  switch (type)
    {
    case F_UNKNOWN:
    case F_CONSOLE:
    case F_SERIAL:
      return false;
      break;
    case F_FILE:
      pick();
      return last_used != first_free;
      break;
    case F_CHAR:
    case F_SOCKET:
    case F_LISTENER:
    case F_PIPE:
      FD_ZERO(&s);
      FD_SET(file_id, &s);
      i= select(file_id+1, &s, NULL, NULL, &tv);
      if (i >= 0)
	{
	  int ret= FD_ISSET(file_id, &s);
	  if (type == F_LISTENER)
	    return ret;
	  if (ret)
	    {
	      pick();
	    }
	  return last_used != first_free;
	}
      break;
    }
  return 0;
}

void
cl_io::prepare_terminal()
{
  if (type == F_SOCKET)
    {
      // assume telnet client
      char s[7];
      //deb("preparing TELNET %d\n", file_id);
      sprintf(s, "%c%c%c%c%c%c", 0xff, 0xfb, 1, 0xff, 0xfb, 3 );
      write(s, 7);
    }
  else if (tty)
    {
      struct termios tattr;
      //deb("preparing TTY %d\n", file_id);
      tcgetattr(file_id, &tattr);
      tattr.c_iflag&= ~IXON;
      tattr.c_lflag&= ~ICANON;
      tattr.c_lflag&= ~ECHO;
      tattr.c_cc[VMIN] = 1;
      tattr.c_cc[VTIME]= 0;
      tcsetattr(file_id, TCSAFLUSH, &tattr);
    }
}

void
cl_io::save_attributes()
{
  if ((tty) &&
      !attributes_saved)
    {
      tcgetattr(file_id, &saved_attributes);
      attributes_saved= 1;
    }
}

void
cl_io::restore_attributes()
{
  if (attributes_saved)
    {
      saved_attributes.c_lflag|= ICANON|ECHO;
      tcsetattr(file_id, TCSAFLUSH, &saved_attributes);
      attributes_saved= 0;
    }
}

int
mk_srv_socket(int port)
{
  int sock, i;
  struct sockaddr_in name;

  /* Create the socket. */
  sock= socket(PF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    {
      perror("socket");
      return(0);
    }

  /* Give the socket a name. */
  i= 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*)&i, sizeof(i)) < 0)
    {
      perror("setsockopt");
    }
  name.sin_family     = AF_INET;
  name.sin_port       = htons(port);
  name.sin_addr.s_addr= htonl(INADDR_ANY);
  if (bind(sock, (struct sockaddr *)&name, sizeof(name)) < 0)
    {
      perror("bind");
      close(sock);
      return(0);
    }

  return(sock);
}


class cl_f *
mk_io(const char *fn, const char *mode)
{
  class cl_io *io;

  if (!fn || !*fn)
    {
      io= new cl_io();
      io->init();
      return io;
    }
  else if (strcmp(fn, "-") == 0)
    {
      if (strcmp(mode, "r") == 0)
	return cp_io(fileno(stdin), mode);
      else if (strcmp(mode, "w") == 0)
	return cp_io(fileno(stdout), mode);
    }
  io= new cl_io(fn, mode);
  io->init();
  return io;
}

class cl_f *
cp_io(/*FILE *f*/int file_id, const char *mode)
{
  class cl_io *io;

  io= new cl_io();
  if (/*f*/file_id>=0)
    io->use_opened(/*fileno(f)*/file_id, mode);
  return io;
}

class cl_f *
mk_srv(int server_port)
{
  class cl_io *io;

  io= new cl_io(server_port);
  io->init();
  io->type= F_LISTENER;
  return io;
}


int
srv_accept(class cl_f *listen_io,
	   class cl_f **fin, class cl_f **fout)
{
  class cl_io *io;
  //ACCEPT_SOCKLEN_T size;
  //struct sockaddr_in sock_addr;
  int new_sock;

  //size= sizeof(struct sockaddr);
  new_sock= accept(listen_io->file_id, /*(struct sockaddr *)sock_addr*/NULL, /*&size*/NULL);
  
  if (fin)
    {
      io= new cl_io(listen_io->server_port);
      if (new_sock > 0)
	{
	  io->own_opened(new_sock, "r");
	}
      *fin= io;
    }
  
  if (fout)
    {
      io= new cl_io(listen_io->server_port);
      if (new_sock > 0)
	{
	  io->use_opened(new_sock, "w");
	}
      *fout= io;
    }

  return 0;
}

bool
check_inputs(class cl_list *active, class cl_list *avail)
{
  int i;
  bool ret= false;
  
  if (!active)
    return false;

  if (avail)
    avail->disconn_all();
  
  for (i= 0; i < active->count; i++)
    {
      class cl_f *fio= (class cl_f *)active->at(i);
      //deb("checking fid=%d\n", fio->file_id);
      if (fio->check_dev() ||
	  fio->eof())
	{
	  deb("found dev input on fid=%d\n", fio->file_id);
	  if (avail)
	    avail->add(fio);
	  ret= true;
	}
      else
        {
          //deb("no dev input on fid=%d\n", fio->file_id);
        }
    }
  return ret;
}

void
msleep(int msec)
{
  struct timespec t;

  t.tv_sec= msec/1000;
  t.tv_nsec= (msec%1000)*1000000;
  nanosleep(&t, NULL);
}

void
loop_delay()
{
  msleep(100);
}


void
sigpipe_off()
{
  struct sigaction sa;
  sa.sa_handler= SIG_IGN;
  sigaction(SIGPIPE, &sa, NULL);
}

/* End of fuio.cc */
