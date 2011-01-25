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

#include "common.h"
#include <string.h>

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
