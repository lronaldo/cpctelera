PRJ		= test_mdu517 test_mdu88x \
		  stn stp strr sts stt stxr stxrw stxw \
		  stt_small stt_large \
		  monitor test_ser tim_ser stck_ovf

all:
	for p in $(PRJ); do \
		$(MAKE) -f $$p.mk clean all ;\
	done

clean:
	for p in $(PRJ); do \
		$(MAKE) -f $$p.mk clean ;\
	done
	rm -f *.out *.sout *.tim *.csv *.sim
	rm -f st*[56][0-9][0-9].cmd st*[56]*.txt
