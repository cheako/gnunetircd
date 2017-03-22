PKGCONFIGLIBS += gnunetutil
CFLAGS += $(shell pkg-config --cflags $(PKGCONFIGLIBS))
LDLIBS += $(shell pkg-config --libs $(PKGCONFIGLIBS))

all: gnunetircd

command_relay.o: command_relay.h
OBJS += command_relay.o

misc_commands.o: misc_commands.h
OBJS += misc_commands.o

inetd.o: inetd.h
OBJS += inetd.o

gnunetircd: $(OBJS)

clean:
	rm -f *.o gnunetircd

.PHONY: clean
