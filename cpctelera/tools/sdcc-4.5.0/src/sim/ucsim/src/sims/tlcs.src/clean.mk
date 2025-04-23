
clean: local_clean sub_clean

local_clean:
	rm -f *core *[%~] *.[oa] *.map
	rm -f .[a-z]*~
	rm -f stlcs stlcs.exe
	rm -f ucsim_tlcs ucsim_tlcs.exe

sub_clean:
ifneq ($(shell test -f test/Makefile && echo ok), )
	$(MAKE) -C test -f clean.mk clean
endif

distclean: local_distclean sub_distclean

local_distclean: local_clean

local_distclean:
	rm -f config.cache config.log config.status
	rm -f Makefile *.dep
	rm -f *.obj *.list *.lst *.hex
	rm -f test/Makefile

sub_distclean:
