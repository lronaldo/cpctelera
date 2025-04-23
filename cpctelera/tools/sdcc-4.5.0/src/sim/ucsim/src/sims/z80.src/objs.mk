PKG		= z80

OBJECTS		= sz80.o \
		  glob.o \
		  inst.o \
		  inst_cb.o \
		  inst_dd.o \
		  inst_ed.o \
		  inst_fd.o \
		  inst_ddcb.o \
		  inst_fdcb.o \
		  glob_gb80.o inst_gb80.o gb80.o lr35902.o \
		  glob_r800.o r800.o \
		  simz80.o z80.o ez80.o
