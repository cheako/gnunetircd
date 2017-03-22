FROM debian:stretch
LABEL Description="This image is used to test gnunet applications" Version="0.1"
RUN apt-get update && apt-get install -y build-essential gnunet-dev libipc-run-perl valgrind
RUN mkdir -p /usr/src/github
COPY . /usr/src/github/gnunetircd
CMD make -C /usr/src/github/gnunetircd test
