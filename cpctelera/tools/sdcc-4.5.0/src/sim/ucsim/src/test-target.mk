SHELL := /bin/sh
VPATH := $(srcdir)

# Prefer built versions of the compiler, assemblers etc. but use installed versions if necessary.
export PATH := $(abs_top_builddir)/bin:$(PATH)

# We ONLY test the simulator version just built, never something installed.
SIM := $(basename $(notdir $(patsubst %/,%,$(dir $(srcdir)))))
ifeq ($(SIM), sim)
	# If we are testing the core simulator itself rather than a specific
	# version then any simulator will do. As long as it's been built.
	SIM := $(firstword \
			$(foreach p, $(basename $(notdir $(sort $(wildcard $(abs_top_builddir)/*.src)))), \
				$(wildcard $(abs_top_builddir)/$(p).src/s$(p)) $(wildcard $(abs_top_builddir)/$(p).src/$(p)) \
			) \
		)
else
ifeq ($(SIM), s51)
	SIM := $(abs_top_builddir)/$(SIM).src/ucsim_51
else
	SIM := $(abs_top_builddir)/$(SIM).src/ucsim_$(SIM)
endif
endif


TESTS := $(patsubst $(srcdir)/%/Makefile,%,$(wildcard $(srcdir)/*/Makefile))
BASELINES := $(patsubst %,%-baseline,$(TESTS))
CLEAN_TESTS := $(addprefix Makefile/,$(TESTS))


.PHONY:	all always clean $(CLEAN_TESTS)

all::		$(SIM) $(TESTS)

# It is not our place to build the simulator.
$(SIM):
	@echo 'FAIL $(SIM) does not exist. Build it before trying to test it!'
	@exit 1

baseline:	$(BASELINES) always

%-baseline:	% always
	@echo 'BASELINE $(srcdir)/$<'
	@test -d '$(srcdir)/$</baseline' || mkdir '$(srcdir)/$</baseline'
	@cp '$</out'/* '$(srcdir)/$</baseline'/.

clean::	$(CLEAN_TESTS)

$(CLEAN_TESTS):
	@-rm -fr $(@F)/*.{rel} $(@F)/out
#@-$(MAKE) $(if $(MAKECMDGOALS),,-s) -C '$(@F)' -f '$(srcdir)/$(@F)/Makefile' -I '$(abs_top_srcdir)' srcdir='$(srcdir)/$(@F)' SIM='$(SIM)' clean


%::	%.mk always
	@$(MAKE) -f '$<' srcdir='$(srcdir)'

%::	%/Makefile always
	@+test -d '$@' || mkdir '$@'
	@$(MAKE) -Otarget $(if $(subst baseline,,$(MAKECMDGOALS)),,-s) -C '$@' -f '$(abspath $<)' -I '$(abs_top_srcdir)' srcdir='$(srcdir)/$@' SIM='$(SIM)' all || true


TESTS_OLD := $(basename $(filter-out sdcc.mk clean.mk, $(notdir $(wildcard $(srcdir)/*.mk))))

.PHONY:	all_old clean_old

all_old::	$(TESTS_OLD)

clean_old::
	-for p in $(TESTS_OLD); do \
		$(MAKE) MAIN=$$p srcdir='$(srcdir)' -f '$(srcdir)'/$$p.mk clean; \
	done
	$(MAKE) -f '$(srcdir)'/clean.mk clean; \
	rm -f *.out *.sout *.tim *.csv *.sim
	rm -f st*[56][0-9][0-9].cmd st*[56]*.txt
