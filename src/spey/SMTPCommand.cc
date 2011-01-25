/* SMTPCommand.cc
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

#include "spey.h"
#include <ctype.h>

SMTPCommand::SMTPCommand()
{
	set();
}

SMTPCommand::SMTPCommand(int command, string parameter)
{
	set(command, parameter);
}

SMTPCommand::SMTPCommand(Socket& in)
{
	set(in);
}

void SMTPCommand::set()
{
	this->command = 0;
	this->parameter = "";
}

void SMTPCommand::set(int command, string parameter)
{
	this->command = command;
	this->parameter = parameter;
}

void SMTPCommand::set(Socket& in)
{
	string l = in.readline();
	SMTPLog() << "cmd "
		  << l;

	try {
		Parser p(l);
		string cmd = p.getword(tolower);

		if (cmd == "helo")
		{
			this->command = HELO;
			goto ehlo;
		}
		else if (cmd == "ehlo")
		{
			this->command = EHLO;
		ehlo:
			p.whitespace();
			this->parameter = p.getword(tolower);
			p.eol();
		}
		else if (cmd == "mail")
		{
			this->command = MAIL;

			p.whitespace();
			p.expect("from:");
			p.whitespace();

			if (p.peek() == '<')
			{
				p.expect("<");
				this->parameter = p.getword(tolower, '>');
				p.expect(">");
			}
			else
				this->parameter = p.getword(tolower);
				
			/* Ignore the rest of the line, to cope with broken mailers who
			 * insist on sending the RFC1870 SIZE=1234 extension despite the
			 * fact we haven't said we support it. */
		}
		else if (cmd == "rcpt")
		{
			this->command = RCPT;

			p.whitespace();
			p.expect("to:");
			p.whitespace();

			if (p.peek() == '<')
			{
				p.expect("<");
				this->parameter = p.getword(tolower, '>');
				p.expect(">");
			}
			else
				this->parameter = p.getword(tolower);
			p.eol();
		}
		else if (cmd == "data")
		{
			this->command = DATA;
			p.eol();
		}
		else if (cmd == "quit")
		{
			this->command = QUIT;
			p.eol();
		}
		else if (cmd == "rset")
		{
			this->command = RSET;
			p.eol();
		}
		else if (cmd == "help")
		{
			this->command = HELP;
			p.eol();
		}
		else if (cmd == "noop")
		{
			this->command = NOOP;
			p.eol();
		}
		else if (cmd == "auth")
		{
			this->command = AUTH;

			p.whitespace();
			this->parameter = p.getword(tolower);

			if (p.peek() == ' ')
			{
				p.whitespace();
				this->parameter += " " + p.getword();
			}

			p.eol();
		}
		else if (cmd == "starttls")
		{
			this->command = STARTTLS;
			p.eol();
		}
		else
			throw ParseErrorException();
	} catch (ParseErrorException e)
	{
		throw InvalidSMTPCommandException();
	}
}

SMTPCommand::operator string ()
{
	stringstream s;

	switch (this->command)
	{
		case HELO:
			s << "HELO "
			  << this->parameter;
			break;

		case EHLO:
			s << "EHLO "
			  << this->parameter;
			break;

		case MAIL:
			s << "MAIL FROM: <"
			  << this->parameter
			  << ">";
			break;

		case RCPT:
			s << "RCPT TO: <"
			  << this->parameter
			  << ">";
			break;

		case DATA:
			s << "DATA";
			break;

		case HELP:
			s << "HELP";
			break;

		case NOOP:
			s << "NOOP";
			break;

		case RSET:
			s << "RSET";
			break;

		case QUIT:
			s << "QUIT";
			break;
			
		case AUTH:
			s << "AUTH "
			  << this->parameter;
			break;
			
		case STARTTLS:
			s << "STARTTLS";
			break;
	}

	return s.str();
}
