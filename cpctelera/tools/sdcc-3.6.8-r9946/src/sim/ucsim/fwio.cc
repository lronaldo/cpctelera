// Need to define _WIN32_WINNT as 0x500 or higher for GetConsoleWindow()
// _WIN32_WINNT will be indirectly defined from WINVER
#ifdef WINVER
#undef WINVER
#endif
#define WINVER 0x500

#include "ddconfig.h"

#ifdef SOCKET_AVAIL
# include HEADER_SOCKET
#endif

#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <stdarg.h>

#include "fwiocl.h"


cl_f *dd= NULL;

void deb(const char *format, ...)
{
  return;
  /*if (dd==NULL)
    {
      dd= cp_io(stdout,cchars("w"));
      dd->init();
      }*/
  va_list ap;
  va_start(ap, format);
  //dd->vprintf(format, ap);
  vprintf(format, ap);
  va_end(ap);
}


static void
init_winsock(void)
{
  static bool is_initialized = false;

  if (!is_initialized)
    {
      WSADATA wsaData;

      // Initialize Winsock
      int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
      if (iResult != 0)
        {
          fprintf(stderr, "WSAStartup failed: %d\n", iResult);
          exit(1);
        }
      //fprintf(stderr, "WSAStartup called\n");
      is_initialized = true;
    }
}

cl_io::cl_io(): cl_f()
{
  init_winsock();
}

cl_io::cl_io(chars fn, chars mode): cl_f(fn, mode)
{
}

cl_io::cl_io(int the_server_port): cl_f(the_server_port)
{
}

cl_io::~cl_io(void)
{
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

enum file_type
cl_io::determine_type()
{
  DWORD _file_type = GetFileType(handle);

  deb("wio determine type fid=%d\n", file_id);
  switch (_file_type)
    {
    case FILE_TYPE_CHAR:
      {
        DWORD err, NumPending;
	if (file_id > 2)
	  return F_FILE;
	if ((err= GetNumberOfConsoleInputEvents(handle, &NumPending)) == 0)
	  {
	    deb("wio file_id=%d (handle=%p) type=console1\n", file_id, handle);
	    return F_CONSOLE;
	  }
	else
	  deb("wio file_id=%d (handle=%p) cons_det1 err=%d\n", file_id, handle, (int)err);
	if (GetConsoleWindow() != NULL)
	  {
	    deb("wio file_id=%d (handle=%p) type=console2\n", file_id, handle);
	    return F_CONSOLE;
	  }
	else
	  deb("wio file_id=%d (handle=%p) cons_det2 NULL\n", file_id, handle);
        if (!ClearCommError(handle, &err, NULL))
          {
            switch (GetLastError())
              {
              case ERROR_INVALID_HANDLE:
		deb("wio file_id=%d (handle=%p) type=console3\n", file_id, handle);
                return F_CONSOLE;
		
              case ERROR_INVALID_FUNCTION:
                /*
                 * In case of NUL device return type F_FILE.
                 * Is this the correct way to test it?
                 */
		deb("wio file_id=%d (handle=%p) type=file\n", file_id, handle);
                return F_FILE;

              default:
                //assert(false);
		deb("wio file_id=%d (handle=%p) type=unknown\n", file_id, handle);
		return F_UNKNOWN;
              }
          }
      }
      deb("wio file_id=%d (handle=%p) type=serial\n", file_id, handle);
      return F_SERIAL;

    case FILE_TYPE_DISK:
      deb("wio file_id=%d (handle=%p) type=file2\n", file_id, handle);
      return F_FILE;

    }

  char sockbuf[256];
  int optlen = sizeof(sockbuf);
  int i;

  i= getsockopt((SOCKET)handle, SOL_SOCKET, SO_TYPE, sockbuf, &optlen);
  int e= WSAGetLastError();
  deb("Checking if fid=%d handle=%p is a socket, i=%d e=%d\n", file_id, handle, i, e);
  if ((i == 0) ||
      (WSAENOTSOCK != e))
    {
      deb("wio file_id=%d (handle=%p) type=socket\n", file_id, handle);
      return F_SOCKET;
    }
  
  //assert(false);
  deb("wio file_id=%d (handle=%p) type=pipe\n", file_id, handle);
  return F_PIPE;
}

int
cl_io::check_dev(void)
{
  //e_handle_type type= F_UNKNOWN;
  //if (F_UNKNOWN == type)
  //type = get_handle_type();

  switch (type)
    {
    case F_SOCKET:
    case F_LISTENER:
      {
        struct timeval tv = {0, 0};
	
        //assert(INVALID_HANDLE_VALUE != handle);
	
        fd_set s;
        FD_ZERO(&s);
        FD_SET((SOCKET)handle, &s);

        int ret = select(0, &s, NULL, NULL, &tv);
        if (SOCKET_ERROR == ret)
	  {
	    fprintf(stderr, "Can't select: %d fid=%d handle=%p type=%d\n",
		    WSAGetLastError(), file_id, handle, type);
	    return 0;
	  }
	if (type == F_LISTENER)
	  return ret;
	
        if ((ret != SOCKET_ERROR) &&
	    (ret != 0))
	  {
	    pick();
	  }
	return last_used != first_free;
      }

    case F_FILE:
    case F_PIPE:
      pick();
      return last_used != first_free;

    case F_CONSOLE:
      {
        INPUT_RECORD *pIRBuf;
	INPUT_RECORD *p;
        DWORD NumPending;
        DWORD NumPeeked;
	bool ret= last_used != first_free;
        /*
         * Peek all pending console events
         */
	//printf("win iput check on console id=%d handle=%p\n", file_id, handle);
        if (INVALID_HANDLE_VALUE == handle)
	  return ret;
	if (!GetNumberOfConsoleInputEvents(handle, &NumPending))
	  return ret;
	if (NumPending <= 0)
	  return ret;
	if (NULL == (pIRBuf = (PINPUT_RECORD)/*_*/malloc/*a*/(NumPending * sizeof(INPUT_RECORD))))
	  return ret;

        if (ReadConsoleInput(handle, pIRBuf, NumPending, &NumPeeked) == 0)
	  return free(pIRBuf), ret;
	if (NumPeeked == 0L)
	  return free(pIRBuf), ret;
	if (NumPeeked > NumPending)
	  return free(pIRBuf), ret;

	/*
	 * Scan all of the peeked events to determine if any is a key event
	 * which should be recognized.
	 */
	int key_presses= 0;
	for (p= pIRBuf ; NumPeeked > 0 ; NumPeeked--, p++ )
	  {
	    if (KEY_EVENT == p->EventType &&
		p->Event.KeyEvent.bKeyDown)
	      {
		int vk= p->Event.KeyEvent.wVirtualKeyCode;
		char c= p->Event.KeyEvent.uChar.AsciiChar;
		unsigned long int ctrl= p->Event.KeyEvent.dwControlKeyState;
		key_presses++;
		if (vk == VK_BACK) 		pick(8);
		else if (vk == VK_TAB)		pick(9);
		else if (vk == VK_RETURN)	pick('\n');
		else if (vk == VK_ESCAPE)	pick(0x1b);
		else if (vk == VK_SPACE)	pick(' ');
		else if (vk == VK_PRIOR)	pick("\033[5~");
		else if (vk == VK_NEXT)		pick("\033[6~");
		else if (vk == VK_END)		pick("\033[4~");
		else if (vk == VK_HOME)		pick("\033[1~");
		else if (vk == VK_LEFT)		pick("\033[D");
		else if (vk == VK_RIGHT)	pick("\033[C");
		else if (vk == VK_UP)		pick("\033[A");
		else if (vk == VK_DOWN)		pick("\033[B");
		else if (vk == VK_INSERT)	pick("\033[2~");
		else if (vk == VK_DELETE)	pick("\033[3~");
		else if ((vk >= 0x30) &&
			 (vk <= 0x39))		pick(c);
		else if ((vk >= 0x41) &&
			 (vk <= 0x5a))		pick(c);
		else if ((vk >= 0xb8) &&
			 (vk <= 0xd7))		pick(c);
		else if ((vk >= 0xdb) &&
			 (vk <= 0xdf))		pick(c);
		else if ((vk >= 0xe1) &&
			 (vk <= 0xe4))		pick(c);
		else if ((vk >= 0xe6) &&
			 (vk <= 0xf5))		pick(c);
		else if ((vk >= VK_NUMPAD0) &&
			 (vk <= VK_NUMPAD9))
		  {
		    if (ctrl & NUMLOCK_ON)	pick(vk-VK_NUMPAD0+'0');
		    else
		      switch (vk) {
		      case VK_NUMPAD0: pick("\033[2~"); break;
		      case VK_NUMPAD1: pick("\033[4~"); break;
		      case VK_NUMPAD2: pick("\033[B"); break;
		      case VK_NUMPAD3: pick("\033[6~"); break;
		      case VK_NUMPAD4: pick("\033[D"); break;
		      case VK_NUMPAD5: break;
		      case VK_NUMPAD6: pick("\033[C"); break;
		      case VK_NUMPAD7: pick("\033[1~"); break;
		      case VK_NUMPAD8: pick("\033[A"); break;
		      case VK_NUMPAD9: pick("\033[5~"); break;
		      };
		  }
		else if (vk == VK_MULTIPLY)	pick('*');
		else if (vk == VK_ADD)		pick('+');
		else if (vk == VK_SEPARATOR)	;
		else if (vk == VK_SUBTRACT)	pick('-');
		else if (vk == VK_DECIMAL)	pick('.');
		else if (vk == VK_DIVIDE)	pick('/');
		else if ((vk >= VK_F1) &&
			 (vk <= VK_F12))
		  {
		    char s[3];
		    sprintf(s, "%d", vk-VK_F1+11);
		    pick("\033["),pick(s[0]);
		    if (s[1]) pick(s[1]);
		    pick('~');
		  }
		else if ((vk >= VK_F13) &&
			 (vk <= VK_F24))	;
		//else printf("vk=%d 0x%x c='%c'\n",vk,vk,c);
	      }
	  }
      
	free(pIRBuf);
        return last_used != first_free;
      }
      
    case F_SERIAL:
      {
        DWORD err;
        COMSTAT comStat;
	
        bool res = ClearCommError(handle, &err, &comStat);
        //assert(res);
	if (!res)
	  return false;
        while (res && (comStat.cbInQue > 0))
	  {
	    pick();
	    res = ClearCommError(handle, &err, &comStat);
	  }
	return last_used != first_free;
      }
      
    default:
      //assert(false);
      return false;
    }
}

void
cl_io::check(void)
{
  if (type == F_CONSOLE)
    input_avail();
}

void
cl_io::changed(void)
{
  //printf("win_f changed fid=%d\n", file_id);
  if (file_id < 0)
    {
      // CLOSE
      //printf("Closing fid=%d\n", file_id);
      if ((F_SOCKET == type) ||
	  (F_LISTENER == type))
	{
	  //printf("Closing sock, handle=%p\n", handle);
	  shutdown((SOCKET)handle, SD_BOTH);
	  closesocket((SOCKET)handle);
	}
      handle= INVALID_HANDLE_VALUE;
      type= F_UNKNOWN;
      return;
    }

  // OPEN
  if (server_port > 0)
    {
      //printf("win opened socket id=%d\n", file_id);
      handle= (void*)file_id;
      type= F_SOCKET;
      deb("assuming TTY on socket %d\n", file_id);
      tty= true;
    }
  else
    {
      handle= (HANDLE)_get_osfhandle(file_id);
      type= determine_type();
      /*if (type == F_CONSOLE)
	{
	  if (strcmp("r", file_mode) == 0)
	    {
	      printf("wio: console mode 0\n");
	      SetConsoleMode(handle, 0);
	    }
	    }*/
      if (type == F_SOCKET)
	{
	  deb("determined socket, assume TTY... id=%d\n", file_id);
	  tty= true;
	}
    }
  //printf("win opened file id=%d\n", file_id);
  //printf("win handle=%p type=%d\n", handle, type);
}

void
cl_io::prepare_terminal()
{
  deb("wio set_attr fid=%d type=%d\n",file_id,type);
  if (type == F_CONSOLE)
    {
      deb("wio: console mode 0 fid=%d handle=%p\n", file_id, handle);
      SetConsoleMode(handle,0);
      //ENABLE_PROCESSED_OUTPUT|4/*ENABLE_VIRTUAL_TERMINAL_PROCESSING*/);
    }
  else if (type == F_SOCKET)
    {
      char s[7];
      sprintf(s, "%c%c%c%c%c%c", 0xff, 0xfb, 1, 0xff, 0xfb, 3 );
      write(s, 7);
    }
}
 

int
mk_srv_socket(int port)
{
  struct sockaddr_in name;

  init_winsock();
  //fprintf(stderr, "make_server_socket(%d)\n", port);
  /* Create the socket. */
  /*SOCKET*/unsigned int sock = WSASocket(PF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
  if (INVALID_SOCKET == sock)
    {
      fprintf(stderr, "socket: %d\n", WSAGetLastError());
      return -1;/*INVALID_SOCKET*/;
    }

  name.sin_family     = AF_INET;
  name.sin_port       = htons(port);
  name.sin_addr.s_addr= htonl(INADDR_ANY);
  if (SOCKET_ERROR == bind(sock, (struct sockaddr *)&name, sizeof(name)))
    {
      /*wchar_t*/LPWSTR s = NULL;
      int e= WSAGetLastError();
      FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		     FORMAT_MESSAGE_FROM_SYSTEM |
		     FORMAT_MESSAGE_IGNORE_INSERTS, 
		     NULL, e,
		     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		     (LPWSTR)&s, 0, NULL);
      fprintf(stderr, "bind of port %d: %d %S\n", port, e, s);
      LocalFree(s);
      return -1/*INVALID_SOCKET*/;
    }

  //printf("socket=%d\n", sock);
  return sock;
}


class cl_f *
mk_io(chars fn, chars mode)
{
  class cl_io *io;

  if (fn.empty())
    {
      io= new cl_io();
      io->init();
      return io;
    }
  else if (strcmp(fn, "-") == 0)
    {
      if (strcmp(mode, "r") == 0)
	{
	  return cp_io(fileno(stdin), mode);
	}
      else if (strcmp(mode, "w") == 0)
	{
	  return cp_io(fileno(stdout), mode);
	}
    }
  io= new cl_io(fn, mode);
  io->init();
  return io;
}

class cl_f *
cp_io(/*FILE *f*/int file_id, chars mode)
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

  //printf("mk_srv(%d)\n", server_port);
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
  //printf("win srv_accept(port=%d,new_sock=%d)\n", listen_io->server_port, new_sock);
  int fh= _open_osfhandle((intptr_t)new_sock, _O_TEXT);
  //printf("Accept, got fh=%d for new_socket %p\n", fh, (void*)new_sock);

  if (fin)
    {
      io= new cl_io();
      if (new_sock > 0)
	{
	  if (fh > 0)
	    {
	      FILE *f= fdopen(fh, "r");
	      //printf("fdopened f=%p for fh=%d as input\n", f, fh);
	      io->own_opened(f, cchars("r"));
	      io->type= F_SOCKET;
	      io->server_port= listen_io->server_port;
	    }
	}
      *fin= io;
    }

  if (fout)
    {
      io= new cl_io();
      if (new_sock > 0)
	{
	  //int fh= _open_osfhandle((intptr_t)new_sock, _O_TEXT);
	  if (fh > 0)
	    {
	      FILE *f= fdopen(fh, "w");
	      //printf("fdopened f=%p for fh=%d as output\n", f, fh);
	      io->use_opened(f, cchars("w"));
	      io->type= F_SOCKET;
	      io->server_port= listen_io->server_port;
	    }
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
      if (fio->input_avail())
	{
	  if (avail)
	    avail->add(fio);
	  ret= true;
	}
    }
  return ret;
}

void
msleep(int msec)
{
  Sleep(msec);
}

void
loop_delay()
{
  msleep(100);
}


/* End of fwio.cc */
