CC=gcc
IDIR=../include
SDIR=../shared
OBJ=../obj
BIN=../bin
CFLAGS=-I$(IDIR) -Ofast -DNDEBUG -Wall -Wextra -pedantic --std=c99

.PHONY: build directories clean
build: directories $(BIN)/mydiff

directories:
	mkdir $(BIN)
	mkdir $(OBJ)

clean:
	rm -rf $(OBJ) $(BIN)

$(OBJ)/%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ)/%.o: $(SDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(BIN)/mydiff: $(OBJ)/mydiff.o $(OBJ)/readfile.o $(OBJ)/diff.o
	$(CC) -o $@ $^ $(CFLAGS)
