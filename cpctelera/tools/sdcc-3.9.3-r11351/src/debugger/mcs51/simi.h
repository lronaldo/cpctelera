/*-------------------------------------------------------------------------
  simi.h - Header file for simulator interaction
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

#ifndef  SIMI_H
#define  SIMI_H

#ifdef _WIN32
/* ugly hack to prevent the incusion of objidl.h */
#ifdef __MINGW32__
#define _OLE2_H
#elif defined _MSC_VER
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _OLE2_H_
#endif
#include <winsock2.h>
#endif

#define MAX_SIM_BUFF 8*1024

#define MAX_CACHE_SIZE 2048
/* number of cache */
#define IMEM_CACHE     0
#define XMEM_CACHE     1
#define SREG_CACHE     2
#define NMEM_CACHE     3
/* special index */
#define  BIT_CACHE     4

typedef struct _memcache
{
    unsigned int addr;
    unsigned int size;
    char buffer[MAX_CACHE_SIZE];
} memcache_t;

//#define SIMNAME "s51"
#ifdef _WIN32
extern SOCKET sock;
#else
extern int sock;
#endif
extern char simactive;
void  openSimulator (char **,int);
void waitForSim(int timeout_ms, char *expect);
void  closeSimulator ();
void  sendSim(char *);
char *simResponse();
void  simSetPC (unsigned int);
void  simSetBP (unsigned int);
void  simClearBP (unsigned int);
void  simLoadFile(char *);
void  simReset ();
char  *simRegs() ;
unsigned int simGoTillBp (unsigned int);
unsigned long simGetValue (unsigned int ,char , unsigned int );
int simSetValue (unsigned int ,char , unsigned int, unsigned long );
#endif
