/* Exception.h
 * Exception handler classes.
 *
 * Copyright (C) 2004 David Given
 * You may distribute under the terms of the GNU General Public
 * License version 2 as specified in the file COPYING that comes with the
 * Spey distribution.
 *
 * $Source$
 * $State$
 */

#ifndef EXCEPTION_H
#define EXCEPTION_H

struct Exception {
	Exception();
	Exception(string reason);
	Exception(string reason, int errno);

	operator string () { return this->reason; }

protected:
	string reason;
};

inline ostream& operator << (ostream& stream, Exception& e)
{
	stream << (string) e;
	return stream;
}

struct SQLException: Exception
{
	SQLException(string reason): Exception(reason)
	{}
};

struct NetworkException: Exception
{
	NetworkException(string reason): Exception(reason)
	{}

	NetworkException(string reason, int errno): Exception(reason, errno)
	{}
};

struct EOFException: NetworkException
{
	EOFException(): NetworkException("EOF")
	{}
};

struct InvalidSMTPCommandException: NetworkException
{
	InvalidSMTPCommandException(): NetworkException("Invalid SMTP command")
	{}
};

struct MalformedDomainException: NetworkException
{
	MalformedDomainException(): NetworkException("Malformed domain")
	{}
};

struct MalformedAddressException: NetworkException
{
	MalformedAddressException(): NetworkException("Malformed address")
	{}
};

struct IllegalRelayingException: NetworkException
{
	IllegalRelayingException(): NetworkException("Relaying not allowed")
	{}
};

struct GreylistedException: NetworkException
{
	GreylistedException(): NetworkException("Message greylisted")
	{}
};

struct NetworkTimeoutException: NetworkException
{
	NetworkTimeoutException(): NetworkException("Socket timeout")
	{}
};

struct ParseErrorException: Exception
{
	ParseErrorException(): Exception("Parse error")
	{}
};

#endif

/* Revision history
 * $Log$
 */