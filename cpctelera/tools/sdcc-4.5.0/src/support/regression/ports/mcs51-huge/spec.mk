# Regression test specification for the mcs51-huge target running with uCsim
#
# model huge

SDCCFLAGS += --model-huge

include $(PORTS_DIR)/mcs51-common/spec.mk

LIBDIR = $(top_builddir)/device/lib/build/huge
