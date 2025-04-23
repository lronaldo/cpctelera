# Regression test specification for the pdk14 target running with uCsim

# path to uCsim
ifdef SDCC_BIN_PATH
  UCPDK14C = $(SDCC_BIN_PATH)/ucsim_pdk$(EXEEXT)

  AS_PDK14C = $(SDCC_BIN_PATH)/sdaspdk$(EXEEXT)
else
  ifdef UCSIM_DIR
    UCPDK14A = $(UCSIM_DIR)/pdk.src/ucsim_pdk$(EXEEXT)
  else
    UCPDK14A = $(top_builddir)/sim/ucsim/src/sims/pdk.src/ucsim_pdk$(EXEEXT)
    UCPDK14B = $(top_builddir)/bin/ucsim_pdk$(EXEEXT)
  endif

  EMU = $(WINE) $(shell if [ -f $(UCPDK14A) ]; then echo $(UCPDK14A); else echo $(UCPDK14B); fi)

  AS = $(WINE) $(top_builddir)/bin/sdaspdk$(EXEEXT)

ifndef CROSSCOMPILING
  SDCCFLAGS += --nostdinc -I$(top_srcdir)
  LINKFLAGS += --nostdlib -L$(top_builddir)/device/lib/build/pdk14
endif
endif

ifdef CROSSCOMPILING
  SDCCFLAGS += -I$(top_srcdir)
endif

SDCCFLAGS += -mpdk14 --less-pedantic --out-fmt-ihx
LINKFLAGS += pdk14.lib

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
