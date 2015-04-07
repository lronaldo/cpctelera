#
# Makefile targets to remake configuration
#

freshconf: Makefile

Makefile: $(srcdir)/Makefile.in $(top_builddir)/configure.in
	cd $(top_builddir) && $(SHELL) ./config.status

# End of conf.mk
