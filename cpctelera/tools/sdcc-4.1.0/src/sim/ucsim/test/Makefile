PROJECTS	= t1 stck_ovf

.PHONY: projects $(PROJECTS)

projects: $(PROJECTS)

$(PROJECTS):
	$(MAKE) MAIN=$@ -f sdcc.mk

clean:
	for p in $(PROJECTS); do \
		$(MAKE) MAIN=$$p -f sdcc.mk clean ;\
		rm -rf $$p*.hex ;\
		rm -rf $$p*.cdb ;\
	done
