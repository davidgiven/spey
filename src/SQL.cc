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
	string query =
		"SELECT COUNT(*) FROM sqlite_master "
		"WHERE type='table' AND name='";
	query += name;
	query += "';";

	SQLQuery q(*this, query);
	q.step();
	return q.getint(0);
}

/* --- SQL query class --------------------------------------------------- */

SQLQuery::SQLQuery(SQL& sql, string statement)
{
	if (sqlite_compile(sql, statement.c_str(),
			NULL, &this->handle, &error))
		sqlerror("SQL query compilation failure");
}

SQLQuery::~SQLQuery()
{
	if (sqlite_finalize(this->handle, &error))
	{
		DetailLog() << "SQL query finalization failure: "
		            << error
			    << flush;
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
	if ((i < 0) || (i >= this->columns))
		return "";
	string s = this->values[i];
	return s;
}

int SQLQuery::getint(int i)
{
	if ((i < 0) || (i >= this->columns))
		return 0;
	return atoi(this->values[i]);
}

/* Revision history
 * $Log$
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
