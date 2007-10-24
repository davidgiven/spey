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

struct Logger : uncopyable
{
	Logger(int level, int type);
	~Logger();

	static void setlevel(int desired);
	static void detach();

	void flush();

private:
	static int desired;
	static bool syslogopened;
	static bool detached;
	
	stringstream _data;
	int _level;
	int _syslevel;

public:
	Logger& operator<< (const char* value)
	{
		_data << value;
		return *this;
	};

	Logger& operator<< (const string& value)
	{
		_data << value;
		return *this;
	};

	Logger& operator<< (unsigned long value)
	{
		_data << value;
		return *this;
	};
};

enum {
	LOGLEVEL_NONE = 0,
	LOGLEVEL_SYSTEM,
	LOGLEVEL_WARNING,
	LOGLEVEL_MESSAGE,
	LOGLEVEL_PARSING,
	LOGLEVEL_DETAIL,
	LOGLEVEL_SMTP,
	LOGLEVEL_THREADS,
	LOGLEVEL_SQL,
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
_LOG(SQLLog,        LOGLEVEL_SQL,       LOG_DEBUG);

#endif

/* Revision history
 * $Log$
 * Revision 1.6  2007/10/24 20:44:15  dtrg
 * Did a lot of minor code cleanups and C++ style improvements: uncopyable C++
 * objects are now marked as such and do not have copy constructors, and RAI is
 * used for the threadlet mutex.
 *
 * Revision 1.5  2007/04/19 14:13:09  dtrg
 * Added SQL tracing. Made the verbosity levels actually match the documentation,
 * and documented the trace format. Cleaned up the logger API a bit.
 *
 * Revision 1.4  2004/11/18 17:57:20  dtrg
 * Rewrote logging system so that it no longer tries to subclass stringstream,
 * that was producing bizarre results on gcc 3.3. Added version tracking to the
 * makefile; spey now knows what version and build number it is, and displays the
 * information in the startup banner. Now properly ignores SIGPIPE, which was
 * causing intermittent silent aborts.
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
