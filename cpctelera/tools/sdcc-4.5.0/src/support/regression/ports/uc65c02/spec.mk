# Regression test specification for the mos6502 target running with uCsim

# simulation timeout in cycles
SIM_CYCLES = 1000000000

ifdef SDCC_BIN_PATH
  AS = $(SDCC_BIN_PATH)/sdas6500$(EXEEXT)
  UC65 = $(SDCC_BIN_PATH)/ucsim_mos6502$(EXEEXT)
else
  ifdef UCSIM_DIR
    UC65 = $(UCSIM_DIR)/mos6502.src/ucsim_mos6502$(EXEEXT)
  else
    UC65 = $(top_builddir)/sim/ucsim/src/sims/mos6502.src/ucsim_mos6502$(EXEEXT)
  endif
  AS = $(WINE) $(top_builddir)/bin/sdas6500$(EXEEXT)
ifndef CROSSCOMPILING
  SDCCFLAGS += --nostdinc -I$(top_srcdir)
  LINKFLAGS += --nostdlib -L$(top_builddir)/device/lib/build/mos65c02
endif
endif

  EMU = $(WINE) $(UC65)

ifdef CROSSCOMPILING
  SDCCFLAGS += -I$(top_srcdir)
endif

SDCCFLAGS += -mmos65c02 --less-pedantic --code-loc 0x8000 --xram-loc 0x0200 --i-code-in-asm --opt-code-speed
LINKFLAGS += mos65c02.lib

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
