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

PREFIX = /usr

EXE = bin/spey

CFLAGS = -Wall -g -I. -O3
CC = g++

LIBS = \
	-lsqlite

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
	src/main.o

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

clean:
	$(RM) -f $(OBJS) $(EXE) 

%.o: %.cc src/spey.h
	$(CC) $(CFLAGS) -c -o $@ $<

src/CLI.cc src/CLI.h: src/CLI.gp
	genparse -l c++ -o src/CLI -p CLI $<

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

# Revision history
# $Log$
# Revision 1.4  2004/06/22 21:00:59  dtrg
# Made a lot of minor tweaks so that spey now builds under gcc 3.3. (3.3 is a lot
# closer to the C++ standard than 2.95 is; plus, the standard library is now
# rather different, which means that I'm not allowed to do things like have local
# variables called errno.)
#
# Revision 1.3  2004/05/30 01:55:13  dtrg
# Numerous and major alterations to implement a system for processing more than
# one message at a time, based around coroutines. Fairly hefty rearrangement of
# constructors and object ownership semantics. Assorted other structural
# modifications.
#
# Revision 1.2  2004/05/01 15:44:52  dtrg
# Now strips binary before installing; creates missing bin directory when
# linking.
#
# Revision 1.1  2004/05/01 12:20:20  dtrg
# Initial version.
#
