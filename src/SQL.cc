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
	if (this->handle)
		sqlite_close(handle);
}

void SQL::open(string filename)
{
	if (this->handle)
		sqlite_close(handle);
	this->handle = sqlite_open(filename.c_str(), 0, &error);
	if (!this->handle)
		sqlerror("Database open failure");
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
		sqlerror("SQL query finalization failure");
}

bool SQLQuery::step()
{
	int r;

	do {
		r = sqlite_step(this->handle,
				&this->columns, &this->values, &this->types);

		switch (r)
		{
			case SQLITE_BUSY:
				usleep(250000);
				break;

			case SQLITE_MISUSE:
			case SQLITE_ERROR:
			{
				string s = "SQL data access error";
				error = NULL;
				throw s;
			}

			case SQLITE_DONE:
				return 0;
		}
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
 */