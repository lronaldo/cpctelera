/* auto-host_in.h: Define values for MSVC 6.0. During build this file
   should be copied to 'auto-host.h'.

Copyright (C) 2002  Jesus Calvino-Fraga, jesusc@ieee.org

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
Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.*/

#ifndef CPP_VC_H_
#define CPP_VC_H_

#include <sys/stat.h>
#include <io.h>
#include <malloc.h>
#include <stdio.h>

#define HAVE_STRINGIZE
#define STDC_HEADERS		1
#define PACKAGE "sdcc"
#define LOCALEDIR ""
#define PREFIX ""
#define inline  __inline
#define SIZEOF_INT			4
#define SIZEOF_LONG			4
#define HAVE_TIME_H			1
#define HAVE_STRING_H		1
#define HAVE_SYS_STAT_H		1
#define HAVE_FCNTL_H		1
#define HAVE_STDLIB_H		1
#define HAVE_STDDEF_H		1
#define HAVE_LIMITS_H		1
#ifndef __STDC__
#define __STDC__			1
#endif

#define ssize_t int

#ifdef _MSC_VER
/*So, which ones are the standard types? */
#define ino_t _ino_t
#define dev_t _dev_t
#define stat _stat
#define strdup _strdup
#define fstat _fstat
#define open _open
#define close _close
#define read _read
#define write _write

#define O_APPEND _O_APPEND
#define O_CREAT  _O_CREAT
#define O_EXCL   _O_EXCL
#define O_RDONLY _O_RDONLY
#define O_RDWR   _O_RDWR
#define O_TRUNC  _O_TRUNC
#define O_WRONLY _O_WRONLY
#define O_BINARY _O_BINARY
#define O_TEXT   _O_TEXT

/*This one borrowed from \borland\bcc55\include\sys\stat.h*/
#define S_IFBLK  0x3000  /* block special */

/*If you want to see all the scary warnings remove these ones:*/
#pragma warning( disable : 4244 )
#pragma warning( disable : 4090 )
#pragma warning( disable : 4022 )

typedef int intptr_t;
#endif  /* _MSC_VER */

#endif  /*CPP_VC_H_*/
