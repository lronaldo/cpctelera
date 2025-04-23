OUTPUTS = stdout


# Ignore changes in white space. The field widths for labels
# depends on the length of the longest label. This will be
# different for different simulators and should not be assumed
# to be constant for any given simulator.
DIFF_OPTS += -b


stdout:	test.ihx test.cmd
	$(call run-sim)


%.p:	%.asm
	asl -L -h -U -olist '$*.lst' -o '$@' '$<'

%.ihx:	%.p
	p2hex -F Intel '$<' '$@'


include test-lib.mk

