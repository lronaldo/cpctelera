## Makefile.am -- Process this file with automake to produce Makefile.in

SUBDIRS = debug libc libio libm libsdcc startup

include $(top_srcdir)/Makefile.common

DISTCLEANFILES = a.cod a.hex ./.checkdevices/* pics.supported

distclean-local:
	rm -rf ./.checkdevices ./autom4te.cache
