## Makefile.am -- Process this file with automake to produce Makefile.in

lib_LIBRARIES = libm18f.a

libm18f_a_SOURCES =
libm18f_a_SOURCES += acosf.c
libm18f_a_SOURCES += asincosf.c
libm18f_a_SOURCES += asinf.c
libm18f_a_SOURCES += atan2f.c
libm18f_a_SOURCES += atanf.c
libm18f_a_SOURCES += ceilf.c
libm18f_a_SOURCES += cosf.c
libm18f_a_SOURCES += coshf.c
libm18f_a_SOURCES += cotf.c
libm18f_a_SOURCES += errno.c
libm18f_a_SOURCES += expf.c
libm18f_a_SOURCES += fabsf.c
libm18f_a_SOURCES += floorf.c
libm18f_a_SOURCES += frexpf.c
libm18f_a_SOURCES += isinf.c
libm18f_a_SOURCES += isnan.c
libm18f_a_SOURCES += ldexpf.c
libm18f_a_SOURCES += log10f.c
libm18f_a_SOURCES += logf.c
libm18f_a_SOURCES += modff.c
libm18f_a_SOURCES += powf.c
libm18f_a_SOURCES += sincosf.c
libm18f_a_SOURCES += sincoshf.c
libm18f_a_SOURCES += sinf.c
libm18f_a_SOURCES += sinhf.c
libm18f_a_SOURCES += sqrtf.c
libm18f_a_SOURCES += tancotf.c
libm18f_a_SOURCES += tanf.c
libm18f_a_SOURCES += tanhf.c

# Use the generic code where possible. Sadly using VPATH does not work:
#VPATH = $(srcdir) ../..
%.c: ../../%.c
	test -e $@ || ln -s $^ $@

include $(top_srcdir)/Makefile.common

