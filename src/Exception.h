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
	Exception(string reason, int errnum);

	operator string () { return this->reason; }

protected:
	string reason;
};

inline Logger& operator << (Logger& stream, Exception& e)
{
	stream << (string) e;
	return stream;
}

struct InternalException: Exception
{
	InternalException(string reason): Exception(reason)
	{
	}
};

struct IOException: Exception
{
	IOException(string reason, int errnum): Exception(reason, errnum)
	{}
};

struct SQLException: Exception
{
	SQLException(string reason): Exception(reason)
	{}
};

struct NetworkException: Exception
{
	NetworkException(string reason): Exception(reason)
	{}

	NetworkException(string reason, int errnum): Exception(reason, errnum)
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
 * Revision 1.3  2004/06/22 21:01:02  dtrg
 * Made a lot of minor tweaks so that spey now builds under gcc 3.3. (3.3 is a lot
 * closer to the C++ standard than 2.95 is; plus, the standard library is now
 * rather different, which means that I'm not allowed to do things like have local
 * variables called errno.)
 *
 * Revision 1.2  2004/05/30 01:55:13  dtrg
 * Numerous and major alterations to implement a system for processing more than
 * one message at a time, based around coroutines. Fairly hefty rearrangement of
 * constructors and object ownership semantics. Assorted other structural
 * modifications.
 *
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 */
