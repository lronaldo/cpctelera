# Regression test specification for the mcs51-xstack-auto target running with uCsim
#
# model small xstack-auto

LIBSDCCFLAGS+= --stack-auto --xstack --std-c99
SDCCFLAGS   += --stack-auto --xstack --std-sdcc99

include $(PORTS_DIR)/mcs51-common/spec.mk

LIBDIR      = $(top_builddir)/device/lib/build/small-xstack-auto

MAKE_LIBRARY = lib-files

.PHONY: lib-files
lib-files:
	make -C $(top_builddir)/device/lib model-mcs51-xstack-auto MODELS=small
