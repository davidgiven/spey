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

Logger::Logger(int level)
{
	this->level = level;
}

Logger::~Logger()
{
	this->flush();
}

void Logger::setlevel(int desired)
{
	Logger::desired = desired;
}

void Logger::flush()
{
	stringstream::flush();
	if (this->level < Logger::desired)
	{
		struct timeval tv;
		gettimeofday(&tv, NULL);

		struct tm* tm = localtime(&tv.tv_sec);

		char buffer[32];
		strftime(buffer, sizeof(buffer), "%T", tm);
		cout << "spey: "
		     << this->level
		     << ": "
		     << buffer
		     << "+"
		     << (tv.tv_usec / 1000)
		     << ": "
		     << this->str()
		     << endl;
	}
	this->str("");
}

/* Revision history
 * $Log$
 */