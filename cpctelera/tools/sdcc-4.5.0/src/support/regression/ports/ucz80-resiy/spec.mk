# Regression test specification for the z80 target running with uCsim

# path to uCsim
ifdef SDCC_BIN_PATH
  UCZ80C = $(SDCC_BIN_PATH)/ucsim_z80$(EXEEXT)

  AS_Z80C = $(SDCC_BIN_PATH)/sdasz80$(EXEEXT)
else
  ifdef UCSIM_DIR
    SZ80A = $(UCSIM_DIR)/z80.src/ucsim_z80$(EXEEXT)
  else
    SZ80A = $(top_builddir)/sim/ucsim/src/sims/z80.src/ucsim_z80$(EXEEXT)
    SZ80B = $(top_builddir)/bin/ucsim_z80$(EXEEXT)
  endif

  EMU = $(WINE) $(shell if [ -f $(SZ80A) ]; then echo $(SZ80A); else echo $(SZ80B); fi)

  AS = $(WINE) $(top_builddir)/bin/sdasz80$(EXEEXT)

ifndef CROSSCOMPILING
  SDCCFLAGS += --nostdinc -I$(top_srcdir)
  LINKFLAGS += --nostdlib -L$(top_builddir)/device/lib/build/z80
endif
endif

ifdef CROSSCOMPILING
  SDCCFLAGS += -I$(top_srcdir)
endif

SDCCFLAGS += -mz80 --less-pedantic --reserve-regs-iy
LINKFLAGS += z80.lib

OBJEXT = .rel
BINEXT = .ihx

# otherwise `make` deletes testfwk.rel and `make -j` will fail
.PRECIOUS: $(PORT_CASES_DIR)/%$(OBJEXT)

# Required extras
EXTRAS = $(PORT_CASES_DIR)/testfwk$(OBJEXT) $(PORT_CASES_DIR)/support$(OBJEXT)
include $(srcdir)/fwk/lib/spec.mk

$(PORT_CASES_DIR)/%$(OBJEXT): $(PORTS_DIR)/$(PORT)/%.asm
	@# TODO: sdas should place it\'s output in the current dir
	cp $< $(PORT_CASES_DIR)
	$(AS) -plosgff $(PORT_CASES_DIR)/$(notdir $<)
	rm $(PORT_CASES_DIR)/$(notdir $<)

_clean:
