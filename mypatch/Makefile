CC=gcc
IDIR=../include
SDIR=../shared
OBJ=../obj
BIN=../bin
CFLAGS=-I$(IDIR) -Ofast -DNDEBUG -Wall -Wextra -pedantic --std=c99

.PHONY: build directories clean
build: directories $(BIN)/mypatch

directories:
	@mkdir -p $(BIN)
	@mkdir -p $(OBJ)

clean:
	@rm -rf $(OBJ) $(BIN)

$(OBJ)/%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ)/%.o: $(SDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(BIN)/mypatch: $(OBJ)/mypatch.o $(OBJ)/readfile.o $(OBJ)/patch.o
	$(CC) -o $@ $^ $(CFLAGS)
