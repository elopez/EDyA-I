MAKE=make
.PHONY: build test clean

build:
	@$(MAKE) -C mydiff/ build
	@$(MAKE) -C mypatch/ build
	@$(MAKE) -C myrepo/ build

clean:
	@$(MAKE) -C mydiff/ clean
	@$(MAKE) -C mypatch/ clean
	@$(MAKE) -C myrepo/ clean

test:
	@./tests/runtests.sh
