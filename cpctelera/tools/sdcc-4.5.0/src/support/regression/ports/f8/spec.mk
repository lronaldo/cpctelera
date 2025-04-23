# Regression test specification for the f8 target running with uCsim

EMU_PORT_FLAG =

# path to uCsim
ifdef SDCC_BIN_PATH
  UCF8C = $(SDCC_BIN_PATH)/ucsim_f8$(EXEEXT)

  AS_F8C = $(SDCC_BIN_PATH)/sdasf8$(EXEEXT)
else
  ifdef UCSIM_DIR
    UCF8A = $(UCSIM_DIR)/f8.src/ucsim_f8$(EXEEXT)
  else
    UCF8A = $(top_builddir)/sim/ucsim/src/sims/f8.src/ucsim_f8$(EXEEXT)
    UCF8B = $(top_builddir)/bin/ucsim_f8$(EXEEXT)
  endif

  EMU = $(WINE) $(shell if [ -f $(UCF8A) ]; then echo $(UCF8A); else echo $(UCF8B); fi)

  AS = $(WINE) $(top_builddir)/bin/sdasf8$(EXEEXT)

ifndef CROSSCOMPILING
  SDCCFLAGS += --nostdinc -I$(top_srcdir)
  LINKFLAGS += --nostdlib -L$(top_builddir)/device/lib/build/f8
endif
endif

ifdef CROSSCOMPILING
  SDCCFLAGS += -I$(top_srcdir)
endif

SDCCFLAGS += -mf8 --less-pedantic --out-fmt-ihx
LINKFLAGS += f8.lib

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
