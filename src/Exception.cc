/* Exception.cc
 * Exception handler base class
 *
 * Copyright (C) 2004 David Given
 * You may distribute under the terms of the GNU General Public
 * License version 2 as specified in the file COPYING that comes with the
 * Spey distribution.
 *
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
 * Revision 1.1  2004/05/01 11:52:00  dtrg
 * Initial version.
 */