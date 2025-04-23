# Regression test specification for the stm8 target running with uCsim

EMU_PORT_FLAG =

# path to uCsim
ifdef SDCC_BIN_PATH
  UCSTM8C = $(SDCC_BIN_PATH)/ucsim_stm8$(EXEEXT)

  AS_STM8C = $(SDCC_BIN_PATH)/sdasstm8$(EXEEXT)
else
  ifdef UCSIM_DIR
    UCSTM8A = $(UCSIM_DIR)/stm8.src/ucsim_stm8$(EXEEXT)
  else
    UCSTM8A = $(top_builddir)/sim/ucsim/src/sims/stm8.src/ucsim_stm8$(EXEEXT)
    UCSTM8B = $(top_builddir)/bin/ucsim_stm8$(EXEEXT)
  endif

  EMU = $(WINE) $(shell if [ -f $(UCSTM8A) ]; then echo $(UCSTM8A); else echo $(UCSTM8B); fi)

  AS = $(WINE) $(top_builddir)/bin/sdasstm8$(EXEEXT)

ifndef CROSSCOMPILING
  SDCCFLAGS += --nostdinc -I$(top_srcdir)
  LINKFLAGS += --nostdlib -L$(top_builddir)/device/lib/build/stm8
endif
endif

ifdef CROSSCOMPILING
  SDCCFLAGS += -I$(top_srcdir)
endif

SDCCFLAGS += -mstm8 --less-pedantic --out-fmt-ihx
LINKFLAGS += stm8.lib

OBJEXT = .rel
BINEXT = .ihx

# otherwise `make` deletes testfwk.rel and `make -j` will fail
.PRECIOUS: $(PORT_CASES_DIR)/%$(OBJEXT)

# Required extras
EXTRAS = $(PORT_CASES_DIR)/testfwk$(OBJEXT) $(PORT_CASES_DIR)/support$(OBJEXT)
include $(srcdir)/fwk/lib/spec.mk

%$(OBJEXT): %.asm
	$(AS) -plosgff $<

_clean:
