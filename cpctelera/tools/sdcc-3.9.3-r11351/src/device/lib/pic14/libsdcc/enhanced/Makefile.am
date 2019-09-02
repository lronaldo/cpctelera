## Process this file with Automake to create Makefile.in

noinst_LIBRARIES = libsdcce.a

libsdcce_a_SOURCES = $(LIBSDCC_SOURCES_COMMON)
libsdcce_a_SOURCES += _gptrget1.S
libsdcce_a_SOURCES += _gptrget2.S
libsdcce_a_SOURCES += _gptrget3.S
libsdcce_a_SOURCES += _gptrget4.S
libsdcce_a_SOURCES += _gptrput1.S
libsdcce_a_SOURCES += _gptrput2.S
libsdcce_a_SOURCES += _gptrput3.S
libsdcce_a_SOURCES += _gptrput4.S
libsdcce_a_SOURCES += macros.inc

# Build for enhanced devices.

ARCH = $(EARCH)
libsdcce_a_CFLAGS = $(AM_CFLAGS_E)

if ENABLE_NOOPTS
noinst_LIBRARIES += libsdcceo.a
libsdcceo_a_SOURCES = $(libsdcce_a_SOURCES)
libsdcceo_a_CFLAGS = $(AM_CFLAGS_EO)
endif ENABLE_NOOPTS

if ENABLE_EXPERIMENTAL
noinst_LIBRARIES += libsdccex.a
libsdccex_a_SOURCES = $(libsdcce_a_SOURCES)
libsdccex_a_CFLAGS = $(AM_CFLAGS_EX)
endif ENABLE_EXPERIMENTAL

if ENABLE_NOOPTS
if ENABLE_EXPERIMENTAL
noinst_LIBRARIES += libsdcceox.a
libsdcceox_a_SOURCES = $(libsdcce_a_SOURCES)
libsdcceox_a_CFLAGS = $(AM_CFLAGS_EOX)
endif ENABLE_EXPERIMENTAL
endif ENABLE_NOOPTS

SOURCE_DIRS = . ..
include $(top_srcdir)/Makefile.common
