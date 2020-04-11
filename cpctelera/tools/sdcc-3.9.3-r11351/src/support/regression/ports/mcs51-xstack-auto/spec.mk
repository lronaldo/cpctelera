# Regression test specification for the mcs51-xstack-auto target running with uCsim
#
# model small xstack-auto

LIBSDCCFLAGS+= --stack-auto --xstack
SDCCFLAGS   += --stack-auto --xstack

include $(PORTS_DIR)/mcs51-common/spec.mk

LIBDIR      = $(top_builddir)/device/lib/build/small-xstack-auto

MAKE_LIBRARY = lib-files

.PHONY: lib-files
lib-files:
	$(MAKE) -C $(top_builddir)/device/lib model-mcs51-xstack-auto MODELS=small
