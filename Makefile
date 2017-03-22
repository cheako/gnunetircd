GNUNETIRCD_VERSION = 0.1.1

all:
	$(MAKE) -C src

clean:
	$(MAKE) -C src clean

test: clean
	CFLAGS="-g -Wall" LDFLAGS=-g $(MAKE)
	CFLAGS="-g -Wall" LDFLAGS=-g $(MAKE) -C src test_container_cdll
	prove -f

.PHONY: all clean test
