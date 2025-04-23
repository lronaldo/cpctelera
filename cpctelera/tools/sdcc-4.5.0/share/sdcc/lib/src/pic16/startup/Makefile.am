## Makefile.am -- Process this file with automake to produce Makefile.in

# Dummy libs to generate crt0*.o
noinst_LIBRARIES = libcrt0.a libcrt0i.a libcrt0iz.a
libcrt0_a_SOURCES = crt0.c
libcrt0i_a_SOURCES = crt0i.c
libcrt0iz_a_SOURCES = crt0iz.c

# Force installation of .o files into $libdir
crtdir = $(libdir)
crt_DATA = crt0.o crt0i.o crt0iz.o

include $(top_srcdir)/Makefile.common

