/* Logger.h
 * Logging classes.
 *
 * Copyright (C) 2004 David Given
 * You may distribute under the terms of the GNU General Public
 * License version 2 as specified in the file COPYING that comes with the
 * Spey distribution.
 *
 * $Source$
 * $State$
 */

#ifndef LOGGER_H
#define LOGGER_H

struct Logger: public stringstream {
	Logger(int level, int type);
	~Logger();

	static void setlevel(int desired);
	static void detach();

	void flush();

private:
	static int desired;
	static bool syslogopened;
	static bool detached;
	
	int _level;
	int _syslevel;
};

enum {
	LOGLEVEL_NONE = 0,
	LOGLEVEL_SYSTEM,
	LOGLEVEL_WARNING,
	LOGLEVEL_MESSAGE,
	LOGLEVEL_PARSING,
	LOGLEVEL_DETAIL,
	LOGLEVEL_SMTP,
	LOGLEVEL_THREADS
};

#define _LOG(_n, _l, _t) \
	struct _n: public Logger { \
		_n(): Logger(_l, _t) \
		{} \
	}

_LOG(SystemLog, 	LOGLEVEL_SYSTEM,	LOG_NOTICE);
_LOG(WarningLog,	LOGLEVEL_WARNING,	LOG_WARNING);
_LOG(MessageLog,	LOGLEVEL_MESSAGE,	LOG_INFO);
_LOG(ParseLog,		LOGLEVEL_PARSING,	LOG_DEBUG);
_LOG(DetailLog,		LOGLEVEL_DETAIL,	LOG_DEBUG);
_LOG(SMTPLog,		LOGLEVEL_SMTP,		LOG_DEBUG);
_LOG(ThreadLog,		LOGLEVEL_THREADS,	LOG_DEBUG);

#endif

/* Revision history
 * $Log$
 * Revision 1.2  2004/05/14 21:33:25  dtrg
 * Added the ability to log through syslog, rather than just to stderr.
 *
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 */
