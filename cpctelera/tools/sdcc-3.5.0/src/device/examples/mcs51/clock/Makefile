CC = sdcc
#CFLAGS = -V
MFLAGS = --model-small --debug
#MFLAGS = --model-large
LFLAGS = --xram-loc 0x4000 --code-loc 0x0000

all: clock.bin

OBJECTS = clock.rel hwinit.rel

clock.bin: clock.ihx
	hex2bin <clock.ihx >clock.bin
	cat clock.bin >/dev/par0
	cu -l /dev/ttyS0 -s 19200 dir

clock.ihx: $(OBJECTS)
	$(CC) $(MFLAGS) $(LFLAGS) $(OBJECTS)

clean:
	rm -f core *~ \#* *.asm *.cdb *.rel *.hex *.ihx *.lst *.map \
		*.rst *.sym *.lnk *.lib *.bin

%.rel: %.c hw.h
	$(CC) -c $(CFLAGS) $(MFLAGS) $<
