OUTPUTS = 51


$(OUTPUTS): %:	test-%.cmd
	$(call run-sim, -t $@)


include test-lib.mk
