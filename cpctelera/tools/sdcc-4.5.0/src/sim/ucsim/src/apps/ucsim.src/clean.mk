
clean: local_clean sub_clean

local_clean:
	rm -f *core *[%~] *.[oa] *.map
	rm -f .[a-z]*~
	rm -f ucsim$(EXEEXT) ucsim.exe

sub_clean:

distclean: local_distclean sub_distclean

local_distclean: local_clean
	rm -f config.cache config.log config.status
	rm -f Makefile *.dep
	rm -f *.obj *.list *.lst *.hex
	rm -f test/Makefile

sub_distclean:
