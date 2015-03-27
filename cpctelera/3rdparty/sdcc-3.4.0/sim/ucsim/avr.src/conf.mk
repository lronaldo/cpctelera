# avr.src/conf.mk
#
# Makefile targets to remake configuration
#

freshconf: Makefile

Makefile: $(srcdir)/Makefile.in $(top_srcdir)/configure.in
	cd $(top_builddir) && $(SHELL) ./config.status

# End of avr.src/conf.mk
