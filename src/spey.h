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

#include <stdlib.h>
#include <syslog.h>
#include <string>
#include <sstream>

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
#include "MessageProcessor.h"

extern SQL Sql;
extern SocketAddress FromAddress;
extern SocketAddress ToAddress;

extern bool greylist(unsigned int sender,
		string fromaddress, string toaddress);

/* Revision history
 * $Log$
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 *
 */
