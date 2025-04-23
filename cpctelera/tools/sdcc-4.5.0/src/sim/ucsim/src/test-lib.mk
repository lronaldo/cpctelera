# Work for individual tests is not parallelized although tests
# themselves may be run in parallel.
.NOTPARALLEL:

VPATH := $(srcdir)

# Include any target specifics.
-include $(srcdir)/../test-conf.mk

# Naming and target options are inconsistent so nearly all simulators
# are going to have to define things in *.src/test-conf.mk some and
# individual tests are likely to have to override on a per-target
# basis using:
#     target: CC=... SDAS=... SDLD=...
#     target: prereqs
#             recipe
TARGET ?= $(patsubst s%,%,$(notdir $(SIM)))

CC ?= sdcc -m$(TARGET)
PACKIHX ?= packihx
AS ?= sdas$(TARGET)
ASFLAGS ?= -plosgffw
LD ?= sdld$(TARGET)
LDFLAGS ?=


# When to colourize the diff output. Unless otherwise overridden we use
# "auto" normally but "always" if output is being collected because make
# is running parallel jobs.
DIFF_COLOUR ?= $(if $(findstring --jobserver, $(MAKEFLAGS)),always,auto)

# Things that are expected to change are elided in the output. We don't
# care when comparing to baselines and we don't want to have to deal with
# these changes when merging or rebasing.
# N.B. These are sed arguments using extended regexps (sed -E)
ELIDE += -e 's/^(uCsim )[^,]*(, Copyright \(C\) )[[:digit:]]+(-[[:digit:]]+)?/\1[ELIDED]\2[ELIDED]/'
ELIDE += -e 's/^(ucsim version ).*/\1[ELIDED]/'
ELIDE += -e 's/^(Loading from ).*/\1[ELIDED]/'
ELIDE += -e 's/( words read from ).*/\1[ELIDED]/'
ELIDE += -e 's/( File: ).*/\1[ELIDED]/'
ELIDE += -e 's/^(Simulated [[:digit:]]+ ticks ).*/\1[ELIDED]/'
ELIDE += -e 's/^(Host usage: ).*/\1[ELIDED]/'
ELIDE += -e 's/([[:upper:]][[:alpha:]]{2} [[:upper:]][[:alpha:]]{2} .[[:digit:]] [[:digit:]]{2}:[[:digit:]]{2}:[[:digit:]]{2} [[:digit:]]{4})/[ELIDED]/'


# $(call run-sim,<sim args>)
# Prerequisites of the form %.cmd are passed as command scripts
# using -e immediately after any args specified.
# Prerequisites of the form %.ihx are given as image files to load.
# Other prerequisites are ignored.
# If there are no %.cmd prerequisites and -e does not appear anywhere
# in the sim args "-e run" is passed to the simulator.
define run-sim =
	-$(SIM) -q -R 0 $(SIM_ARGS) $(1) \
		$(if $(filter %.cmd, $+), \
			$(foreach file, $(filter %.cmd, $+), -e 'exec "$(file)"'), \
			$(if $(findstring -e, $(1)), , -g)) \
		$(filter %.ihx, $+) \
		2>&1 < /dev/null > 'out/$@'; \
	for file in out/*.vcd; do [ -r "$$file" ] && sed -E -i $(ELIDE) "$$file"; done
endef


.PHONY:	all clean _diff_to_baseline _log_test _silent

# silent is used to suppress "nothing to do" messages.
_silent:
	@echo -n

# A test produces one or more outputs which are then diff'd against
# the baseline saved in the source tree.
all:	_log_test out $(OUTPUTS) _diff_to_baseline

_log_test:
	@echo 'TEST $(srcdir)'

out:
	mkdir '$@'

# Outputs must, at least, be up to date with respect to the baseline
# output (if it exists), the main test's Makefile, test-lib.mk and the
# simulator being tested. Other dependencies and the recipe used for
# the test are in the main test's Makefile.
.PHONY: $(patsubst %,baseline/%,$(OUTPUTS))
$(OUTPUTS): %: baseline/% Makefile $(srcdir)/../../../test-lib.mk $(SIM)

# Compare the complete output with the saved baseline. If there are
# differences there is no need to report fail (the diff output is
# sufficient) but hide the error to keep make quiet and allow other
# tests to go ahead.
_diff_to_baseline:
	diff -urN --color='$(DIFF_COLOUR)' \
		$(DIFF_OPTS) \
		'$(srcdir)/baseline' 'out' \
	|| true

clean:	| _silent


MAKEDEPEND = $(CC) -MM $(CPPFLAGS) $< > $*.d.tmp

POSTCOMPILE = cp $*.d.tmp $*.d \
	&& while read line; do \
		for file in $${line\#*:}; do \
			echo "$$file:"; \
		done; \
	done < $*.d.tmp >> $*.d \
	&& rm -f $*.d.tmp


%.hex:	%.ihx
	$(PACKIHX) '$<' >'$@'

# Build rules assuming an sdcc tool-chain. Override in per-target
# test-conf.mk to use something else. Ideally pre-build tests and
# ship the .ihx - we are NOT testing tool-chains!
%.ihx:	%.c
	$(MAKEDEPEND)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) --out-fmt-ihx -o '$@' '$<'
	$(POSTCOMPILE)

%.rel:	%.asm
	$(AS) $(ASFLAGS) '$@' '$<'

%.ihx:	%.rel
	$(LD) $(LINKFLAGS) -i '$@' '$<'


include $(wildcard *.d)
