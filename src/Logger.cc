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
		Logger::syslogopened = 1;
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
	Logger::detached = 1;
}

void Logger::flush()
{
	stringstream::flush();
	if (_level < Logger::desired)
	{
		stringstream s;
		Threadlet* current = Threadlet::current();

		/* It would be much cleaner to use iostream for this, but I'm
		 * not sure that setw() and setbase() work on stringstreams.
		 * Besides, good old sprintf is easier... */

		char buf[16];
		if (current)
			sprintf(buf, "[%d]: ", current->debugid());
		else
			strcpy(buf, "[master]: ");

		s << buf
		  << str();

		if (Logger::detached)
			syslog(_syslevel, s.str().c_str());
		else
			cerr << s.str()
			     << endl;
	}
	this->str("");
}

/* Revision history
 * $Log$
 * Revision 1.2  2004/05/14 21:33:25  dtrg
 * Added the ability to log through syslog, rather than just to stderr.
 *
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 */
