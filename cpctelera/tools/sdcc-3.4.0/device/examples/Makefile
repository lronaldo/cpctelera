
#DO_GCC=1
DO_Z80=1
#DO_MCS51=1

ifdef DO_GCC
CC = gcc -Wall -g
endif

ifdef DO_Z80
OBJEXT=o
CC = sdcc
MFLAGS = --debug -mz80
endif

ifdef DO_MCS51
OBJEXT=rel
CC = sdcc
MFLAGS = --debug --model-small --stack-after-data
#CFLAGS = -V
#MFLAGS = --debug --model-large --stack-after-data
#LFLAGS = --xram-loc 0x4000 --code-loc 0x0000
endif


#OBJECTS = test3.c itoa.$(OBJEXT) crc.$(OBJEXT)
OBJECTS = test3.c itoa.$(OBJEXT)

all: test3.ihx

ifdef DO_MCS51
an429.ihx : an429.c
	sdcc --model-small an429.c

test2.ihx : test2.c
	sdcc --model-small test2.c
endif

%.$(OBJEXT): %.c
	$(CC) -c $(CFLAGS) $(MFLAGS) $<

itoa.rel : itoa.c
	$(CC) -c $(CFLAGS) $(MFLAGS) itoa.c

#crc.rel : crc.c
#	$(CC) -c $(CFLAGS) $(MFLAGS) crc.c

test3.ihx: $(OBJECTS)
	$(CC) $(MFLAGS) $(LFLAGS) $(OBJECTS)

test3.bin: test3.ihx
	makebin < test3.ihx > test3.bin

clean:
	rm -f core *~ \#* *.asm *.cdb *.rel *.hex *.ihx *.lst *.map *.o \
		*.rst *.sym *.lnk *.lib *.bin

testser: test3.ihx
	s51 -Sout=serial.log test3.ihx

