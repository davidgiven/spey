/* Exception.cc
 * Exception handler base class
 * $Source$
 * $State$
 */

#include "spey.h"

Exception::Exception()
{
	this->reason = "(no error message)";
}

Exception::Exception(string reason)
{
	this->reason = reason;
}

Exception::Exception(string reason, int errno)
{
	stringstream s;
	s << reason
	  << ": "
	  << strerror(errno)
	  << " ("
	  << errno
	  << ")";
	this->reason = s.str();
}

/* Revision history
 * $Log$
 */