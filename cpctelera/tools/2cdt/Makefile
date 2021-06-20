# Makefile for 2cdt utility

.PHONY: clean cleanall all
CC   = gcc
BIND = gcc
RM   = rm
OBJ  = obj
SRC  = src
BIN  = bin

#   CFLAGS    flags for C compile
#   LFLAGS1   flags after output file spec, before obj file list
#   LFLAGS2   flags after obj file list (libraries, etc)

CFLAGS  = -O3 -fsigned-char -DUNIX -Wall -pedantic
LFLAGS1 =
LFLAGS2 = -s 

TARGET = $(BIN)/2cdt
FILES  = 2cdt tzxfile
F_OBJ  = $(foreach F, $(FILES), $(OBJ)/$(F).o)

all: Makefile $(OBJ) $(F_OBJ) $(TARGET)

$(TARGET): $(F_OBJ) Makefile 
	$(BIND) $(F_OBJ) -o $(TARGET) $(LFLAGS1) $(LFLAGS2) $(LIBS)

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) $< -c -o $@

$(OBJ):
	@echo "Creating $(OBJ)/ intermediate folder..."
	@mkdir -p $(OBJ)

clean:
	@echo "Removing $(OBJ)/ intermediate folder..."
	@rm -rf obj

cleanall: clean
	@echo "Removing $(TARGET)..."
	@rm -f $(TARGET)
