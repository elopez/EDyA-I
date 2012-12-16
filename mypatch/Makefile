CC=gcc
IDIR=../include
SDIR=../shared
OBJ=../obj
BIN=../bin
CFLAGS=-I$(IDIR) -D_POSIX_C_SOURCE=200809L --std=c99

ifeq ($(DEBUG),)
    CFLAGS+= -DNDEBUG -O2
else
    CFLAGS+=-O0 -g -Wall -Wextra -pedantic
    CC=$(shell which cgcc > /dev/null && echo cgcc || echo gcc)
endif

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

$(BIN)/mypatch: $(OBJ)/mypatch.o $(OBJ)/readfile.o $(OBJ)/salloc.o $(OBJ)/patch.o
	$(CC) -o $@ $^ $(CFLAGS)
