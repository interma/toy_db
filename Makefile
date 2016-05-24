
default: all

.DEFAULT:
	cd src && $(MAKE) $@

.PHONY: test
test:
