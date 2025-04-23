
clean: local_clean sub_clean

local_clean:
	rm -f *core *[%~] *.[oa]
	rm -f .[a-z]*~
	rm -f spblaze spblaze.exe
	rm -f ucsim_pblaze ucsim_pblaze.exe
	rm -f ucsim_spblaze ucsim_spblaze.exe

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
