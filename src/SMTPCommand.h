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

inline Logger& operator << (Logger& s, SMTPCommand& sa)
{
	s << (string) sa;
	return s;
}

#endif

/* Revision history
 * $Log$
 * Revision 1.2  2004/06/22 21:01:02  dtrg
 * Made a lot of minor tweaks so that spey now builds under gcc 3.3. (3.3 is a lot
 * closer to the C++ standard than 2.95 is; plus, the standard library is now
 * rather different, which means that I'm not allowed to do things like have local
 * variables called errno.)
 *
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 */

