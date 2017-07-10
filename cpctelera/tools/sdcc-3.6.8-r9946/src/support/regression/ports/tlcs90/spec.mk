# Regression test specification for the tlcs90 target running with uCsim

# simulation timeout in seconds
SIM_TIMEOUT = 30

# path to uCsim
ifdef SDCC_BIN_PATH
  UCTLCS90C = $(SDCC_BIN_PATH)/stlcs$(EXEEXT)

  AS_TLCS90C = $(SDCC_BIN_PATH)/sdastlcs90$(EXEEXT)
else
  ifdef UCSIM_DIR
    UCTLCS90A = $(UCSIM_DIR)/tlcs.src/stlcs$(EXEEXT)
  else
    UCTLCS90A = $(top_builddir)/sim/ucsim/tlcs.src/stlcs$(EXEEXT)
    UCTLCS90B = $(top_builddir)/bin/stlcs$(EXEEXT)
  endif

  EMU = $(WINE) $(shell if [ -f $(UCTLCS90A) ]; then echo $(UCTLCS90A); else echo $(UCTLCS90B); fi)

  AS = $(WINE) $(top_builddir)/bin/sdastlcs90$(EXEEXT)

ifndef CROSSCOMPILING
  SDCCFLAGS += --nostdinc -I$(top_srcdir)
  LINKFLAGS += --nostdlib -L$(top_builddir)/device/lib/build/tlcs90
endif
endif

ifdef CROSSCOMPILING
  SDCCFLAGS += -I$(top_srcdir)
endif

SDCCFLAGS += --debug -mtlcs90 --less-pedantic --out-fmt-ihx
LINKFLAGS += tlcs90.lib

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

# run simulator with SIM_TIMEOUT seconds timeout
%.out: %$(BINEXT) $(CASES_DIR)/timeout
	mkdir -p $(dir $@)
	-$(CASES_DIR)/timeout $(SIM_TIMEOUT) $(EMU) $< < $(PORTS_DIR)/$(PORT)/uCsim.cmd > $@ \
	  || echo -e --- FAIL: \"timeout, simulation killed\" in $(<:$(BINEXT)=.c)"\n"--- Summary: 1/1/1: timeout >> $@
	$(PYTHON) $(srcdir)/get_ticks.py < $@ >> $@
	-grep -n FAIL $@ /dev/null || true

_clean:
