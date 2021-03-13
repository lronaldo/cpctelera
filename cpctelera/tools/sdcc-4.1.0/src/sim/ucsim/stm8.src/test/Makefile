PROJECTS	= s0 s1 s2 s3 t1 t2 t3 t4 xtest \
		  sl ss lled sled ltim stim sertest stck_ovf \
		  getbl \
		  monitor

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
