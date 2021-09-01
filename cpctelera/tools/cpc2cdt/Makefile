.PHONY: clean cleanall all
CC   = gcc
RM   = rm
OBJ  = obj
SRC  = src
BIN  = bin

CFLAGS  = -O3 -fsigned-char -Wall -DUNIX -pedantic -fcommon

TARGET = $(BIN)/cpc2cdt
FILES  = tinytape cpc2cdt
F_OBJ  = $(foreach F, $(FILES), $(OBJ)/$(F).o)

all: Makefile $(OBJ) $(BIN) $(F_OBJ) $(TARGET)

$(TARGET): $(F_OBJ) Makefile 
	$(CC) $(F_OBJ) -o $(TARGET) $(CFLAGS)

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) $< -c -o $@

$(OBJ):
	@echo "Creating $(OBJ)/ intermediate folder..."
	@mkdir -p $(OBJ)

$(BIN):
	@echo "Creating $(BIN)/ intermediate folder..."
	@mkdir -p $(BIN)

clean:
	@echo "Removing $(OBJ)/ intermediate folder..."
	@rm -rf ./$(OBJ)

cleanall: clean
	@echo "Removing $(TARGET)..."
	@rm -f $(TARGET)
