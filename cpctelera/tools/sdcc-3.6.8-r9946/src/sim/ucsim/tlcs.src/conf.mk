# tlcs.src/conf.mk
#
# Makefile targets to remake configuration
#
# $Id: conf.mk 214 2016-06-18 08:00:47Z  $

freshconf: Makefile

Makefile: $(srcdir)/Makefile.in $(top_srcdir)/configure.ac
	cd $(top_builddir) && $(SHELL) ./config.status

# End of tlcs.src/conf.mk
