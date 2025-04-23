OUTPUTS = stdout


# Ignore changes in white space. The field widths for labels
# depends on the length of the longest label. This will be
# different for different simulators and should not be assumed
# to be constant for any given simulator.
DIFF_OPTS += -b


stdout:	xconnect.ihx xconnect.vars test.cmd
	@$(call run-sim, -t S003 '$(srcdir)/xconnect.ihx' -e 'exec "$(srcdir)/xconnect.vars"')


include test-lib.mk
