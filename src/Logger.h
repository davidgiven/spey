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
	Logger(int level);
	~Logger();

	static void setlevel(int desired);

	void flush();

private:
	static int desired;
	
	int level;
};

enum {
	LOGLEVEL_NONE = 0,
	LOGLEVEL_SYSTEM,
	LOGLEVEL_WARNING,
	LOGLEVEL_MESSAGE,
	LOGLEVEL_PARSING,
	LOGLEVEL_DETAIL,
	LOGLEVEL_SMTP
};

#define _LOG(_n, _l) \
	struct _n: public Logger { \
		_n(): Logger(_l) \
		{} \
	}

_LOG(SystemLog, LOGLEVEL_SYSTEM);
_LOG(WarningLog, LOGLEVEL_WARNING);
_LOG(MessageLog, LOGLEVEL_MESSAGE);
_LOG(ParseLog, LOGLEVEL_PARSING);
_LOG(DetailLog, LOGLEVEL_DETAIL);
_LOG(SMTPLog, LOGLEVEL_SMTP);

#undef _LOG

#endif

/* Revision history
 * $Log$
 */