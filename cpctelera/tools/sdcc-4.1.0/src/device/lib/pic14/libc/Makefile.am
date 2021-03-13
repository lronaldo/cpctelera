## Process this file with Automake to create Makefile.in

noinst_LIBRARIES = libc.a

libc_a_SOURCES =

# assert.h
libc_a_SOURCES += __assert.c

# ctype.h
libc_a_SOURCES += isalnum.c
libc_a_SOURCES += isalpha.c
libc_a_SOURCES += isblank.c
libc_a_SOURCES += iscntrl.c
libc_a_SOURCES += isdigit.c
libc_a_SOURCES += isgraph.c
libc_a_SOURCES += islower.c
libc_a_SOURCES += isprint.c
libc_a_SOURCES += ispunct.c
libc_a_SOURCES += isspace.c
libc_a_SOURCES += isupper.c
libc_a_SOURCES += isxdigit.c
libc_a_SOURCES += tolower.c
libc_a_SOURCES += toupper.c

# errno.h
libc_a_SOURCES += errno.c

# setjmp.h
#libc_a_SOURCES += _setjmp.c

# stdio.h
libc_a_SOURCES += fprintf.c
libc_a_SOURCES += fputc.c
libc_a_SOURCES += fputs.c
libc_a_SOURCES += getchar.c
libc_a_SOURCES += gets.c
libc_a_SOURCES += printf.c
libc_a_SOURCES += printfl.c
libc_a_SOURCES += putchar.c
libc_a_SOURCES += puts.c
libc_a_SOURCES += sprintf.c
libc_a_SOURCES += streams.c
libc_a_SOURCES += strmgpsim.c
libc_a_SOURCES += strmmssp.c
libc_a_SOURCES += strmusart.c
libc_a_SOURCES += vfprintf.c
libc_a_SOURCES += vprintf.c
libc_a_SOURCES += vsprintf.c

# stdlib.h
libc_a_SOURCES += _atof.c
libc_a_SOURCES += _ftoa.c
libc_a_SOURCES += __itoa.c
libc_a_SOURCES += __ltoa.c
libc_a_SOURCES += abs.c
libc_a_SOURCES += aligned_alloc.c
libc_a_SOURCES += atoi.c
libc_a_SOURCES += atol.c
libc_a_SOURCES += bsearch.c
libc_a_SOURCES += _calloc.c
libc_a_SOURCES += free.c
libc_a_SOURCES += labs.c
libc_a_SOURCES += malloc.c
libc_a_SOURCES += mblen.c
libc_a_SOURCES += mbstowcs.c
libc_a_SOURCES += mbtowc.c
libc_a_SOURCES += memfree.c
libc_a_SOURCES += memfreemax.c
libc_a_SOURCES += memmisc.c
libc_a_SOURCES += qsort.c
libc_a_SOURCES += rand.c
libc_a_SOURCES += realloc.c
libc_a_SOURCES += strtol.c
libc_a_SOURCES += strtoul.c
libc_a_SOURCES += wcstombs.c
libc_a_SOURCES += wctomb.c

# string.h
libc_a_SOURCES += _memchr.c
libc_a_SOURCES += _memcmp.c
libc_a_SOURCES += __memcpy.c
libc_a_SOURCES += memcpy.c
libc_a_SOURCES += _memmove.c
libc_a_SOURCES += _memset.c
libc_a_SOURCES += _strcat.c
libc_a_SOURCES += _strchr.c
libc_a_SOURCES += _strcmp.c
libc_a_SOURCES += _strcpy.c
libc_a_SOURCES += _strcspn.c
libc_a_SOURCES += _strlen.c
libc_a_SOURCES += _strncat.c
libc_a_SOURCES += _strncmp.c
libc_a_SOURCES += _strncpy.c
libc_a_SOURCES += _strpbrk.c
libc_a_SOURCES += _strrchr.c
libc_a_SOURCES += _strspn.c
libc_a_SOURCES += _strstr.c
libc_a_SOURCES += _strtok.c
libc_a_SOURCES += strxfrm.c

# time.h
#libc_a_SOURCES += time.c

# uchar.c
libc_a_SOURCES += c16rtomb.c
libc_a_SOURCES += c16stombs.c
libc_a_SOURCES += c32rtomb.c
libc_a_SOURCES += mbrtoc16.c
libc_a_SOURCES += mbrtoc32.c
libc_a_SOURCES += mbstoc16s.c

# wchar.h
libc_a_SOURCES += btowc.c
libc_a_SOURCES += mbrlen.c
libc_a_SOURCES += mbrtowc.c
libc_a_SOURCES += mbsinit.c
libc_a_SOURCES += wcrtomb.c
libc_a_SOURCES += wcscmp.c
libc_a_SOURCES += wcslen.c
libc_a_SOURCES += wctob.c

libc_a_CFLAGS = $(AM_CFLAGS)

if ENABLE_ENHANCED_PICS
noinst_LIBRARIES += libce.a
libce_a_SOURCES = $(libc_a_SOURCES)
libce_a_CFLAGS = $(AM_CFLAGS_E)
endif ENABLE_ENHANCED_PICS

if ENABLE_NOOPTS
noinst_LIBRARIES += libco.a
libco_a_SOURCES = $(libc_a_SOURCES)
libco_a_CFLAGS = $(AM_CFLAGS_O)
if ENABLE_ENHANCED_PICS
noinst_LIBRARIES += libceo.a
libceo_a_SOURCES = $(libc_a_SOURCES)
libceo_a_CFLAGS = $(AM_CFLAGS_EO)
endif ENABLE_ENHANCED_PICS
endif ENABLE_NOOPTS

if ENABLE_EXPERIMENTAL
noinst_LIBRARIES += libcx.a
libcx_a_SOURCES = $(libc_a_SOURCES)
libcx_a_CFLAGS = $(AM_CFLAGS_X)
if ENABLE_ENHANCED_PICS
noinst_LIBRARIES += libcex.a
libcex_a_SOURCES = $(libc_a_SOURCES)
libcex_a_CFLAGS = $(AM_CFLAGS_EX)
endif ENABLE_ENHANCED_PICS
endif ENABLE_EXPERIMENTAL

if ENABLE_NOOPTS
if ENABLE_EXPERIMENTAL
noinst_LIBRARIES += libcox.a
libcox_a_SOURCES = $(libc_a_SOURCES)
libcox_a_CFLAGS = $(AM_CFLAGS_OX)
if ENABLE_ENHANCED_PICS
noinst_LIBRARIES += libceox.a
libceox_a_SOURCES = $(libc_a_SOURCES)
libceox_a_CFLAGS = $(AM_CFLAGS_EOX)
endif ENABLE_ENHANCED_PICS
endif ENABLE_EXPERIMENTAL
endif ENABLE_NOOPTS

SOURCE_DIRS = .
include $(top_srcdir)/Makefile.common
