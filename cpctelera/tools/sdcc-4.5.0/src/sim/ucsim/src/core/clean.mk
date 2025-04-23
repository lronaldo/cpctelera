
clean: local_clean sub_clean

local_clean:
	rm -f *~ lib*.a

sub_clean:
	$(MAKE) -C cmd.src      -f clean.mk clean
	$(MAKE) -C gui.src      -f clean.mk clean
	$(MAKE) -C motorola.src -f clean.mk clean
	$(MAKE) -C sim.src      -f clean.mk clean
	$(MAKE) -C utils.src    -f clean.mk clean

distclean: local_distclean sub_distclean

local_distclean: local_clean

sub_distclean:
	$(MAKE) -C cmd.src      -f clean.mk distclean
	$(MAKE) -C gui.src      -f clean.mk distclean
	$(MAKE) -C motorola.src -f clean.mk distclean
	$(MAKE) -C sim.src      -f clean.mk distclean
	$(MAKE) -C utils.src    -f clean.mk distclean
