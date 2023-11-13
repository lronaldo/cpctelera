## Process this file with Automake to create Makefile.in

noinst_LIBRARIES = libsdcc.a

libsdcc_a_SOURCES = $(LIBSDCC_SOURCES_COMMON)
libsdcc_a_SOURCES += _gptrget1.S
libsdcc_a_SOURCES += _gptrget2.S
libsdcc_a_SOURCES += _gptrget3.S
libsdcc_a_SOURCES += _gptrget4.S
libsdcc_a_SOURCES += _gptrput1.S
libsdcc_a_SOURCES += _gptrput2.S
libsdcc_a_SOURCES += _gptrput3.S
libsdcc_a_SOURCES += _gptrput4.S
libsdcc_a_SOURCES += macros.inc
libsdcc_a_SOURCES += shadowregs.c

libsdcc_a_CFLAGS = $(AM_CFLAGS)

if ENABLE_NOOPTS
noinst_LIBRARIES += libsdcco.a
libsdcco_a_SOURCES = $(libsdcc_a_SOURCES)
libsdcco_a_CFLAGS = $(AM_CFLAGS_O)
endif ENABLE_NOOPTS

if ENABLE_EXPERIMENTAL
noinst_LIBRARIES += libsdccx.a
libsdccx_a_SOURCES = $(libsdcc_a_SOURCES)
libsdccx_a_CFLAGS = $(AM_CFLAGS_X)
endif ENABLE_EXPERIMENTAL

if ENABLE_NOOPTS
if ENABLE_EXPERIMENTAL
noinst_LIBRARIES += libsdccox.a
libsdccox_a_SOURCES = $(libsdcc_a_SOURCES)
libsdccox_a_CFLAGS = $(AM_CFLAGS_OX)
endif ENABLE_EXPERIMENTAL
endif ENABLE_NOOPTS

SOURCE_DIRS = . ..
include $(top_srcdir)/Makefile.common
