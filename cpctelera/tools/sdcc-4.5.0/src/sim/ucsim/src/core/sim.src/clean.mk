
clean: local_clean sub_clean

local_clean:
	rm -f *core *[%~] *.[oa] test_mem_speed
	rm -f .[a-z]*~

sub_clean:
	$(MAKE) -C test -f clean.mk clean

distclean: local_distclean sub_distclean

local_distclean: local_clean
	rm -f Makefile *.dep
	rm -f test/Makefile

sub_distclean:
	$(MAKE) -C test -f clean.mk distclean
