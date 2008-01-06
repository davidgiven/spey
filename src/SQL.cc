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
	SQLLog() << s;
	
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
	SQLLog() << "-> (string) " << s;
	return s;
}

int SQLQuery::getint(int i)
{
	if ((i < 0) || (i >= this->columns) || !this->values)
		return 0;
	SQLLog() << "-> (int) " << this->values[i];
	return atoi(this->values[i]);
}
