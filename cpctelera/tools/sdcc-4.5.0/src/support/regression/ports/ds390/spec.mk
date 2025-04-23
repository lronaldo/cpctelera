# Regression test specification for the ds390 target running with uCsim

.PRECIOUS: %.c

CC_FOR_BUILD = $(CC)

# simulation timeout in seconds
SIM_TIMEOUT = 240

EMU_PORT_FLAG = -tds390
EMU_FLAGS = -S in=$(DEV_NULL),out=-

# path to uCsim
ifdef SDCC_BIN_PATH
  S51 = $(SDCC_BIN_PATH)/ucsim_51$(EXEEXT)
else
  ifdef UCSIM_DIR
    S51A = $(UCSIM_DIR)/s51.src/ucsim_51$(EXEEXT)
  else
    S51A = $(top_builddir)/sim/ucsim/src/sims/s51.src/ucsim_51$(EXEEXT)
    S51B = $(top_builddir)/bin/ucsim_51$(EXEEXT)
  endif

  EMU = $(WINE) $(shell if [ -f $(S51A) ]; then echo $(S51A); else echo $(S51B); fi)

ifndef CROSSCOMPILING
  SDCCFLAGS += --nostdinc -I$(top_srcdir)
  LINKFLAGS += --nostdlib -L$(top_builddir)/device/lib/build/ds390
endif
endif

ifdef CROSSCOMPILING
  DEV_NULL ?= NUL
  SDCCFLAGS += -I$(top_srcdir)
else
  DEV_NULL ?= /dev/null
endif

SDCCFLAGS += -mds390 --less-pedantic -Wl-r
LINKFLAGS += libsdcc.lib liblong.lib liblonglong.lib libint.lib libfloat.lib
LINKFLAGS += libds390.lib

OBJEXT = .rel
BINEXT = .ihx

# otherwise `make` deletes testfwk.rel and `make -j` will fail
.PRECIOUS: $(PORT_CASES_DIR)/%$(OBJEXT)

# Required extras
EXTRAS = $(PORT_CASES_DIR)/testfwk$(OBJEXT) $(PORT_CASES_DIR)/support$(OBJEXT)
include $(srcdir)/fwk/lib/spec.mk

_clean:
