/* version.cc
 * Contains the version number constants.
 *
 * Copyright (C) 2007 David Given
 * You may distribute under the terms of the GNU General Public
 * License version 2 as specified in the file COPYING that comes with the
 * Spey distribution.
 *
 * $Source$
 * $State$
 */

#include "common.h"
#include "SpeyCtlCLI.h"
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <signal.h>
#include <iostream>
#include <fstream>

/* If Electric Fence is turned on, enable its maximum paranoia settings. */

#ifdef ELECTRICFENCE
bool EF_FREE_WIPES = true;
#endif

/* SQL initialisation script. */

extern unsigned int init_sql_length;
extern const char init_sql_data[];

extern unsigned int purge_sql_length;
extern const char purge_sql_data[];

extern unsigned int statsreset_sql_length;
extern const char statsreset_sql_data[];

static void Xlistsyntaxerror(const string& cmd)
{
	throw InvocationException(
			string("syntax error: speyctl ") + cmd +
				" [add or sub or list] [<spec>]");
}

static void usersyntaxerror()
{
	throw InvocationException(
			"syntax error: speyctl user [add or sub or list] [<username>] [<password>]");
}

static void cmd_init()
{
	string initscript(init_sql_data, init_sql_length);
	Sql.exec(initscript);
}

static void cmd_set(const string& key, const string& value)
{
	SQLCommitLock commitlock(Sql);

	Sql.exec("DELETE FROM settings WHERE key=%Q;", key.c_str());
	if (value != "")
		Sql.exec("INSERT INTO settings VALUES (%Q, %Q);", key.c_str(),
				value.c_str());
	commitlock.commit();
}

static void cmd_list()
{
	SQLQuery query(Sql, "SELECT key, value FROM settings");

	printf("Variable                 Value\n");
	printf("----------------------------------------------------------\n");

	for (;;)
	{
		if (!query.step())
			break;

		printf("%-24s %s\n",
				query.getstring(0).c_str(),
				query.getstring(1).c_str());
	}
}

static void cmd_stats()
{
	SQLQuery query(Sql, "SELECT key, value FROM statistics");

	printf("Counter                  Value\n");
	printf("----------------------------------------------------------\n");

	for (;;)
	{
		if (!query.step())
			break;

		printf("%-24s %s\n",
				query.getstring(0).c_str(),
				query.getstring(1).c_str());
	}
}

static void cmd_statsreset()
{
	string script(statsreset_sql_data, statsreset_sql_length);
	Sql.exec(script);
}

static void cmd_showdb()
{
	SQLQuery query(Sql, "SELECT * FROM triples ORDER BY timesseen");

	for (;;)
	{
		if (!query.step())
			break;

		string s = query.getstring(2) + "->" + query.getstring(3);
		printf("%04X%04X %4d %65s\n",
				query.getunsigned(1) >> 16,
				query.getunsigned(1) & 0xffff,
				query.getunsigned(4),
				s.c_str());
	}
}

static void cmd_purge()
{
	string purgescript(purge_sql_data, purge_sql_length);
	Sql.exec(purgescript);
}

static void cmd_trust(const string& command, const string& value)
{
	if (command == "list")
	{
		SQLQuery query(Sql, "SELECT left, right FROM trustedhosts");

		for (;;)
		{
			if (!query.step())
				break;

			unsigned left = query.getunsigned(0);
			unsigned right = query.getunsigned(1);

			printf("%d.%d.%d.%d/%d\n",
					(left >> 24) & 0xff,
					(left >> 16) & 0xff,
					(left >> 8) & 0xff,
					(left) & 0xff,
					right);
		}
	}
	else
	{
		unsigned b1, b2, b3, b4, right, tail;
		int i = sscanf(value.c_str(), "%u.%u.%u.%u/%u%n",
				&b1, &b2, &b3, &b4, &right, &tail);
		if ((i != 5) || (value.size() != tail))
			Xlistsyntaxerror("trust");

		if (right > 31)
			throw InvocationException("IP address size is out of range (0-31)");
		if ((b1|b2|b3|b4) > 0xff)
			throw InvocationException("IP address element is out of range (0-255)");
		unsigned left = (b1 << 24) | (b2 << 16) | (b3 << 8) | b4;

		SQLCommitLock lock(Sql);

		Sql.exec("DELETE FROM trustedhosts WHERE "
				"left=%u AND right=%u;",
				left, right);
		if (command == "add")
			Sql.exec("INSERT INTO trustedhosts VALUES "
					"(%u, %u);",
					left, right);

		lock.commit();
	}
}

static void cmd_recipient(const string& command, const string& value)
{
	if (command == "list")
	{
		SQLQuery query(Sql, "SELECT left, right FROM validrecipients");

		for (;;)
		{
			if (!query.step())
				break;

			string left = query.getstring(0);
			string right = query.getstring(1);

			printf("%s@%s\n", left.c_str(), right.c_str());
		}
	}
	else
	{
		string::size_type atsign = value.find_first_of('@');
		if (atsign == string::npos)
			Xlistsyntaxerror("recipient");

		string left(value, 0, atsign);
		string right(value, atsign+1);
		if (right.find_first_of('@') != string::npos)
			Xlistsyntaxerror("recipient");

		SQLCommitLock lock(Sql);

		Sql.exec("DELETE FROM validrecipients "
				"WHERE left=%Q AND right=%Q;",
				left.c_str(), right.c_str());
		if (command == "add")
			Sql.exec("INSERT INTO validrecipients VALUES "
					"(%Q, %Q);",
					left.c_str(), right.c_str());

		lock.commit();
	}
}

static string strip(string s)
{
	string::size_type i = s.find_first_not_of(" \t");
	if (i != string::npos)
		s.erase(0, i);

	i = s.find_last_not_of(" \t");
	if (i != string::npos)
		s.erase(i+1);

	return s;
}

static void cmd_Xlist(const string& list, const string& command,
		const string& value)
{
	if (command == "list")
	{
		printf("                        Sender   Recipient\n"
			   "------------------------------   ------------------------------\n");

		SQLQuery query(Sql, "SELECT sender, recipient FROM %Q;", list.c_str());

		for (;;)
		{
			if (!query.step())
				break;

			string sender = query.getstring(0);
			string recipient = query.getstring(1);

			printf("%30s : %s\n",
					sender.c_str(), recipient.c_str());
		}
	}
	else
	{
		string::size_type colon = value.find_first_of(':');
		if (colon == string::npos)
			Xlistsyntaxerror(list);

		string sender(value, 0, colon);
		string recipient(value, colon+1);
		if (recipient.find_first_of(':') != string::npos)
			Xlistsyntaxerror(list);

		sender = strip(sender);
		recipient = strip(recipient);

		SQLCommitLock lock(Sql);

		Sql.exec("DELETE FROM %Q WHERE "
			"sender=%Q AND recipient=%Q;",
			list.c_str(), sender.c_str(), recipient.c_str());
		if (command == "add")
			Sql.exec("INSERT INTO %Q VALUES "
					"(%Q, %Q);",
					list.c_str(), sender.c_str(), recipient.c_str());

		lock.commit();
	}
}

static void cmd_user(const string& command,
		const string& username, const string& password)
{
	if (command == "list")
	{
		printf("       Username   Password\n"
			   "---------------   ---------------\n");

		SQLQuery query(Sql, "SELECT username, password FROM users;");

		for (;;)
		{
			if (!query.step())
				break;

			string username = query.getstring(0);
			string password = query.getstring(1);

			printf("%15s   %s\n",
					username.c_str(), password.c_str());
		}
	}
	else
	{
		bool exists;
		SQLCommitLock lock(Sql);

		{
			SQLQuery query(Sql, "SELECT COUNT(*) FROM users "
					"WHERE username=%Q;",
					username.c_str());

			query.step();
			exists = (query.getint(0) != 0);
		}

		if (command == "add")
		{
			if (password == "")
				usersyntaxerror();
			if (exists)
				throw InvocationException(
						string("the user '") + username + "' already exists");

			Sql.exec("INSERT INTO users VALUES "
					"(%Q, %Q);",
					username.c_str(), password.c_str());
			lock.commit();
		}
		else
		{
			if (password != "")
				usersyntaxerror();
			if (!exists)
				throw InvocationException(
						string("the user '") + username + "' does not exist");

			Sql.exec("DELETE FROM users WHERE "
					"username=%Q;",
					username.c_str());
			lock.commit();
		}
	}
}

int main_speyctl(int argc, char* argv[])
{
	SpeyCtlCLI cli(argc, argv);
	Logger::setlevel(cli.v());
	Sql.open(cli.d());

	int numparameters = cli.getparametercount();
	const string& command = cli.getparameter(0);
	if (command == "help")
		cli.usage();
	else if (command == "init")
	{
		if (numparameters > 1)
			goto toomanyparameters;

		cmd_init();
	}
	else if (command == "set")
	{
		const string& key = cli.getparameter(1);
		const string& value = cli.getparameter(2);
		if (cli.getparametercount() > 3)
			goto toomanyparameters;
		if (key == "")
			throw InvocationException("syntax: speyctl set <key> [<value>]");

		cmd_set(key, value);
	}
	else if (command == "list")
	{
		if (cli.getparametercount() != 1)
			goto toomanyparameters;

		cmd_list();
	}
	else if (command == "stats")
	{
		if (cli.getparametercount() != 1)
			goto toomanyparameters;

		cmd_stats();
	}
	else if (command == "statsreset")
	{
		if (cli.getparametercount() != 1)
			goto toomanyparameters;

		cmd_statsreset();
	}
	else if (command == "showdb")
	{
		if (cli.getparametercount() != 1)
			goto toomanyparameters;

		cmd_showdb();
	}
	else if (command == "purge")
	{
		if (cli.getparametercount() != 1)
			goto toomanyparameters;

		cmd_purge();
	}
	else if ((command == "trust") || (command == "recipient") ||
			 (command == "whitelist") || (command == "blacklist"))
	{
		if ((cli.getparametercount() < 2) || (cli.getparametercount() > 3))
			goto toomanyparameters;

		const string& subcommand = cli.getparameter(1);
		if ((subcommand != "list") && (subcommand != "add") &&
			(subcommand != "sub"))
			Xlistsyntaxerror(command);

		const string& value = cli.getparameter(2);
		if (command == "trust")
			cmd_trust(subcommand, value);
		else if (command == "recipient")
			cmd_recipient(subcommand, value);
		else
			cmd_Xlist(command, subcommand, value);
	}
	else if (command == "user")
	{
		if ((cli.getparametercount() < 2) || (cli.getparametercount() > 4))
			goto toomanyparameters;

		const string& subcommand = cli.getparameter(1);
		if ((subcommand != "list") && (subcommand != "add") &&
			(subcommand != "sub"))
		{
			usersyntaxerror();
		}

		const string& key = cli.getparameter(2);
		const string& value = cli.getparameter(3);

		cmd_user(subcommand, key, value);
	}
	else
		throw InvocationException("unknown command --- try --help for help");

	return 0;

toomanyparameters:
	throw InvocationException("too many parameters --- try --help for help");
}
