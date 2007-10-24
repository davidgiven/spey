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
	MessageLog() << "doing RBL check for " << s.str();

	{
		Threadlet::Concurrent c;
		try
		{	
			SocketAddress sa;
			sa.setname(s.str());
			return true;
		}
		catch (NetworkException e)
		{
			/* Do nothing --- we expected this if the host was not found. */
		}
		return false;
	}
}

/* Revision history
 * $Log$
 * Revision 1.3  2007/10/24 20:44:15  dtrg
 * Did a lot of minor code cleanups and C++ style improvements: uncopyable C++
 * objects are now marked as such and do not have copy constructors, and RAI is
 * used for the threadlet mutex.
 *
 * Revision 1.2  2007/02/10 21:15:54  dtrg
 * Fixed a typo in some tracing.
 *
 * Revision 1.1  2007/02/10 20:59:16  dtrg
 * Added support for DNS-based RBLs.
 */
