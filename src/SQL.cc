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

#include "common.h"
#include <unistd.h>

SQL Sql;

static char* error;
static void sqlerror(string msg)
{
	if (error)
	{
		msg += ": ";
		msg += error;
		free(error);
		error = NULL;
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

void SQL::open(const string& filename)
{
	if (this->handle)
		sqlite_close(handle);
	this->handle = sqlite_open(filename.c_str(), 0, &error);
	if (!this->handle)
		sqlerror("Database open failure");
	sqlite_busy_timeout(this->handle, 1000);

	exec("PRAGMA synchronous=OFF;");
}

void SQL::close()
{
	if (this->handle)
		sqlite_close(handle);
}

bool SQL::checktable(const string& name)
{
	SQLQuery q(*this, "SELECT COUNT(*) FROM sqlite_master "
		                "WHERE type='table' AND name=%Q;",
		                name.c_str());
	q.step();
	return q.getint(0);
}

void SQL::exec(const string& sql, ...)
{
	va_list ap;
	va_start(ap, sql);
	char* s = sqlite_vmprintf(sql.c_str(), ap);
	if (!s)
		throw std::bad_alloc();
	SQLLog() << s;

	try
	{
		if (sqlite_exec(this->handle, s, NULL, NULL, &error))
			sqlerror("SQL query execution failure");
		sqlite_freemem(s);
	}
	catch (...)
	{
		/* Ensure that s is freed, even if an exception is thrown. */
		sqlite_freemem(s);
		throw;
	}
}

/* --- SQL commit lock --------------------------------------------------- */

SQLCommitLock::SQLCommitLock(SQL& sql):
		_sql(sql),
		_committed(false)
{
	_sql.exec("BEGIN;");
}

SQLCommitLock::~SQLCommitLock()
{
	if (_committed)
		_sql.exec("COMMIT;");
	else
		_sql.exec("ROLLBACK;");
}

void SQLCommitLock::commit()
{
	_committed = true;
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
	const char* p = this->values[i];
	if (!p)
		return "";
	string s = p;
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

unsigned SQLQuery::getunsigned(int i)
{
	if ((i < 0) || (i >= this->columns) || !this->values)
		return 0;
	SQLLog() << "-> (unsigned) " << this->values[i];
	return strtoul(this->values[i], NULL, 10);
}
