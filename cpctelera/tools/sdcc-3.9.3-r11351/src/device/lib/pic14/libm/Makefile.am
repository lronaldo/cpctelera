## Process this file with Automake to create Makefile.in

noinst_LIBRARIES = libm.a

libm_a_SOURCES =
libm_a_SOURCES += acosf.c
libm_a_SOURCES += asincosf.c
libm_a_SOURCES += asinf.c
libm_a_SOURCES += atan2f.c
libm_a_SOURCES += atanf.c
libm_a_SOURCES += ceilf.c
libm_a_SOURCES += cosf.c
libm_a_SOURCES += coshf.c
libm_a_SOURCES += cotf.c
libm_a_SOURCES += errno.c
libm_a_SOURCES += expf.c
libm_a_SOURCES += fabsf.c
libm_a_SOURCES += floorf.c
libm_a_SOURCES += frexpf.c
libm_a_SOURCES += isinf.c
libm_a_SOURCES += isnan.c
libm_a_SOURCES += ldexpf.c
libm_a_SOURCES += log10f.c
libm_a_SOURCES += logf.c
libm_a_SOURCES += modff.c
libm_a_SOURCES += powf.c
libm_a_SOURCES += sincosf.c
libm_a_SOURCES += sincoshf.c
libm_a_SOURCES += sinf.c
libm_a_SOURCES += sinhf.c
libm_a_SOURCES += sqrtf.c
libm_a_SOURCES += tancotf.c
libm_a_SOURCES += tanf.c
libm_a_SOURCES += tanhf.c

libm_a_CFLAGS = $(AM_CFLAGS)

if ENABLE_ENHANCED_PICS
noinst_LIBRARIES += libme.a
libme_a_SOURCES = $(libm_a_SOURCES)
libme_a_CFLAGS = $(AM_CFLAGS_E)
endif ENABLE_ENHANCED_PICS

if ENABLE_NOOPTS
noinst_LIBRARIES += libmo.a
libmo_a_SOURCES = $(libm_a_SOURCES)
libmo_a_CFLAGS = $(AM_CFLAGS_O)
if ENABLE_ENHANCED_PICS
noinst_LIBRARIES += libmeo.a
libmeo_a_SOURCES = $(libm_a_SOURCES)
libmeo_a_CFLAGS = $(AM_CFLAGS_EO)
endif ENABLE_ENHANCED_PICS
endif ENABLE_NOOPTS

if ENABLE_EXPERIMENTAL
noinst_LIBRARIES += libmx.a
libmx_a_SOURCES = $(libm_a_SOURCES)
libmx_a_CFLAGS = $(AM_CFLAGS_X)
if ENABLE_ENHANCED_PICS
noinst_LIBRARIES += libmex.a
libmex_a_SOURCES = $(libm_a_SOURCES)
libmex_a_CFLAGS = $(AM_CFLAGS_EX)
endif ENABLE_ENHANCED_PICS
endif ENABLE_EXPERIMENTAL

if ENABLE_NOOPTS
if ENABLE_EXPERIMENTAL
noinst_LIBRARIES += libmox.a
libmox_a_SOURCES = $(libm_a_SOURCES)
libmox_a_CFLAGS = $(AM_CFLAGS_OX)
if ENABLE_ENHANCED_PICS
noinst_LIBRARIES += libmeox.a
libmeox_a_SOURCES = $(libm_a_SOURCES)
libmeox_a_CFLAGS = $(AM_CFLAGS_EOX)
endif ENABLE_ENHANCED_PICS
endif ENABLE_EXPERIMENTAL
endif ENABLE_NOOPTS

SOURCE_DIRS = .
include $(top_srcdir)/Makefile.common
