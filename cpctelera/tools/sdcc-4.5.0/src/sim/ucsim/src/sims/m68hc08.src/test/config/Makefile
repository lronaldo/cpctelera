OUTPUTS = $(filter-out Parameter, $(shell $(SIM) -H | sort -u -k2b | cut -d' ' -f1))


$(OUTPUTS):
	$(call run-sim, -t $(@F) -e 'conf; info reg; info mem')


include test-lib.mk
