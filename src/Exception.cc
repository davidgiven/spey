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

Exception::Exception(string reason, int errnum)
{
	stringstream s;
	s << reason
	  << ": "
	  << strerror(errnum)
	  << " ("
	  << errnum
	  << ")";
	this->reason = s.str();
}

/* Revision history
 * $Log$
 * Revision 1.2  2004/05/01 12:19:49  dtrg
 * Adjusted boilerplate.
 *
 * Revision 1.1  2004/05/01 11:52:00  dtrg
 * Initial version.
 */

