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

libm_a_CFLAGS = -p$(ARCH) $(AM_CFLAGS)

if ENABLE_ENHANCED_PICS
noinst_LIBRARIES += libme.a
endif ENABLE_ENHANCED_PICS
libme_a_SOURCES = $(libm_a_SOURCES)
libme_a_CFLAGS = -p$(EARCH) $(AM_CFLAGS)
if !ENABLE_EXTENDED_INSTRUCTIONS
libme_a_CFLAGS += --no-extended-instructions
endif !ENABLE_EXTENDED_INSTRUCTIONS

# Use the generic code where possible. Sadly using VPATH does not work:
#VPATH = $(srcdir) ../..
%.c: ../../%.c
	test -e $@ || ln -s $^ $@

include $(top_srcdir)/Makefile.common
