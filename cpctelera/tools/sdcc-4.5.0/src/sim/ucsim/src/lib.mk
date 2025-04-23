
include $(srcdir)/objs.mk

libs: $(top_builddir)/lib$(PKG).a


# Compiling and installing everything
# -----------------------------------
installdirs:

install:

uninstall:


# Creating dependencies
# ---------------------
dep: Makefile.dep

Makefile.dep: $(SOURCES) $(srcdir)/*.h objs.mk
ifeq ($(SILENT),yes)
	@echo DEP $(PKG)
	@$(MAKEDEP) $(CPPFLAGS) $(filter %.cc,$^) >Makefile.dep
else
	$(MAKEDEP) $(CPPFLAGS) $(filter %.cc,$^) >Makefile.dep
endif

-include Makefile.dep
include $(srcdir)/clean.mk


# My rules
# --------

$(top_builddir)/lib$(PKG).a: $(ALL_OBJECTS) objs.mk
ifeq ($(SILENT),yes)
	@echo LIB $(PKG)-$@
	@$(AR) -rc $@ $(ALL_OBJECTS)
	@$(RANLIB) $@
else
	$(AR) -rc $@ $(ALL_OBJECTS)
	$(RANLIB) $@
endif

