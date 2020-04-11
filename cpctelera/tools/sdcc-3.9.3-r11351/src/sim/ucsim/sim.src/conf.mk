#
# Makefile targets to remake configuration
#

freshconf: Makefile

Makefile: $(srcdir)/Makefile.in $(top_builddir)/configure.ac
	cd $(top_builddir) && $(SHELL) ./config.status

# End of conf.mk
