/* SMTPCommand.h
 * Represents an SMTP command from the remote server.
 *
 * Copyright (C) 2004 David Given
 * You may distribute under the terms of the GNU General Public
 * License version 2 as specified in the file COPYING that comes with the
 * Spey distribution.
 *
 * $Source$
 * $State$
 */

#ifndef SMTCOMMAND_H
#define SMTCOMMAND_H

struct SMTPCommand {
	enum {
		HELO,
		EHLO,
		MAIL,
		RCPT,
		DATA,
		VRFY,
		RSET,
		EXPN,
		HELP,
		NOOP,
		QUIT
	};

	SMTPCommand();
	SMTPCommand(int command, string parameter="");
	SMTPCommand(Socket& in);

	void set();
	void set(int command, string parameter="");
	void set(Socket& in);

	operator string ();

	int cmd() { return command; }
	string arg() { return parameter; }

protected:
	int command;
	string parameter;
};

inline ostream& operator << (ostream& s, SMTPCommand& sa)
{
	s << (string) sa;
	return s;
}

#endif

/* Revision history
 * $Log$
 */