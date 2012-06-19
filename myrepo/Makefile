CC=gcc
IDIR=../include
SDIR=../shared
OBJ=../obj
BIN=../bin
CFLAGS=-I$(IDIR) -Ofast -DNDEBUG -Wall -Wextra -pedantic --std=c99

.PHONY: build directories clean
build: directories $(BIN)/myrepo

directories:
	@mkdir -p $(BIN)
	@mkdir -p $(OBJ)

clean:
	@rm -rf $(OBJ) $(BIN)

$(OBJ)/%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ)/%.o: $(SDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(BIN)/myrepo: $(OBJ)/myrepo.o $(OBJ)/usage.o $(OBJ)/init.o
	$(CC) -o $@ $^ $(CFLAGS)