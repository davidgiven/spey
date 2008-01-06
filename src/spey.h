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

/* AUTH modes. */

enum AuthMode {
	NoAuth,
	ProxyAuth,
	InternalAuth
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

/* Globals */

extern char MajorVersion[];
extern char BuildCount[];

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

/* Base64 encode/decode. */

extern string base64_encode(const void *data, int size);
extern int base64_encode(const void *data, int size, char **str);
extern int base64_decode(const string& str, void *data);
extern int base64_decode(const char *str, void *data);

/* Authentication systems. */

typedef int Authenticator(Socket& stream, const string& initial);
extern Authenticator auth_plain;
extern Authenticator auth_login;
