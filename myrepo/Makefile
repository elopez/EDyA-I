CC=gcc
IDIR=../include
SDIR=../shared
OBJ=../obj
BIN=../bin
CFLAGS=-I$(IDIR) -D_POSIX_C_SOURCE=200809L --std=c99

ifeq ($(DEBUG),)
    CFLAGS+= -DNDEBUG -Ofast -DSPEC_COMPLIANT
else
    CFLAGS+=-O0 -g -Wall -Wextra -pedantic
    CC=$(shell which cgcc > /dev/null && echo cgcc || echo gcc)
endif

.PHONY: build directories clean
build: directories $(BIN)/myrepo

directories:
	@mkdir -p $(BIN)
	@mkdir -p $(OBJ)

clean:
	@rm -rf $(OBJ) $(BIN)

# Don't use -pedantic on the gcc sha1 code
$(OBJ)/sha1.o: sha1.c
	$(CC) -c -o $@ $< -I$(IDIR) -Ofast

$(OBJ)/%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ)/%.o: $(SDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(BIN)/myrepo: $(OBJ)/usage.o $(OBJ)/init.o $(OBJ)/recursive.o \
               $(OBJ)/catalog.o $(OBJ)/sha1.o $(OBJ)/hash.o \
               $(OBJ)/hashtree.o $(OBJ)/commit.o $(OBJ)/log.o \
               $(OBJ)/patch.o $(OBJ)/diff.o $(OBJ)/readfile.o \
               $(OBJ)/mkpath.o $(OBJ)/cleanup.o $(OBJ)/pager.o \
               $(OBJ)/salloc.o $(OBJ)/compare.o $(OBJ)/myrepo.o
	$(CC) -o $@ $^ $(CFLAGS)
