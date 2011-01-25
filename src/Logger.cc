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

#include "common.h"
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
	if (_level <= Logger::desired)
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
