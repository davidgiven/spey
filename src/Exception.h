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

struct Exception
{
	Exception();
	Exception(string reason);
	Exception(string reason, int errnum);

	operator string () const { return this->reason; }

protected:
	string reason;
};

inline Logger& operator << (Logger& stream, const Exception& e)
{
	stream << (string) e;
	return stream;
}

struct InvocationException: Exception
{
	InvocationException(string reason): Exception(reason)
	{
	}
};

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

struct AuthenticationCancelledException: Exception
{
	AuthenticationCancelledException(): Exception("Authentication cancelled")
	{}
};

#endif
