/* Logger.cc
 * Logging code.
 *
 * Copyright (C) 2004 David Given
 * You may distribute under the terms of the GNU General Public
 * License version 2 as specified in the file COPYING that comes with the
 * Spey distribution.
 *
 * $Source$
 * $State$
 */

#include "spey.h"
#include <time.h>
#include <sys/time.h>

int Logger::desired = 0;
bool Logger::syslogopened = 0;
bool Logger::detached = 0;

Logger::Logger(int level, int syslevel)
{
	if (!Logger::syslogopened)
	{
		Logger::syslogopened = true;
		openlog("spey", LOG_NDELAY|LOG_PID, LOG_MAIL);
	}

	_level = level;
	_syslevel = syslevel;
}

Logger::~Logger()
{
	this->flush();
}

void Logger::setlevel(int desired)
{
	Logger::desired = desired;
}

void Logger::detach()
{
	Logger::detached = true;
}

void Logger::flush()
{
	_data.flush();
	if (_level < Logger::desired)
	{
		stringstream s;
		Threadlet* current = Threadlet::current();

		s << _level
		  << '[';
		if (current)
			s << current->debugid();
		else
			s << 'M';
		s << "]: " << _data.str();

		if (Logger::detached)
			syslog(_syslevel, "%s", s.str().c_str());
		else
			cerr << s.str()
			     << endl;
	}
	_data.str("");
}

/* Revision history
 * $Log$
 * Revision 1.6  2005/10/08 21:05:26  dtrg
 * Fixed a security flaw in the call to syslog() that prevents the processing of bogus printf characters. I don't believe this could be used as a root exploit, but it could certainly crash Spey. Thanks to Joshua Drake for pointing this out.
 *
 * Revision 1.5  2005/09/25 23:09:44  dtrg
 * Changed some references to '0' and '1' to 'false' and 'true' for clarity.
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
