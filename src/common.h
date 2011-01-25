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

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <syslog.h>
#include <string>
#include <sstream>
#include <iostream>

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

#include "Threadlet.h"
#include "Logger.h"
#include "Exception.h"
#include "SQL.h"
#include "Statistics.h"
#include "SocketAddress.h"
#include "Socket.h"
#include "SocketServer.h"
#include "Settings.h"

/* Globals */

extern const char MajorVersion[];
extern const char BuildCount[];

extern SQL Sql;

/* Base64 encode/decode. */

extern string base64_encode(const void *data, int size);
extern int base64_encode(const void *data, int size, char **str);
extern int base64_decode(const string& str, void *data);
extern int base64_decode(const char *str, void *data);

#endif
