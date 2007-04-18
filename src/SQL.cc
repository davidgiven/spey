/* SQL.cc
 * Interface to the SQL database.
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
#include <unistd.h>

static char* error;
static void sqlerror(string msg)
{
	if (error)
	{
		msg += ": ";
		msg += error;
	}
	throw SQLException(msg);
}

SQL::SQL()
{
	this->handle = NULL;
}

SQL::~SQL()
{
	this->close();
}

void SQL::open(string filename)
{
	if (this->handle)
		sqlite_close(handle);
	this->handle = sqlite_open(filename.c_str(), 0, &error);
	if (!this->handle)
		sqlerror("Database open failure");
	sqlite_busy_timeout(this->handle, 1000);

	SQLQuery q(*this, "PRAGMA synchronous=OFF;");
	q.step();
}

void SQL::close()
{
	if (this->handle)
		sqlite_close(handle);
}

bool SQL::checktable(string name)
{
	SQLQuery q(*this, "SELECT COUNT(*) FROM sqlite_master "
		                "WHERE type='table' AND name=%Q;",
		                name.c_str());
	q.step();
	return q.getint(0);
}

/* --- SQL query class --------------------------------------------------- */

SQLQuery::SQLQuery(SQL& sql, const string& statement, ...)
{
	va_list ap;
	va_start(ap, statement);
	char* s = sqlite_vmprintf(statement.c_str(), ap);
	if (!s)
		throw std::bad_alloc();
	
	try
	{
		if (sqlite_compile(sql, s, NULL, &this->handle, &error))
			sqlerror("SQL query compilation failure");
		sqlite_freemem(s);
	}
	catch (...)
	{
		/* Ensure that s is freed, even if an exception is thrown. */
		sqlite_freemem(s);
		throw;
	}
}

SQLQuery::~SQLQuery()
{
	if (sqlite_finalize(this->handle, &error))
	{
		DetailLog() << "SQL query finalization failure: "
		            << error;
	}
}

bool SQLQuery::step()
{
	int r;

	do {
		r = sqlite_step(this->handle,
				&this->columns, &this->values, &this->types);

		switch (r)
		{
			case SQLITE_MISUSE:
			case SQLITE_ERROR:
				sqlerror("SQL data access error");

			case SQLITE_DONE:
				return 0;
		}
		/* SQLITE_BUSY falls through and repeats */
	} while (r != SQLITE_ROW);
	return 1;
}

string SQLQuery::getstring(int i)
{
	if ((i < 0) || (i >= this->columns) || !this->values)
		return "";
	string s = this->values[i];
	return s;
}

int SQLQuery::getint(int i)
{
	if ((i < 0) || (i >= this->columns) || !this->values)
		return 0;
	return atoi(this->values[i]);
}

/* Revision history
 * $Log$
 * Revision 1.6  2005/09/30 23:17:12  dtrg
 * Prevented a crash if one of the get...() methods was called on a Query if it was stepped off the end of the result; it now returns 0 or an empty string.
 *
 * Revision 1.5  2004/11/18 17:57:20  dtrg
 * Rewrote logging system so that it no longer tries to subclass stringstream,
 * that was producing bizarre results on gcc 3.3. Added version tracking to the
 * makefile; spey now knows what version and build number it is, and displays the
 * information in the startup banner. Now properly ignores SIGPIPE, which was
 * causing intermittent silent aborts.
 *
 * Revision 1.4  2004/05/30 01:55:13  dtrg
 * Numerous and major alterations to implement a system for processing more than
 * one message at a time, based around coroutines. Fairly hefty rearrangement of
 * constructors and object ownership semantics. Assorted other structural
 * modifications.
 *
 * Revision 1.3  2004/05/09 18:23:16  dtrg
 * SQL server now accessed asynchronously; backed out fix for mysterious SQL crash
 * and instead put in some code that should recover sanely from it. Don't know
 * what's going on here.
 *
 * Revision 1.2  2004/05/09 14:17:48  dtrg
 * No longer throws an exception when an SQLQuery is destructed (very evil!). Put
 * in failsafe code so that if the sql_step() returns an error code, keep retrying
 * for a short period; hopefully this will fix the strange crash that sometimes
 * occurs on my system when speyctl is used while spey is running.
 *
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 */
