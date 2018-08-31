.PHONY: clean cleanall all
CC   = gcc
BIND = gcc
RM   = rm
OBJ  = obj
SRC  = src
BIN  = bin

CFLAGS  = -O3 -fsigned-char -Wall -DUNIX -pedantic

TARGET = $(BIN)/zx7b
FILES  = zx7b
F_OBJ  = $(foreach F, $(FILES), $(OBJ)/$(F).o)

all: Makefile $(OBJ) $(F_OBJ) $(TARGET)

$(TARGET): $(F_OBJ) Makefile 
	$(BIND) $(F_OBJ) -o $(TARGET) $(CFLAGS)

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
