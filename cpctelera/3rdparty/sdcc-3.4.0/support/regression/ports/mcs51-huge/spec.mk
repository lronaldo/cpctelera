# Regression test specification for the mcs51-huge target running with uCsim
#
# model huge

LIBSDCCFLAGS+= --model-huge --std-c99
SDCCFLAGS   += --model-huge

include $(PORTS_DIR)/mcs51-common/spec.mk

LIBDIR = $(top_builddir)/device/lib/build/huge

MAKE_LIBRARY = lib-files

.PHONY: lib-files
lib-files:
	echo making huge library
	make -C $(top_builddir)/device/lib models MODELS=huge
