/* spey.h
 * Global declarations and top-level include file.
 *
 * Copyright (C) 2004 David Given
 * You may distribute under the terms of the GNU General Public
 * License version 2 as specified in the file COPYING that comes with the
 * Spey distribution.
 *
 * $Source$
 * $State$
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <syslog.h>
#include <string>
#include <sstream>

using namespace std;

/* Any class that wishes to be uncopyable (no const constructor or assignment
 * operator) should inherit from this. */
 
struct uncopyable
{
	uncopyable() {};
private:
	uncopyable(const uncopyable&);
	uncopyable& operator= (const uncopyable&);
};

#include "Logger.h"
#include "Exception.h"
#include "SQL.h"
#include "CLI.h"
#include "SocketAddress.h"
#include "Socket.h"
#include "SocketServer.h"
#include "Statistics.h"
#include "Settings.h"
#include "Parser.h"
#include "SMTPResponse.h"
#include "SMTPCommand.h"
#include "Threadlet.h"
#include "MessageProcessor.h"
#include "ServerProcessor.h"

extern SQL Sql;
extern SocketAddress FromAddress;
extern SocketAddress ToAddress;

/* Greylist validation. */

enum GreylistResponse {
	Accepted,
	BlackListed,
	GreyListed
};

extern GreylistResponse greylist(uint32_t sender,
		string fromaddress, string toaddress);

/* RBL validation. */

extern bool rblcheck(uint32_t sender, string rbldomainlist);

/* Revision history
 * $Log$
 * Revision 1.8  2007/10/24 20:44:15  dtrg
 * Did a lot of minor code cleanups and C++ style improvements: uncopyable C++
 * objects are now marked as such and do not have copy constructors, and RAI is
 * used for the threadlet mutex.
 *
 * Revision 1.7  2007/02/10 20:59:16  dtrg
 * Added support for DNS-based RBLs.
 *
 * Revision 1.6  2007/01/29 23:05:11  dtrg
 * Due to various unpleasant incompatibilities with ucontext, the
 * entire coroutine implementation has been rewritten to use
 * pthreads instead of user-level scheduling. This should make
 * things far more robust and portable, if a bit more heavyweight.
 * It also has the side effect of drastically simplified threadlet code.
 *
 * Revision 1.5  2004/11/18 17:57:20  dtrg
 * Rewrote logging system so that it no longer tries to subclass stringstream,
 * that was producing bizarre results on gcc 3.3. Added version tracking to the
 * makefile; spey now knows what version and build number it is, and displays the
 * information in the startup banner. Now properly ignores SIGPIPE, which was
 * causing intermittent silent aborts.
 *
 * Revision 1.4  2004/06/21 23:11:15  dtrg
 * Added a fix for gcc 3.0, hopefully. Untested *on* gcc 3.0, but it still builds
 * on 2.95.
 *
 * Revision 1.3  2004/05/30 01:55:13  dtrg
 * Numerous and major alterations to implement a system for processing more than
 * one message at a time, based around coroutines. Fairly hefty rearrangement of
 * constructors and object ownership semantics. Assorted other structural
 * modifications.
 *
 * Revision 1.2  2004/05/14 21:33:25  dtrg
 * Added the ability to log through syslog, rather than just to stderr.
 *
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 */

