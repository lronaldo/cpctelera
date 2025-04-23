PKG		= mos6502

OBJECTS		= smos6502.o \
		  simmos6502.o mos6502.o glob.o irq.o \
		  inst.o ialu.o ibranch.o imove.o \
		  mos65c02.o mos65c02s.o mos6510.o mos65ce02.o \
		  port10.o
