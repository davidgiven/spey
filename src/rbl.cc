/* rbl.cc
 * RBL lookup engine.
 *
 * Copyright (C) 2007 David Given
 * You may distribute under the terms of the GNU General Public
 * License version 2 as specified in the file COPYING that comes with the
 * Spey distribution.
 *
 * $Source$
 * $State$
 */

#include "spey.h"

bool rblcheck(uint32_t sender, string rbldomain)
{
	stringstream s;
	s << ((sender >>  0) & 0xFF) << '.'
	  << ((sender >>  8) & 0xFF) << '.'
	  << ((sender >> 16) & 0xFF) << '.'
	  << ((sender >> 24) & 0xFF) << '.'
	  << rbldomain;
	MessageLog() << "checking " << s.str();

	bool found = false;
	
	Threadlet::releaseCPUlock();
	try
	{	
		SocketAddress sa;
		sa.setname(s.str());
		found = true;
	}
	catch (NetworkException e)
	{
		/* Do nothing --- we expected this if the host was not found. */
	}
	Threadlet::takeCPUlock();
	return found;
}

/* Revision history
 * $Log$
  */
