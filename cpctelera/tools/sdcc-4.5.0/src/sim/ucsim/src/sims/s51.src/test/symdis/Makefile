OUTPUTS = 8751 DS390F


# Ignore changes in white space. The field widths for labels
# depends on the length of the longest label. This will be
# different for different simulators and should not be assumed
# to be constant for any given simulator.
DIFF_OPTS += -b


$(OUTPUTS): %:	test-%.ihx test-%.cmd
	$(call run-sim, -t '$@')


include test-lib.mk
