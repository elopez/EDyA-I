CC=gcc
IDIR=../include
SDIR=../shared
OBJ=../obj
BIN=../bin
CFLAGS=-I$(IDIR) -DNDEBUG -D_POSIX_C_SOURCE=200112L \
       -Wall -Wextra -pedantic --std=c99 \
       -Ofast

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

$(BIN)/myrepo: $(OBJ)/usage.o $(OBJ)/init.o $(OBJ)/addrm.o $(OBJ)/catalog.o $(OBJ)/myrepo.o
	$(CC) -o $@ $^ $(CFLAGS)
