/* SQL.h
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

#ifndef SQL_H
#define SQL_H

#include <sqlite.h>

struct SQL : uncopyable
{
	SQL();
	~SQL();

	void open(const string& filename);
	void close();
	bool checktable(const string& name);
	void exec(const string& sql, ...);

	operator sqlite* () { return this->handle; }

private:
	sqlite* handle;
};

struct SQLQuery
{
	SQLQuery(SQL& sql, const string& statement, ...);
	~SQLQuery();

	bool step();
	string getstring(int i);
	int getint(int i);
	unsigned getunsigned(int i);

private:
	sqlite_vm* handle;
	int columns;
	const char** values;
	const char** types;
};

struct SQLCommitLock
{
	SQLCommitLock(SQL& sql);
	~SQLCommitLock();

	void commit();

private:
	SQL& _sql;
	bool _committed : 1;
};

extern SQL Sql;

#endif
