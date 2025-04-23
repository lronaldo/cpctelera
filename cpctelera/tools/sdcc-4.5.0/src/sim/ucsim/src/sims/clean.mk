SIMS		= avr f8 i8048 i8085 m6800 \
		  m6809 m68hc08 m68hc11 m68hc12 \
		  mos6502 p1516 pblaze pdk rxk \
		  s51 st7 stm8 tlcs xa z80 oisc

all: clean

clean: local_clean sub_clean

local_clean:
	rm -f *~ lib*.a

sub_clean:
	@for sim in $(SIMS); do \
		$(MAKE) -C $${sim}.src -f clean.mk clean ;\
	done

distclean: local_distclean sub_distclean

local_distclean: local_clean

sub_distclean:
	@for sim in $(SIMS); do \
		$(MAKE) -C $${sim}.src -f clean.mk distclean ;\
	done
