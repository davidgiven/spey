# Makefile
# Top-level makefile that builds Spey.
#
# Copyright (C) 2004 David Given
# You may distribute under the terms of the GNU General Public
# License version 2 as specified in the file COPYING that comes with the
# Spey distribution.
#
# $Source$
# $State$

# Compilation options.

PREFIX = /usr
EXE = bin/spey
OPTIMISATION = -g 
CC = g++ -Wall

# If you want GNUTLS support, set this to 'yes'.

GNUTLS = yes

# If you want to debug Spey, you can enable Electric Fence support here.

ELECTRICFENCE = no

# If you're on a platform that needs extra library and include paths, put
# them here.

EXTRAINCLUDES = -I/usr/local/include
EXTRALIBS = -L/usr/local/lib

# You shouldn't need to touch anything below here.

MAJORVERSION := 0.4.3
BUILDCOUNT := 1
include version

CFLAGS = $(OPTIMISATION) -I. $(EXTRAINCLUDES) -DMAJORVERSION=\"$(MAJORVERSION)\" -DBUILDCOUNT=\"$(BUILDCOUNT)\"
LIBS = \
	$(EXTRALIBS) \
	-lpthread \
	-lsqlite

ifeq ($(ELECTRICFENCE),yes)
	CFLAGS += -DELECTRICFENCE
	LIBS += -lefence
endif

ifeq ($(GNUTLS),yes)
	CFLAGS += -DGNUTLS
	LIBS += -lgnutls
endif

OBJS = \
	src/Exception.o \
	src/Logger.o \
	src/SQL.o \
	src/SocketAddress.o \
	src/SocketServer.o \
	src/Socket.o \
	src/Statistics.o \
	src/Settings.o \
	src/CLI.o \
	src/Parser.o \
	src/SMTPResponse.o \
	src/SMTPCommand.o \
	src/Threadlet.o \
	src/MessageProcessor.o \
	src/ServerProcessor.o \
	src/greylist.o \
	src/rbl.o \
	src/base64.o \
	src/auth.o \
	src/main.o \
	src/version.o

all: $(EXE)

install:
	install -D $(EXE) $(PREFIX)/sbin/spey
	strip $(PREFIX)/sbin/spey
	install -D scripts/speyctl $(PREFIX)/sbin/speyctl
	install -D scripts/init.d.script /etc/init.d/spey
	install -D doc/speyctl.8 $(PREFIX)/man/man8/speyctl.8
	install -D doc/spey.8 $(PREFIX)/man/man8/spey.8

$(EXE): $(OBJS)
	mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)
	echo "MAJORVERSION := $(MAJORVERSION)" > version
	echo "BUILDCOUNT :=" `expr $(BUILDCOUNT) + 1` >> version

clean:
	$(RM) -f $(OBJS) $(EXE) 

%.o: %.cc src/spey.h
	$(CC) $(CFLAGS) -c -o $@ $<

src/CLI.cc src/CLI.h: src/CLI.gp
	genparse -l c++ -o src/CLI -p CLI $<

src/version.o: version

src/spey.h: \
	src/Exception.h \
	src/Logger.h \
	src/SQL.h \
	src/CLI.h \
	src/SocketAddress.h \
	src/SocketServer.h \
	src/Statistics.h \
	src/Settings.h \
	src/MessageProcessor.h \
	src/Parser.h \
	src/SMTPResponse.h \
	src/SMTPCommand.h

version:
	echo "MAJORVERSION := $(MAJORVERSION)" > version
	echo "BUILDCOUNT := $(BUILDCOUNT)" >> version
