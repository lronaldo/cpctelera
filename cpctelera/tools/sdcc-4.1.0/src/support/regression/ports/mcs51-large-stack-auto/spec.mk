# Regression test specification for the mcs51-large-stack-auto target running with uCsim
#
# model large stack-auto

SDCCFLAGS +=--model-large --stack-auto

include $(PORTS_DIR)/mcs51-common/spec.mk

LIBDIR = $(top_builddir)/device/lib/build/large-stack-auto
