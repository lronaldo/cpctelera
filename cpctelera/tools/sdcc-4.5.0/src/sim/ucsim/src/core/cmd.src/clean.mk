
clean: local_clean sub_clean

local_clean:
	rm -f *core *[%~] *.[oa]
	rm -f .[a-z]*~

sub_clean:

distclean: local_distclean sub_distclean

local_distclean: local_clean
	rm -f config.cache config.log config.status
	rm -f Makefile *.dep
	rm -f cmdpars.cc cmdpars.hh cmdlex.cc

sub_distclean:
