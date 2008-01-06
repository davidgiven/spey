/* MessageProcessor.h
 * Class that handles processing an individual message.
 *
 * Copyright (C) 2004 David Given
 * You may distribute under the terms of the GNU General Public
 * License version 2 as specified in the file COPYING that comes with the
 * Spey distribution.
 *
 * $Source$
 * $State$
 */

#ifndef MESSAGEPROCESSOR_H
#define MESSAGEPROCESSOR_H

struct MessageProcessor: Threadlet
{
	MessageProcessor(int fd, SocketAddress& address);
	virtual ~MessageProcessor();

	void verifydomain(string domain);
	void verifyaddress(string address);
	void process();

	virtual int debugid();
	virtual void run();

protected:
	Socket _inside;
	Socket _outside;

	string _from;
	SMTPCommand _command;
	SMTPResponse _response;
	void readinside();
	void writeinside();
	void readoutside();
	void writeoutside();
	
	void connect();
};

#endif
