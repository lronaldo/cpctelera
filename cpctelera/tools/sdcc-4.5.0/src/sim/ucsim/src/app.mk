simulator: $(LOCAL_TARGETS) $(LEN)


install: all installdirs local_install
	$(INSTALL) $(LEN) $(DESTDIR)$(bindir)/$(LEN)
	$(STRIP) $(DESTDIR)$(bindir)/$(LEN)
#rm -f $(DESTDIR)$(bindir)/$(SEN)

uninstall: local_uninstall
	rm -f $(DESTDIR)$(bindir)/$(SEN)
	rm -f $(DESTDIR)$(bindir)/$(LEN)
	rm -f $(DESTDIR)$(libdir)/$(SONAME)

installdirs:
	test -d $(DESTDIR)$(bindir) || $(INSTALL) -d $(DESTDIR)$(bindir)


.PHONY:	test baseline
test:
	@$(MAKE) --no-print-directory -C test

baseline:
	@$(MAKE) --no-print-directory -C test baseline


$(LEN): $(OBJECTS) $(UCSIM_LIB_PREREQUES) $(LOCAL_LIB_PREREQU)
ifeq ($(SILENT),yes)
	@echo LNK-$(PKG)
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) -L$(top_builddir) $(LOCAL_LIB_OPTIONS) $(UCSIM_LIBS) -o $@
else
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) -L$(top_builddir) $(LOCAL_LIB_OPTIONS) $(UCSIM_LIBS) -o $@
endif

ifeq ($(dlso_ok),yes)
shared_lib: $(top_builddir)/$(SONAME)
install_shared_lib: shared_lib
	test -d $(DESTDIR)$(libdir) || $(INSTALL) -d $(DESTDIR)$(libdir)
	$(INSTALL) $(top_builddir)/$(SONAME) $(DESTDIR)$(libdir)/$(SONAME)
else
shared_lib:
install_shared_lib:
endif

$(top_builddir)/$(SONAME): $(OBJECTS_SHARED)
	$(CXX) -shared $(LDFLAGS) $(OBJECTS_SHARED) -o $@

otherlibs: $(UCSIM_LIB_TARGETS) $(LOCAL_LIB_PREREQU)

$(top_builddir)/libcmd.a:
	$(MAKE) -C $(top_builddir)/src/core/cmd.src libs

cmd_lib:
	$(MAKE) -C $(top_builddir)/src/core/cmd.src libs

$(top_builddir)/libgui.a:
	$(MAKE) -C $(top_builddir)/src/core/gui.src libs

gui_lib:
	$(MAKE) -C $(top_builddir)/src/core/gui.src libs

$(top_builddir)/libsim.a:
	$(MAKE) -C $(top_builddir)/src/core/sim.src libs

sim_lib:
	$(MAKE) -C $(top_builddir)/src/core/sim.src libs

$(top_builddir)/libucsimutil.a:
	$(MAKE) -C $(top_builddir)/src/core/utils.src libs

ucsimutil_lib:
	$(MAKE) -C $(top_builddir)/src/core/utils.src libs

$(top_builddir)/libmotorola.a:
	$(MAKE) -C $(top_builddir)/src/core/motorola.src libs

motorola_lib:
	$(MAKE) -C $(top_builddir)/src/core/motorola.src libs

UCSIM_LIB_TARGETS = cmd_lib gui_lib sim_lib ucsimutil_lib motorola_lib

# Creating dependencies
# ---------------------
dep: Makefile.dep

Makefile.dep: $(srcdir)/*.cc $(srcdir)/*.h objs.mk
ifeq ($(SILENT),yes)
	@echo DEP-$(PKG)
	@$(MAKEDEP) $(CPPFLAGS) $(filter %.cc,$^) >Makefile.dep
else
	$(MAKEDEP) $(CPPFLAGS) $(filter %.cc,$^) >Makefile.dep
endif

-include Makefile.dep
include $(srcdir)/clean.mk
