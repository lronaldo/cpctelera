
clean: local_clean sub_clean

local_clean:
	rm -f *core *[%~] *.[oa]
	rm -f test_*.??* '(null).cdb' *.lnk *.ihx
	rm -f .[a-z]*~
	rm -f s51 s51.exe
	rm -f ucsim_51 ucsim_51.exe ucsim_i8051

sub_clean:
ifneq ($(shell test -f test/Makefile && echo ok), )
	$(MAKE) -C test -f clean.mk clean
endif

distclean: local_distclean sub_distclean

local_distclean: local_clean
	rm -f config.cache config.log config.status
	rm -f Makefile *.dep
	rm -f test/Makefile

sub_distclean:
