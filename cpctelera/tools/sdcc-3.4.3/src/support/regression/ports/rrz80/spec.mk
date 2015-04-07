# Regression test specification for the z80 target running on top of the Java based
# 'ConsoleZ80' emulator.

EMU = $(SDCC_EXTRA_DIR)/emu/rrz80/rrz80$(EXEEXT)
MAKEBIN = $(top_builddir)/bin/makebin$(EXEEXT)

SDCCFLAGS += --nostdinc -I$(top_srcdir)
LINKFLAGS += --nostdlib -L$(top_builddir)/device/lib/build/z80

SDCCFLAGS += -mz80 --less-pedantic --profile -DREENTRANT=
LINKFLAGS += --out-fmt-ihx z80.lib

OBJEXT = .rel
BINEXT = .bin

# otherwise `make` deletes testfwk.rel and `make -j` will fail
.PRECIOUS: $(PORT_CASES_DIR)/%$(OBJEXT)

# Needs parts of gbdk-lib, namely the internal mul/div/mod functions.
EXTRAS = $(PORT_CASES_DIR)/testfwk$(OBJEXT) $(PORT_CASES_DIR)/support$(OBJEXT)
include $(srcdir)/fwk/lib/spec.mk

# Rule to generate a Emulator .bin file from the .ihx linker output.
%$(BINEXT): %.ihx
	$(MAKEBIN) -s 32768 < $< > $@

# Rule to link into .ihx
%.ihx: %$(OBJEXT) $(EXTRAS) $(FWKLIB) $(PORT_CASES_DIR)/fwk.lib
	$(SDCC) $(SDCCFLAGS) $(LINKFLAGS) $(EXTRAS) $(PORT_CASES_DIR)/fwk.lib $< -o $@

$(PORT_CASES_DIR)/%$(OBJEXT): $(PORTS_DIR)/$(PORT)/%.asm
	@# TODO: sdas should place it\'s output in the current dir
	cp $< $(PORT_CASES_DIR)
	$(top_builddir)/bin/sdasz80 -plosgff $(PORT_CASES_DIR)/$(notdir $<)
	rm $(PORT_CASES_DIR)/$(notdir $<)

%$(OBJEXT): %.s
	$(top_builddir)/bin/sdasz80 -plosgff $<

%$(OBJEXT): %.c
	$(SDCC) $(SDCCFLAGS) -c $< -o $@

$(PORT_CASES_DIR)/%$(OBJEXT): $(srcdir)/fwk/lib/%.c
	$(SDCC) $(SDCCFLAGS) -c $< -o $@

$(PORT_CASES_DIR)/fwk.lib: $(srcdir)/fwk/lib/fwk.lib
	cat < $(srcdir)/fwk/lib/fwk.lib > $@

# PENDING: Path to sdcc-extra
%.out: %$(BINEXT)
	mkdir -p $(dir $@)
	$(EMU) --maxruntime=3 --mapfile=$(<:.bin=.sym) $< > $@
	-grep -n FAIL $@ /dev/null || true

_clean:
