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
#include <syslog.h>
#include <ucontext.h>
#include <string>
#include <sstream>

using namespace std;

#include "Exception.h"
#include "Logger.h"
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

enum GreylistResponse {
	Accepted,
	BlackListed,
	GreyListed
};

extern GreylistResponse greylist(unsigned int sender,
		string fromaddress, string toaddress);

/* Revision history
 * $Log$
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

