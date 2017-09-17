PRJ		= simif banking

all:
	for p in $(PRJ); do \
		$(MAKE) -f $$p.mk; \
	done

clean:
	for p in $(PRJ); do \
		$(MAKE) -f $$p.mk clean; \
	done

#all: simif.ihx
#
#simif.ihx: simif.c
#	sdcc --model-large --debug simif.c
#
#clean:
#	rm -f *.asm *.ihx *.lk *.lst *.map *.mem *.rel *.rst *.sym
#	rm -f *.adb *.cdb *.omf
#	rm -f simif.out simif_fout.txt
