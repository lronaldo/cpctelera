CC = sdcc

CFLAGS= -Wa-s --dumpall
MFLAGS= -mxa51
#LFLAGS= --code-loc 0x1000 --xram-loc 0x4000
LFLAGS= --xram-loc 0x4000

all: hello.hex

OBJECTS = hello.rel hwinit.rel dummy.rel

hello.hex: $(OBJECTS)
	$(CC) $(MFLAGS) $(LFLAGS) $(OBJECTS)
	echo go | sxa hello.hex

clean:
	rm -f core *~ \#* *.asm *.cdb *.rel *.hex *.ihx *.lst *.map \
		*.xa *.rst *.sym *.lnk *.lib *.bin *.dump* *.log

%.rel: %.c hw.h
	$(CC) -c $(CFLAGS) $(MFLAGS) $<
