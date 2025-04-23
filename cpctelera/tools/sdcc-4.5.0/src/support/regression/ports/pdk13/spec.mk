# Regression test specification for the pdk13 target running with uCsim

EMU_PORT_FLAG = -tPDK13

# path to uCsim
ifdef SDCC_BIN_PATH
  UCPDK13C = $(SDCC_BIN_PATH)/ucsim_pdk$(EXEEXT)

  AS_PDK13C = $(SDCC_BIN_PATH)/sdaspdk$(EXEEXT)
else
  ifdef UCSIM_DIR
    UCPDK13A = $(UCSIM_DIR)/pdk.src/ucsim_pdk$(EXEEXT)
  else
    UCPDK13A = $(top_builddir)/sim/ucsim/src/sims/pdk.src/ucsim_pdk$(EXEEXT)
    UCPDK13B = $(top_builddir)/bin/ucsim_pdk$(EXEEXT)
  endif

  EMU = $(WINE) $(shell if [ -f $(UCPDK13A) ]; then echo $(UCPDK13A); else echo $(UCPDK13B); fi)

  AS = $(WINE) $(top_builddir)/bin/sdaspdk$(EXEEXT)

ifndef CROSSCOMPILING
  SDCCFLAGS += --nostdinc -I$(top_srcdir)
  LINKFLAGS += --nostdlib -L$(top_builddir)/device/lib/build/pdk13
endif
endif

ifdef CROSSCOMPILING
  SDCCFLAGS += -I$(top_srcdir)
endif

SDCCFLAGS += -mpdk13 --less-pedantic --out-fmt-ihx
LINKFLAGS += pdk13.lib

OBJEXT = .rel
BINEXT = .ihx

# otherwise `make` deletes testfwk.rel and `make -j` will fail
.PRECIOUS: $(PORT_CASES_DIR)/%$(OBJEXT)

# Required extras
EXTRAS = $(PORT_CASES_DIR)/testfwk$(OBJEXT) $(PORT_CASES_DIR)/support$(OBJEXT)
include $(srcdir)/fwk/lib/spec.mk

# Rule to link into .ihx
%$(BINEXT): %$(OBJEXT) $(EXTRAS) $(FWKLIB) $(PORT_CASES_DIR)/fwk.lib
	$(SDCC) $(SDCCFLAGS) $(LINKFLAGS) $(EXTRAS) $(PORT_CASES_DIR)/fwk.lib $< -o $@

%$(OBJEXT): %.asm
	$(AS) -plosgff $<

%$(OBJEXT): %.c
	$(SDCC) $(SDCCFLAGS) -c $< -o $@

$(PORT_CASES_DIR)/%$(OBJEXT): $(PORTS_DIR)/$(PORT)/%.c
	$(SDCC) $(SDCCFLAGS) -c $< -o $@

$(PORT_CASES_DIR)/%$(OBJEXT): $(srcdir)/fwk/lib/%.c
	$(SDCC) $(SDCCFLAGS) -c $< -o $@

$(PORT_CASES_DIR)/fwk.lib: $(srcdir)/fwk/lib/fwk.lib
	cat < $(srcdir)/fwk/lib/fwk.lib > $@

_clean:
