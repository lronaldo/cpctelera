## Process this file with Automake to create Makefile.in

SUBDIRS = libm libsdcc/regular

if ENABLE_ENHANCED_PICS
if ENABLE_EXTENDED_INSTRUCTIONS
SUBDIRS += libsdcc/enhanced
else !ENABLE_EXTENDED_INSTRUCTIONS
SUBDIRS += libsdcc/enhanced-no-xinst
endif !ENABLE_EXTENDED_INSTRUCTIONS
endif ENABLE_ENHANCED_PICS

include $(top_srcdir)/Makefile.common

DISTCLEANFILES = a.cod a.hex ./.checkdevices/*

distclean-local:
	rm -rf ./.checkdevices ./autom4te.cache
