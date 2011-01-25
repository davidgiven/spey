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

#ifndef SPEY_H
#define SPEY_H

#include "common.h"

#include "SpeyCLI.h"
#include "Statistics.h"
#include "Parser.h"
#include "SMTPResponse.h"
#include "SMTPCommand.h"
#include "MessageProcessor.h"
#include "ServerProcessor.h"

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

/* Authentication systems. */

typedef int Authenticator(Socket& stream, const string& initial);
extern Authenticator auth_plain;
extern Authenticator auth_login;

#endif
