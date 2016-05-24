
default: all

.DEFAULT:
	cd src && $(MAKE) $@

.PHONY: test
test:
	cd output && ./test_db
