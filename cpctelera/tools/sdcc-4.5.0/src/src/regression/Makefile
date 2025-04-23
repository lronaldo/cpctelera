# Regression testing Makefile for Pic Port of SDCC
#
# note that this regression suite was started before
# the one in sdcc/regression. The regression suite in
# sdcc/regression is better suited for testing mature
# ports.
#
# GPL'd
#
# T. Scott Dattalo scott@dattalo.com
#
# This makefile provides a means by which the output
# of the SDCC Compiler can be tested. This version
# is unique to the PIC (as in Microchip PIC) port.
# As such it requires the following software:
#
#  gpasm version 0.11.1 (or greater)
#  gpsim version 0.20.7 (or greater)
#
# Usage: make without options shows it

############################################################
# Configuration

# Uncomment this to show only errors.
# Comment this out for debugging.
.SILENT:

# default verbose mode
VERBOSE ?= 0

# default debug mode
DEBUG ?= 0

# List of targets
# You can prefix any target name with a dash to disable it
TARGETS ?= $(sort $(filter pic%, pic14 pic14e -pic14o -pic14eo -pic14x -pic14ex -pic14ox -pic14eox -pic16 ))

# List of C files to be tested
TESTS ?= $(sort $(notdir $(wildcard $(srcdir)/*.c)))

############################################################
# Directories

builddir = .
top_builddir = $(builddir)/../../../..

srcdir := $(dir $(lastword $(MAKEFILE_LIST)))
top_srcdir = $(srcdir)/../..

############################################################
# Other settings

# The session is saved here:
SESSIONLOG = make.log

# Results of the test are placed here:
LOGFILE = test.log

# Script file for creating gpsim scripts
CREATESTC = create_stc

# Script file for invoking gpsim
SIMULATE = simulate

STC = $(patsubst %.c, %.stc, $(TESTS))

############################################################
# Architecture and target specific settings

# pic16 architecture
ifneq (,$(filter pic16,$(TARGET)))
ARCH = pic16
DEV ?= 18f452
endif

# pic14 architecture
ifneq (,$(filter pic14%,$(TARGET)))
ARCH = pic14

# regular/enhanced core
LIB_E = $(findstring e,$(TARGET))

# optimizations enabled/disabled
LIB_O = $(findstring o,$(TARGET))

# experimental code disabled/enabled
LIB_X = $(findstring x,$(TARGET))

ifeq (e,$(strip $(LIB_E)))
# pic14 enhanced device
# the default device for enhanced cores in SDCC is the 16f1934,
# but it is not supported by gpsim, so we use here the 16f1788,
# which in fact has more ROM and RAM memory
DEV ?= 16f1788
else
# pic14 non-enhanced device
DEV ?= 16f877
endif

endif

############################################################
# Toolchain

# toolchain
CC = $(top_builddir)/bin/sdcc

# compiler options
CFLAGS =  --no-warn-non-free -m$(ARCH) -p$(DEV) --fsigned-char --i-code-in-asm --fverbose-asm --std-c99
CFLAGS += --nostdinc -I$(top_srcdir)/device/include/$(ARCH) -I$(top_srcdir)/device/non-free/include/$(ARCH) -I$(top_srcdir)/device/include

# linker options (for sdcc)
CFLAGS += -Wl,-l,-O2
CFLAGS += --nostdlib -L$(top_builddir)/device/lib/build/$(ARCH) -L$(top_builddir)/device/non-free/lib/build/$(ARCH)

# linker libraries
LIB_SUFFIX = $(LIB_E)$(LIB_O)$(LIB_X)
CFLAGS += -l libsdcc$(LIB_SUFFIX).lib

############################################################
# Build alternatives

ifeq (e,$(strip $(LIB_E)))
# pic14 enhanced architecture
#CFLAGS += --no-extended-instructions
else
# pic14 non-enhanced architecture
endif

ifeq (o,$(strip $(LIB_O)))
# disable optimizations
CFLAGS += -D__SDCC_PIC14_NOOPTS
CFLAGS += --no-pcode-opt --no-peep
else
# enable optimizations
#CFLAGS += --opt-code-size
endif

ifeq (x,$(strip $(LIB_X)))
# enable experimental options
CFLAGS += -D__SDCC_PIC14_EXPERIMENTAL
CFLAGS += --experimental-pcall-all --experimental-pcall-enhanced --experimental-variadics --experimental-ralloc
# gpsim fails to place breakpoints when gplink optimizations are enabled
#CFLAGS += -Wl,-b1,-p1,-B,-P,-W
ifeq (1,$(strip $(DEBUG)))
CFLAGS += --debug-variadics
endif
else
# disable experimental options
endif

ifeq (1,$(strip $(VERBOSE)))
# enable verbose
Q ?=
CFLAGS += -V --verbose
else
# disable verbose
Q ?= @
#CFLAGS += -Wl,-q
endif

ifeq (1,$(strip $(DEBUG)))
# enable debug
CFLAGS += --debug --debug-xtra --debug-glue --debug-ralloc
CFLAGS += -Wl,-m
# this gplink option dumps too much data. not enabled by default.
#CFLAGS += -Wl,--debug
else
# disable debug
#CFLAGS += -Wl,-q
endif

############################################################
# Rules

all: help

# make without arguments shows this help
help:
	@echo "make test           test all targets"
	@echo "make test-TARGET    test one target"
	@echo "make clean          clean all targets"
	@echo "make clean-TARGET   clean one target"
	@echo
	@echo "Available targets: $(TARGETS)"

# Test all targets
test: $(patsubst %,test-%,$(TARGETS))
	@echo "Summary of all targets:"
	@for target in $(TARGETS) ; do \
	    printf "%-20s %s\n" $$target "` grep Total tests/$$target/$(SESSIONLOG) `" ; \
	done

# Clean all targets
clean: $(patsubst %,clean-%,$(TARGETS))

# Rules for each target: test-TARGET and clean-TARGET

define target-rules
test-$(1): Makefile $$(TESTS)
	@echo "Testing target $(1)"
	@[ -d tests ] || mkdir tests
	@[ -d tests/$(1) ] || mkdir tests/$(1)
	$$(Q)$$(MAKE) TARGET=$(1) -f ../../Makefile -C tests/$(1) test-target 2>&1 | tee tests/$(1)/$$(SESSIONLOG)

clean-$(1):
	@if [ -d tests/$(1) ] ; then \
	    echo "Cleaning target $(1)" ; \
	    rm -rf tests/$(1) ; \
	    if [ ` ls tests | wc -l ` -eq 0 ] ; then \
	        rmdir tests ; \
	    fi ; \
	fi

endef

$(foreach TARGET,$(TARGETS),$(eval $(call target-rules,$(TARGET))))

# Remove noise from gplink
FILTER_NOISE = | grep -v '^message: Using default linker script'

# The .cod files are generated by sdcc
%.cod: $(srcdir)/%.c
	$(Q)-$(CC) $(CFLAGS) $< $(FILTER_NOISE) 2>&1 | tee $*.log

# The .stc files are script files for gpsim
%.stc: %.cod
	$(Q)-$(srcdir)/$(CREATESTC) $< $@
	$(Q)-$(srcdir)/$(SIMULATE) $@ $(LOGFILE)

# Show info about the test before starting it
test-info:
	@echo "TARGET: $(TARGET) ARCH: $(ARCH) DEV: $(DEV) LIB: libsdcc$(LIB_SUFFIX).lib"
	@echo "LOGFILE: $(LOGFILE) SESSIONLOG: $(SESSIONLOG)"
	@echo "srcdir: $(srcdir)"
	@echo "top_srcdir: $(top_srcdir)"
	@echo "builddir: $(builddir)"
	@echo "top_builddir: $(top_builddir)"
	@echo "TEST_DIR: $(shell pwd)"
	@echo "TESTS: $(TESTS)"
	@echo "CFLAGS: $(CFLAGS)"

# Run all tests for one target
ifeq (1,$(strip $(VERBOSE)))
test-target: test-info $(STC)
	@echo "Done - Total tests: ` cat $(LOGFILE) | wc -l ` Failures: ` grep FAILED $(LOGFILE) | wc -l `"
	@echo "Done - Each TEST is logged in tests/$(TARGET)/TEST.log"
	@echo "Done - This session has been saved in tests/$(TARGET)/$(SESSIONLOG)"
	@echo "Done - Results summary is in tests/$(TARGET)/$(LOGFILE)"
else
test-target: $(STC)
	@echo "Done - Total tests: ` cat $(LOGFILE) | wc -l ` Failures: ` grep FAILED $(LOGFILE) | wc -l `"
endif

.SUFFIXES: .asm .c .cod .stc
.NOTPARALLEL:
.PRECIOUS: %.cod
.PHONY: all test clean help $(patsubst %,test-% clean-%,$(TARGETS)) test-info test-target

