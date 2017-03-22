GNUNETIRCD_VERSION = 0.1.1

all:
	$(MAKE) -C src

clean:
	$(MAKE) -C src clean

test: clean
	CFLAGS="-g -Wall" LDFLAGS=-g $(MAKE)
	prove -f

.PHONY: all clean test
