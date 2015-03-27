/*-------------------------------------------------------------------------
  simi.c - source file for simulator interaction
        Written By -  Sandeep Dutta . sandeep.dutta@usa.net (1999)

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   In other words, you are welcome to use, share and improve this program.
   You are forbidden to forbid anyone else to use, share and improve
   what you give them.   Help stamp out software-hoarding!
-------------------------------------------------------------------------*/
#include "sdcdb.h"
#undef DATADIR
#include "simi.h"
#include "newalloc.h"

#ifdef _WIN32
# include <windows.h>
# include <winsock2.h>
# include <io.h>
#else
# ifdef __sun
#   define BSD_COMP
#   include <sys/file.h>
# endif
# ifdef HAVE_SYS_SOCKET_H
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#   include <unistd.h>
#   include <sys/ioctl.h>
# else
#   error "Cannot build debugger without socket support"
# endif
#endif
#include <fcntl.h>
#include <signal.h>
#include <time.h>

FILE *simin ; /* stream for simulator input */
FILE *simout; /* stream for simulator output */

#ifdef _WIN32
SOCKET sock = INVALID_SOCKET;
PROCESS_INFORMATION *simPid = NULL;
#else
int sock = -1; /* socket descriptor to comm with simulator */
pid_t simPid = -1;
#endif
static char simibuff[MAX_SIM_BUFF];    /* sim buffer       */
static char *sbp = simibuff;           /* simulator buffer pointer */
char simactive = 0;

static memcache_t memCache[NMEM_CACHE];

/*-----------------------------------------------------------------*/
/* get data from  memory cache/ load cache from simulator          */
/*-----------------------------------------------------------------*/
static char *getMemCache(unsigned int addr,int cachenum, unsigned int size)
{
  char *resp, *buf;
  unsigned int laddr;
  memcache_t *cache = &memCache[cachenum];

  if ( cache->size <=   0 ||
       cache->addr > addr ||
       cache->addr + cache->size < addr + size )
    {
      if ( cachenum == IMEM_CACHE )
        {
          sendSim("di 0x0 0xff\n");
        }
      else if ( cachenum == SREG_CACHE )
        {
          sendSim("ds 0x80 0xff\n");
        }
      else
        {
          laddr = addr & 0xffffffc0;
          sprintf(cache->buffer,"dx 0x%x 0x%x\n",laddr,laddr+0xff );
          sendSim(cache->buffer);
        }
      waitForSim(100,NULL);
      resp = simResponse();
      cache->addr = strtol(resp,0,0);
      buf = cache->buffer;
      cache->size = 0;
      while ( *resp && *(resp+1) && *(resp+2))
        {
          /* cache is a stringbuffer with ascii data like
             " 00 00 00 00 00 00 00 00"
          */
          resp += 2;
          laddr = 0;
          /* skip thru the address part */
          while (isxdigit(*resp))
              resp++;
          while ( *resp && *resp != '\n')
            {
              if ( laddr < 24 )
                {
                  laddr++ ;
                  *buf++ = *resp ;
                }
              resp++;
            }
          resp++ ;
          cache->size += 8;
        }
      *buf = '\0';
      if ( cache->addr > addr ||
           cache->addr + cache->size < addr + size )
        {
          return NULL;
        }
    }
  return cache->buffer + (addr - cache->addr)*3;
}

/*-----------------------------------------------------------------*/
/* invalidate memory cache                                         */
/*-----------------------------------------------------------------*/
static void invalidateCache( int cachenum )
{
  memCache[cachenum].size = 0;
}

/*-----------------------------------------------------------------*/
/* waitForSim - wait till simulator is done doing its job          */
/*-----------------------------------------------------------------*/
void waitForSim(int timeout_ms, char *expect)
{
  int ch;
  clock_t timeout;

  Dprintf(D_simi, ("simi: waitForSim start(%d)\n", timeout_ms));
  sbp = simibuff;

  timeout = clock() + ((timeout_ms * CLOCKS_PER_SEC) / 1000);
  while (((ch = fgetc(simin)) > 0 ) && (clock() <= timeout))
    {
      *sbp++ = ch;
    }
  *sbp = 0;
  Dprintf(D_simi, ("waitForSim(%d) got[%s]\n", timeout_ms, simibuff));
}

/*-----------------------------------------------------------------*/
/* openSimulator - create a pipe to talk to simulator              */
/*-----------------------------------------------------------------*/
#ifdef _WIN32
static void init_winsock(void)
{
  static int is_initialized = 0;

  if (!is_initialized)
    {
      WSADATA wsaData;
      int iResult;

      // Initialize Winsock
      iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
      if (0 != iResult)
        {
          fprintf(stderr, "WSAStartup failed: %d\n", iResult);
          exit(1);
        }
    }
}

static PROCESS_INFORMATION *execSimulator(char **args, int nargs)
{
  STARTUPINFO si;
  static PROCESS_INFORMATION pi;
  char *cmdLine = argsToCmdLine(args, nargs);

  memset(&si, 0, sizeof(si));
  si.cb = sizeof(si);
  memset(&pi, 0, sizeof(pi));

  // Start the child process.
  if (!CreateProcess(NULL,   // No module name (use command line)
      cmdLine,        // Command line
      NULL,           // Process handle not inheritable
      NULL,           // Thread handle not inheritable
      FALSE,          // Set handle inheritance to FALSE
      0,              // No creation flags
      NULL,           // Use parent's environment block
      NULL,           // Use parent's starting directory
      &si,            // Pointer to STARTUPINFO structure
      &pi)            // Pointer to PROCESS_INFORMATION structure
     )
    {
      Safe_free(cmdLine);
      printf( "CreateProcess failed (%lu).\n", GetLastError() );
      return NULL;
    }

  return &pi;
}

void openSimulator (char **args, int nargs)
{
    struct sockaddr_in sin;
    int retry = 0;
    int i;
    int iResult;
    int fh;
    u_long iMode;

  init_winsock();

  Dprintf(D_simi, ("simi: openSimulator\n"));
#ifdef SDCDB_DEBUG
  if (D_simi & sdcdbDebug)
    {
      printf("simi: openSimulator: ");
      for (i=0; i < nargs; i++ )
        {
          printf("arg%d: %s ", i, args[i]);
        }
      printf("\n");
    }
#endif
  invalidateCache(XMEM_CACHE);
  invalidateCache(IMEM_CACHE);
  invalidateCache(SREG_CACHE);

  if (INVALID_SOCKET == (sock = WSASocket(PF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0)))
    {
      fprintf(stderr, "cannot create socket: %d\n", WSAGetLastError());
      exit(1);
    }

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = inet_addr("127.0.0.1");
  sin.sin_port = htons(9756);

 try_connect:
  /* connect to the simulator */
  if (SOCKET_ERROR == connect(sock, (struct sockaddr *)&sin, sizeof(sin)))
    {
      /* if failed then wait 1 second & try again
         do this for 10 secs only */
      if (retry < 10)
        {
          if ( !retry )
              simPid = execSimulator(args, nargs);
          retry ++;
          Sleep(1000);
          goto try_connect;
        }
      perror("connect failed :");
      exit(1);
    }

  iMode = 1; /* set non-blocking mode */
  iResult = ioctlsocket(sock, FIONBIO, &iMode);
  if (iResult != NO_ERROR)
    {
      perror("ioctlsocket failed");
      exit(1);
    }

  fh = _open_osfhandle(sock, _O_TEXT);
  if (-1 == fh)
    {
      perror("cannot _open_osfhandle");
      exit(1);
    }

  /* got the socket now turn it into a file handle */
  if (!(simin = fdopen(fh, "r")))
    {
      perror("cannot open socket for read");
      exit(1);
    }

  fh = _open_osfhandle(sock, _O_TEXT);
  if (-1 == fh)
    {
      perror("cannot _open_osfhandle");
      exit(1);
    }

  if (!(simout = fdopen(fh, "w")))
    {
      perror("cannot open socket for write");
      exit(1);
    }
  /* now that we have opened, wait for the prompt */
  waitForSim(200, NULL);
  simactive = 1;
}
#else
static int execSimulator(char **args, int nargs)
{
  if ((simPid = fork()))
    {
      Dprintf(D_simi, ("simi: simulator pid %d\n",(int) simPid));
    }
  else
    {
      /* we are in the child process : start the simulator */
      signal(SIGINT , SIG_IGN );
      signal(SIGABRT, SIG_IGN );
      signal(SIGHUP , SIG_IGN );
      signal(SIGCHLD, SIG_IGN );

      if (execvp(args[0],args) < 0)
        {
          perror("cannot exec simulator");
          exit(1);
        }
    }
  return simPid;
}

void openSimulator (char **args, int nargs)
{
    struct sockaddr_in sin;
    int retry = 0;
    int i;
    int iResult;
#if defined(__sun) || defined(__CYGWIN__)
    u_long iMode;
#endif

    Dprintf(D_simi, ("simi: openSimulator\n"));
#ifdef SDCDB_DEBUG
  if (D_simi & sdcdbDebug)
    {
      printf("simi: openSimulator: ");
      for (i=0; i < nargs; i++ )
        {
          printf("arg%d: %s ",i,args[i]);
        }
      printf("\n");
    }
#endif
  invalidateCache(XMEM_CACHE);
  invalidateCache(IMEM_CACHE);
  invalidateCache(SREG_CACHE);

  if ((sock = socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
      perror("cannot create socket");
      exit(1);
    }

  memset(&sin,0,sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = inet_addr("127.0.0.1");
  sin.sin_port = htons(9756);

 try_connect:
  /* connect to the simulator */
  if (connect(sock,(struct sockaddr *) &sin, sizeof(sin)) < 0)
    {
      /* if failed then wait 1 second & try again
         do this for 10 secs only */
      if (retry < 10)
        {
          if ( !retry )
            {
              simPid = execSimulator(args, nargs);
            }
          retry ++;
          sleep (1);
          goto try_connect;
        }
      perror("connect failed :");
      exit(1);
    }

#if defined(__sun) || defined(__CYGWIN__)
  iMode = 1; /* set non-blocking mode */
  iResult = ioctl(sock, FIONBIO, &iMode);
#else
  iResult = fcntl(sock, F_SETFL, O_NONBLOCK | O_ASYNC);
#endif
  if (iResult != 0)
    {
      perror("ioctl failed");
      exit(1);
    }

  /* got the socket now turn it into a file handle */
  if (!(simin = fdopen(sock, "r")))
    {
      fprintf(stderr,"cannot open socket for read\n");
      exit(1);
    }

  if (!(simout = fdopen(sock, "w")))
    {
      fprintf(stderr,"cannot open socket for write\n");
      exit(1);
    }
  /* now that we have opened, wait for the prompt */
  waitForSim(200,NULL);
  simactive = 1;
}
#endif

/*-----------------------------------------------------------------*/
/* simResponse - returns buffer to simulator's response            */
/*-----------------------------------------------------------------*/
char *simResponse(void)
{
  return simibuff;
}

/*-----------------------------------------------------------------*/
/* sendSim - sends a command to the simuator                 */
/*-----------------------------------------------------------------*/
void sendSim(char *s)
{
  if ( ! simout )
      return;

  Dprintf(D_simi, ("simi: sendSim-->%s", s));  // s has LF at end already
  fputs(s,simout);
  fflush(simout);
}


static int getMemString(char *buffer, char wrflag,
                        unsigned int *addr, char mem, unsigned int size )
{
  int cachenr = NMEM_CACHE;
  char *prefix;
  char *cmd ;

  if ( wrflag )
      cmd = "set mem";
  else
      cmd = "dump";
  buffer[0] = '\0' ;

  switch (mem)
    {
      case 'A': /* External stack */
      case 'F': /* External ram */
          prefix = "xram";
          cachenr = XMEM_CACHE;
          break;
      case 'C': /* Code */
      case 'D': /* Code / static segment */
          prefix = "rom";
          break;
      case 'B': /* Internal stack */
      case 'E': /* Internal ram (lower 128) bytes */
      case 'G': /* Internal ram */
          prefix = "iram";
          cachenr = IMEM_CACHE;
          break;
      case 'H': /* Bit addressable */
      case 'J': /* SBIT space */
          cachenr = BIT_CACHE;
          if ( wrflag )
            {
              cmd = "set bit";
            }
          sprintf(buffer,"%s 0x%x\n",cmd,*addr);
          return cachenr;
      case 'I': /* SFR space */
          prefix = "sfr" ;
          cachenr = SREG_CACHE;
          break;
      case 'R': /* Register space */
          prefix = "iram";
          /* get register bank */
          cachenr = simGetValue (0xd0,'I',1);
          *addr  += cachenr & 0x18 ;
          cachenr = IMEM_CACHE;
          break;
      default:
      case 'Z': /* undefined space code */
          return cachenr;
    }
  if ( wrflag )
      sprintf(buffer,"%s %s 0x%x\n",cmd,prefix,*addr);
  else
      sprintf(buffer,"%s %s 0x%x 0x%x\n",cmd,prefix,*addr,*addr+size-1);
  return cachenr;
}

void simSetPC( unsigned int addr )
{
  char buffer[40];
  sprintf(buffer,"pc %d\n", addr);
  sendSim(buffer);
  waitForSim(100,NULL);
  simResponse();
}

int simSetValue (unsigned int addr,char mem, unsigned int size, unsigned long val)
{
  unsigned int i;
  char cachenr;
  char buffer[40];
  char *s;

  if ( size <= 0 )
      return 0;

  cachenr = getMemString(buffer, 1, &addr, mem, size);
  if ( cachenr < NMEM_CACHE )
    {
      invalidateCache(cachenr);
    }
  s = buffer + strlen(buffer) -1;
  for ( i = 0 ; i < size ; i++ )
    {
      sprintf(s," 0x%lx", val & 0xff);
      s += strlen(s);
      val >>= 8;
    }
  sprintf(s,"\n");
  sendSim(buffer);
  waitForSim(100,NULL);
  simResponse();
  return 0;
}

/*-----------------------------------------------------------------*/
/* simGetValue - get value @ address for mem space                 */
/*-----------------------------------------------------------------*/
unsigned long simGetValue (unsigned int addr,char mem, unsigned int size)
{
  unsigned int b[4] = {0,0,0,0}; /* can be a max of four bytes long */
  char cachenr;
  char buffer[40];
  char *resp;

  if ( size <= 0 )
      return 0;

  cachenr = getMemString(buffer, 0, &addr, mem, size);

  resp = NULL;
  if ( cachenr < NMEM_CACHE )
    {
      resp = getMemCache(addr,cachenr,size);
    }
  if ( !resp )
    {
      /* create the simulator command */
      sendSim(buffer);
      waitForSim(100,NULL);
      resp = simResponse();

      /* got the response we need to parse it the response
         is of the form
         [address] [v] [v] [v] ... special case in
         case of bit variables which case it becomes
         [address] [assembler bit address] [v] */
      /* first skip thru white space */
      resp = trim_left(resp);

      if (strncmp(resp, "0x",2) == 0)
          resp += 2;

      /* skip thru the address part */
      while (isxdigit(*resp))
          resp++;
    }
  /* make the branch for bit variables */
  if ( cachenr == BIT_CACHE)
    {
      /* skip until newline */
      while (*resp && *resp != '\n' )
          resp++ ;
      if ( *--resp != '0' )
          b[0] = 1;
    }
  else
    {
      unsigned int i;

      for (i = 0 ; i < size ; i++ )
        {
          /* skip white space */
          resp = trim_left(resp);

          b[i] = strtol(resp,&resp,16);
        }
    }

  return b[0] | b[1] << 8 | b[2] << 16 | b[3] << 24 ;
}

/*-----------------------------------------------------------------*/
/* simSetBP - set break point for a given address                  */
/*-----------------------------------------------------------------*/
void simSetBP (unsigned int addr)
{
  char buff[50];

  sprintf(buff, "break 0x%x\n", addr);
  sendSim(buff);
  waitForSim(100, NULL);
}

/*-----------------------------------------------------------------*/
/* simClearBP - clear a break point                                */
/*-----------------------------------------------------------------*/
void simClearBP (unsigned int addr)
{
  char buff[50];

  sprintf(buff, "clear 0x%x\n", addr);
  sendSim(buff);
  waitForSim(100, NULL);
}

/*-----------------------------------------------------------------*/
/* simLoadFile - load the simulator file                           */
/*-----------------------------------------------------------------*/
void simLoadFile (char *s)
{
  char buff[128];

  sprintf(buff, "file \"%s\"\n", s);
  printf("%s",buff);
  sendSim(buff);
  waitForSim(500, NULL);
}

/*-----------------------------------------------------------------*/
/* simGoTillBp - send 'go' to simulator till a bp then return addr */
/*-----------------------------------------------------------------*/
unsigned int simGoTillBp ( unsigned int gaddr)
{
  char *sr;
  int wait_ms = 1000;

  invalidateCache(XMEM_CACHE);
  invalidateCache(IMEM_CACHE);
  invalidateCache(SREG_CACHE);
  if (gaddr == 0)
    {
      /* initial start, start & stop from address 0 */
      //char buf[20];

      // this program is setting up a bunch of breakpoints automatically
      // at key places.  Like at startup & main() and other function
      // entry points.  So we don't need to setup one here..
      //sendSim("break 0x0\n");
      //sleep(1);
      //waitForSim();

      sendSim("reset\n");
      waitForSim(wait_ms, NULL);
      sendSim("run 0x0\n");
    }
  else if (gaddr == -1)
    { /* resume */
      sendSim ("run\n");
      wait_ms = 100;
    }
  else if (gaddr == 1 )
    { /* nexti or next */
      sendSim ("next\n");
      wait_ms = 100;
    }
  else if (gaddr == 2 )
    { /* stepi or step */
      sendSim ("step\n");
      wait_ms = 100;
    }
  else
    {
      printf("Error, simGoTillBp > 0!\n");
      exit(1);
    }

  waitForSim(wait_ms, NULL);

  /* get the simulator response */
  sr = simResponse();
  /* check for errors */
  while ( *sr )
    {
      while ( *sr && *sr != 'E' )
          sr++ ;
      if ( !*sr )
          break;
      if ( ! strncmp(sr, "Error:", 6))
        {
          fputs(sr, stdout);
          break;
        }
      sr++ ;
    }

  nointerrupt = 1;
  /* get answer of stop command */
  if ( userinterrupt )
      waitForSim(wait_ms, NULL);

  /* better solution: ask pc */
  sendSim ("pc\n");
  waitForSim(100, NULL);
  sr = simResponse();
  nointerrupt = 0;

  gaddr = strtol(sr+3, 0, 0);
  return gaddr;
}

/*-----------------------------------------------------------------*/
/* simReset - reset the simulator                                  */
/*-----------------------------------------------------------------*/
void simReset (void)
{
  invalidateCache(XMEM_CACHE);
  invalidateCache(IMEM_CACHE);
  invalidateCache(SREG_CACHE);
  sendSim("res\n");
  waitForSim(100, NULL);
}

/*-----------------------------------------------------------------*/
/* closeSimulator - close connection to simulator                  */
/*-----------------------------------------------------------------*/
void closeSimulator (void)
{
#ifdef _WIN32
  if ( ! simin || ! simout || INVALID_SOCKET == sock )
#else
  if ( ! simin || ! simout || sock == -1 )
#endif
    {
      simactive = 0;
      return;
    }
  simactive = 0;
  sendSim("quit\n");
  fclose (simin);
  fclose (simout);
  shutdown(sock, 2);
#ifdef _WIN32
  closesocket(sock);
  sock = -1;
  if (NULL != simPid)
      TerminateProcess(simPid->hProcess, 0);
  // Close process and thread handles.
  CloseHandle(simPid->hProcess);
  CloseHandle(simPid->hThread);
#else
  close(sock);
  sock = -1;
  if ( simPid > 0 )
      kill (simPid,SIGKILL);
#endif
}
