PKGCONFIGLIBS += gnunetutil
CFLAGS += $(shell pkg-config --cflags $(PKGCONFIGLIBS))
LDLIBS += $(shell pkg-config --libs $(PKGCONFIGLIBS))

all: gnunetircd

hybrid-6/match.o: $(wildcard hybrid-6/*.h)
OBJS += hybrid-6/match.o

hybrid-6/s_user.o: $(wildcard hybrid-6/*.h)
OBJS += hybrid-6/s_user.o

command_relay.o: command_relay.h
OBJS += command_relay.o

misc_commands.o: misc_commands.h
OBJS += misc_commands.o

registration_commands.o: registration_commands.h
OBJS += registration_commands.o

inetd.o: inetd.h
OBJS += inetd.o

gnunetircd: $(OBJS)

clean:
	rm -f *.o hybrid-6/*.o gnunetircd test_container_cdll

.PHONY: clean
