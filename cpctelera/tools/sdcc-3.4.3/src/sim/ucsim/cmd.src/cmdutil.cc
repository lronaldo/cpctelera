/*
 * Simulator of microcontrollers (cmd.src/cmdutil.cc)
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
 * Copyright (C) 2006, Borut Razem - borut.razem@siol.net
 *
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
 *
 */

/* This file is part of microcontroller simulator: ucsim.

UCSIM is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

UCSIM is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with UCSIM; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA. */
/*@1@*/

#include "ddconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/types.h>
#ifdef SOCKET_AVAIL
# include HEADER_SOCKET
# if defined HAVE_SYS_SOCKET_H
#  include <netinet/in.h>
#  include <arpa/inet.h>
# endif
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef _WIN32
#include <malloc.h>
#endif

#include "i_string.h"

#include "stypes.h"
#include "globals.h"
#include "uccl.h"
#include "cmdutil.h"


/*
 * Making a socket which can be used to listen for a specified port
 */

#ifdef SOCKET_AVAIL
#ifdef _WIN32
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
          printf("WSAStartup failed: %d\n", iResult);
          exit(1);
        }
    }
}

SOCKET
make_server_socket(unsigned short int port)
{
  init_winsock();

  struct sockaddr_in name;

  /* Create the socket. */
  SOCKET sock = WSASocket(PF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
  if (INVALID_SOCKET == sock)
    {
      fprintf(stderr, "socket: %d\n", WSAGetLastError());
      return INVALID_SOCKET;
    }

  name.sin_family     = AF_INET;
  name.sin_port       = htons(port);
  name.sin_addr.s_addr= htonl(INADDR_ANY);
  if (SOCKET_ERROR == bind(sock, (struct sockaddr *)&name, sizeof(name)))
    {
      fprintf(stderr, "bind: %d\n", WSAGetLastError());
      return INVALID_SOCKET;
    }

  return sock;
}
#else
int
make_server_socket(unsigned short int port)
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
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&i, sizeof(i)) < 0)
    {
      perror("setsockopt");
    }
  name.sin_family     = AF_INET;
  name.sin_port       = htons(port);
  name.sin_addr.s_addr= htonl(INADDR_ANY);
  if (bind(sock, (struct sockaddr *)&name, sizeof(name)) < 0)
    {
      perror("bind");
      return(0);
    }

  return(sock);
}
#endif
#endif

#if _WIN32
enum e_handle_type
get_handle_type(HANDLE handle)
{
  DWORD file_type = GetFileType(handle);

  switch (file_type)
    {
    case FILE_TYPE_CHAR:
      {
        DWORD err;

        if (!ClearCommError(handle, &err, NULL))
          {
            switch (GetLastError())
              {
              case ERROR_INVALID_HANDLE:
                return CH_CONSOLE;

              case ERROR_INVALID_FUNCTION:
                /*
                 * In case of NUL device return type CH_FILE.
                 * Is this the correct way to test it?
                 */
                return CH_FILE;

              default:
                assert(false);
              }
          }
      }
      return CH_SERIAL;

    case FILE_TYPE_DISK:
      return CH_FILE;
    }

  char sockbuf[256];
  int optlen = sizeof(sockbuf);

  if (SOCKET_ERROR != getsockopt((SOCKET)handle, SOL_SOCKET, SO_TYPE, sockbuf, &optlen) ||
    WSAENOTSOCK != WSAGetLastError())
    return CH_SOCKET;

  assert(false);
  return CH_UNDEF;
}

bool
input_avail(HANDLE handle, e_handle_type type)
{
  if (CH_UNDEF == type)
      type = get_handle_type(handle);

  switch (type)
    {
    case CH_SOCKET:
      {
        struct timeval tv = {0, 0};

        assert(INVALID_HANDLE_VALUE != handle);

        fd_set s;
        FD_ZERO(&s);
        FD_SET((SOCKET)handle, &s);

        int ret = select(0, &s, NULL, NULL, &tv);
        if (SOCKET_ERROR == ret)
          fprintf(stderr, "Can't select: %d\n", WSAGetLastError());

        return ret != SOCKET_ERROR && ret != 0;
      }

    case CH_FILE:
      return true;

    case CH_CONSOLE:
      {
        PINPUT_RECORD pIRBuf;
        DWORD NumPending;
        DWORD NumPeeked;

        /*
         * Peek all pending console events
         */
        if (INVALID_HANDLE_VALUE == handle ||
          !GetNumberOfConsoleInputEvents(handle, &NumPending) ||
          NumPending == 0 ||
          NULL == (pIRBuf = (PINPUT_RECORD)_alloca(NumPending * sizeof(INPUT_RECORD))))
          return false;

        if (PeekConsoleInput(handle, pIRBuf, NumPending, &NumPeeked) &&
          NumPeeked != 0L &&
          NumPeeked <= NumPending)
          {
            /*
             * Scan all of the peeked events to determine if any is a key event
             * which should be recognized.
             */
            for ( ; NumPeeked > 0 ; NumPeeked--, pIRBuf++ )
              {
                if (KEY_EVENT == pIRBuf->EventType &&
                  pIRBuf->Event.KeyEvent.bKeyDown &&
                  pIRBuf->Event.KeyEvent.uChar.AsciiChar)
                  return true;
              }
          }

        return false;
      }

    case CH_SERIAL:
      {
        DWORD err;
        COMSTAT comStat;

        bool res = ClearCommError(handle, &err, &comStat);
        assert(res);

        return res ? comStat.cbInQue > 0 : false;
      }

    default:
      assert(false);
      return false;
    }
}
#else
bool
input_avail(UCSOCKET_T fd)
{
  assert(0 <= fd);

  fd_set s;
  FD_ZERO(&s);
  FD_SET(fd, &s);

  struct timeval tv = {0, 0};

  int i = select(fd + 1, &s, NULL, NULL, &tv);
  if (i < 0)
    perror("select");

  return i > 0;
}
#endif

/*
 * Searching for a name in the specified table
 */

struct name_entry *
get_name_entry(struct name_entry tabl[], char *name, class cl_uc *uc)
{
  int i= 0;
  char *p;

  if (!tabl ||
      !name ||
      !(*name))
    return(0);
  for (p= name; *p; *p= toupper(*p), p++);
  while (tabl[i].name &&
         (!(tabl[i].cpu_type & uc->type) ||
         (strcmp(tabl[i].name, name) != 0)))
    {
      //printf("tabl[%d].name=%s <-> %s\n",i,tabl[i].name,name);
      i++;
    }
  if (tabl[i].name != NULL)
    return(&tabl[i]);
  else
    return(0);
}


/*
 * Interpreting a bitname
 */

/*bool
interpret_bitname(char *name, class cl_uc *uc,
                  uchar **cell, uchar *celladdr,
                  uchar *bitaddr, uchar *bitmask,
                  char **symname)
{
  char *dot, *p;
  char *sym, bitnumstr[2];
  struct name_entry *ne;
  int bitnum, i;

  if ((dot= strchr(name, '.')) != NULL)
    {
      *dot++= '\0';
      if ((ne= get_name_entry(uc->sfr_tbl(), name, uc)) == NULL)
        {
          *celladdr= strtol(name, &p, 0);
          if (p && *p)
            {
              dot--;
              *dot= '.';
              return(DD_FALSE);
            }
        }
      else
        *celladdr= ne->addr;
      if ((*celladdr < 0x20) ||
          ((*celladdr > 0x2f) && (*celladdr < 0x80)) ||
          ((*celladdr > 0x7f) && (*celladdr & 0x07)))
        return(DD_FALSE);
      bitnum= strtol(dot, &p, 0);
      if ((p && *p) ||
          (bitnum < 0) ||
          (bitnum > 7))
        return(DD_FALSE);
      if (*celladdr > 0x7f)
        *bitaddr= *celladdr + bitnum;
      else
        *bitaddr= (*celladdr - 0x20)*8 + bitnum;
      dot--;
      *dot= '.';
    }
  else
    {
      if ((ne= get_name_entry(uc->bit_tbl(), name, uc)) == NULL)
        {
          *bitaddr= strtol(name, &p, 0);
          if ((p && *p) ||
              (*bitaddr > 0xff))
            return(DD_FALSE);
        }
      else
        *bitaddr= ne->addr;
      if (*bitaddr > 0x7f)
        *celladdr= *bitaddr & 0xf8;
      else
        *celladdr= (*bitaddr >> 3) + 0x20;
    }
  // *bitaddr, *celladdr now OK
  *cell= uc->get_bit//FIXME
    (*bitaddr);
  *bitmask= BIT_MASK(*bitaddr);
  // making symbolic name
  if (!symname)
    return(DD_TRUE);
  i= 0;
  while (uc->bit_tbl()[i].name &&
         (uc->bit_tbl()[i].addr != *bitaddr))
    i++;
  if (uc->bit_tbl()[i].name)
    {
      sym= strdup(uc->bit_tbl()[i].name);
      *symname= sym;
      return(DD_TRUE);
    }
  i= 0;
  while (uc->sfr_tbl()[i].name &&
         (uc->sfr_tbl()[i].addr != *celladdr))
    i++;
  if (uc->sfr_tbl()[i].name)
    sym= strdup(uc->sfr_tbl()[i].name);
  else
    {
      sym= (char *)malloc(3);
      sprintf(sym, "%02x", *celladdr);
    }
  sym= (char *)realloc(sym, strlen(sym)+2);
  strcat(sym, ".");
  sprintf(bitnumstr, "%1d", *bitaddr & 0x07);
  strcat(sym, bitnumstr);
  *symname= sym;
  return(DD_TRUE);
}*/


/*
 * Processing escape sequencies in a string
 */

char *
proc_escape(char *string, int *len)
{
  char  spec_chars[]= "fnrtvab\"";
  char  spec[]= "\f\n\r\t\v\a\b\"";
  char  *s, *str, *p;

  s  = string;
  str= (char *)malloc(strlen(string)+1);
  p  = str;
  while (*s)
    {
      char *spec_c;

      if (*s == '\\' &&
          *(s+1))
        {
          s++;
          if (*s == '0')
            {
              if (!isdigit(*(s+1)))
                {
                  *p++= '\0';
                  s++;
                }
              else
                {
                  char *octal, *chk, data;
                  int i, j;
                  i= strspn(s, "01234567");
                  octal= (char *)malloc(i+1);
                  j= 0;
                  while (*s &&
                         (j < i))
                    octal[j++]= *s++;
                  octal[j]= '\0';
                  data= strtol(octal, &chk, 8);
                  if (!chk || !(*chk))
                    *p++= data;
                }
            }
          else
            if ((spec_c= strchr(spec_chars, *s)) != NULL)
              {
                *p++= spec[spec_c-spec_chars];
                s++;
              }
            else
              *p++= *s++;
        }
      else
        *p++= *s++;
    }
  *p= '\0';
  *len= p-str;
  return(str);
}


/* End of cmd.src/cmdutil.cc */
