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

struct SQL
{
	SQL();
	~SQL();

	void open(string filename);
	void close();
	bool checktable(string name);

	operator sqlite* () { return this->handle; }

private:
	sqlite* handle;
};

struct SQLQuery
{
	SQLQuery(SQL& sql, string statement);
	~SQLQuery();

	bool step();
	string getstring(int i);
	int getint(int i);

private:
	sqlite_vm* handle;
	int columns;
	const char** values;
	const char** types;
};

#endif

/* Revision history
 * $Log$
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 *
 */
