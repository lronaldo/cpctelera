
clean: local_clean sub_clean

local_clean:
	rm -f *core *[%~] *.[oa] *.map
	rm -f .[a-z]*~
	rm -f srxk$(EXEEXT) srxk.exe
	rm -f ucsim_rxk$(EXEEXT) ucsim_rxk.exe
	rm -f gen_insts gen_wrap

sub_clean:
ifneq ($(shell test -f test/Makefile && echo ok), )
	$(MAKE) -C test -f clean.mk clean
endif

distclean: local_distclean sub_distclean

local_distclean: local_clean
	rm -f config.cache config.log config.status
	rm -f Makefile *.dep
	rm -f *.obj *.list *.lst *.hex
	rm -f test/Makefile

sub_distclean:
