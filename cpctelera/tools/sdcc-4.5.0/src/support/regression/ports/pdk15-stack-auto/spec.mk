# Regression test specification for the pdk14 target running with uCsim

EMU_PORT_FLAG = -tPDK15

# path to uCsim
ifdef SDCC_BIN_PATH
  UCPDK15C = $(SDCC_BIN_PATH)/ucsim_pdk$(EXEEXT)

  AS_PDK15C = $(SDCC_BIN_PATH)/sdaspdk$(EXEEXT)
else
  ifdef UCSIM_DIR
    UCPDK15A = $(UCSIM_DIR)/pdk.src/ucsim_pdk$(EXEEXT)
  else
    UCPDK15A = $(top_builddir)/sim/ucsim/src/sims/pdk.src/ucsim_pdk$(EXEEXT)
    UCPDK15B = $(top_builddir)/bin/ucsim_pdk$(EXEEXT)
  endif

  EMU = $(WINE) $(shell if [ -f $(UCPDK15A) ]; then echo $(UCPDK15A); else echo $(UCPDK15B); fi)

  AS = $(WINE) $(top_builddir)/bin/sdaspdk$(EXEEXT)

ifndef CROSSCOMPILING
  SDCCFLAGS += --nostdinc -I$(top_srcdir)
  LINKFLAGS += --nostdlib -L$(top_builddir)/device/lib/build/pdk15-stack-auto
endif
endif

ifdef CROSSCOMPILING
  SDCCFLAGS += -I$(top_srcdir)
endif

SDCCFLAGS += -mpdk15 --stack-auto --less-pedantic --out-fmt-ihx
LINKFLAGS += pdk15.lib

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
