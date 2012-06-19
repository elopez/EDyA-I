MAKE=make
.PHONY: build test clean

build:
	@$(MAKE) -C mydiff/ build

clean:
	@$(MAKE) -C mydiff/ clean

test:
	@./tests/runtests.sh
