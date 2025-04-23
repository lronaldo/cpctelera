
all:
	$(MAKE) -C src all

libs:
	$(MAKE) -C src libs

pkgs:
	$(MAKE) -C src pkgs

sims: pkgs

apps:
	$(MAKE) -C src apps

install:
	$(MAKE) -C src install
	$(MAKE) -C docs install

uninstall:
	$(MAKE) -C src uninstall
	$(MAKE) -C docs uninstall

clean: local_clean sub_clean

local_clean:
	rm -f *~ lib*.a

sub_clean:
	$(MAKE) -C src  -f clean.mk clean
	$(MAKE) -C docs -f clean.mk clean

distclean: local_distclean sub_distclean

local_distclean: local_clean
	rm -f config.log config.status
	rm -rf autom4te.cache
	rm -f Makefile

sub_distclean:
	$(MAKE) -C src  -f clean.mk distclean
	$(MAKE) -C docs -f clean.mk distclean

conf: configure

configure: src/configure.ac
	autoconf $< >$@
