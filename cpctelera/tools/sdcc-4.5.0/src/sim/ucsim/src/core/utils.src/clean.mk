
clean: local_clean sub_clean

local_clean:
	rm -f *core *[%~] *.[oa] *.so ucsim.map
	rm -f  ucsim$(EXEEXT) relay$(EXEEXT) ucsim.exe relay.exe
	rm -f .[a-z]*~ ptt

sub_clean:

distclean: local_distclean sub_distclean

local_distclean: local_clean
	rm -f config.cache config.log config.status
	rm -f ddconfig.h main.mk Makefile *.dep packages.mk
	rm -rf autom4te.cache
	rm -f GPATH GRTAGS GSYMS GTAGS

sub_distclean:
