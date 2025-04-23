# Regression test specification for the s08 target running with uCsim

EMU_PORT_FLAG=-thcs08

# path to uCsim
ifdef SDCC_BIN_PATH
  UCHC08C = $(SDCC_BIN_PATH)/ucsim_m68hc08$(EXEEXT)

  AS_HC08C = $(SDCC_BIN_PATH)/sdas6808$(EXEEXT)
else
  ifdef UCSIM_DIR
    UCHC08A = $(UCSIM_DIR)/m68hc08.src/ucsim_m68hc08$(EXEEXT)
  else
    UCHC08A = $(top_builddir)/sim/ucsim/src/sims/m68hc08.src/ucsim_m68hc08$(EXEEXT)
    UCHC08B = $(top_builddir)/bin/ucsim_m68hc08$(EXEEXT)
  endif

  EMU = $(WINE) $(shell if [ -f $(UCHC08A) ]; then echo $(UCHC08A); else echo $(UCHC08B); fi)

  AS = $(WINE) $(top_builddir)/bin/sdas6808$(EXEEXT)

ifndef CROSSCOMPILING
  SDCCFLAGS += --nostdinc -I$(top_srcdir)
  LINKFLAGS += --nostdlib -L$(top_builddir)/device/lib/build/s08-stack-auto
endif
endif

ifdef CROSSCOMPILING
  SDCCFLAGS += -I$(top_srcdir)
endif

SDCCFLAGS += -ms08 --stack-auto --less-pedantic --out-fmt-ihx
LINKFLAGS += s08.lib

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
