/* SMTPResponse.cc
 * Represents a status response from the local SMTP server.
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

SMTPResponse::SMTPResponse(SMTPResponse& r)
{
	_code = r._code;
	_parameter = r._parameter;
	_msgoverride = r._msgoverride;
}

SMTPResponse::SMTPResponse()
{
	set();
}

SMTPResponse::SMTPResponse(int code, string parameter="")
{
	set(code, parameter);
}

SMTPResponse::SMTPResponse(Socket& in)
{
	set(in);
}

void SMTPResponse::set()
{
	_code = 0;
	_parameter = "";
}

void SMTPResponse::set(int code, string parameter="")
{
	_code = code;
	_parameter = parameter;
}

void SMTPResponse::set(Socket& in)
{
	string l = in.readline();
	SMTPLog() << "rsp "
		  << l
		  << flush;

	/* Validate the first four characters. */

	if (l.length() < 4)
	{
		MessageLog() << "SMTP response isn't long enough"
			     << flush;
		goto malformed;
	}

	if (!isdigit(l[0]) ||
	    !isdigit(l[1]) ||
	    !isdigit(l[2]))
	{
		MessageLog() << "SMTP response has no status code"
			     << flush;
		goto malformed;
	}

	if (l[3] == '-')
	{
		string s;

		do {
			s = in.readline();
			SMTPLog() << "rsp-"
				  << s
				  << flush;

			if (s.length() < 4)
			{
				MessageLog() << "SMTP continuation isn't long enough"
					     << flush;
				goto malformed;
			}

			if ((s[0] != l[0]) ||
			    (s[1] != l[1]) ||
			    (s[2] != l[2]))
			{
				MessageLog() << "SMTP continuation's "
						"status code is inconsistent"
					     << flush;
				goto malformed;
			}
		} while (s[3] == '-');
	}
	else if (l[3] != ' ')
	{
		MessageLog() << "SMTP response has invalid 4th char"
			     << flush;
		goto malformed;
	}

	_code = atoi(l.substr(0, 3).c_str());
	switch (_code)
	{
		case 220:
		case 221:
		case 421:
		{
			string::size_type i = l.find(' ', 4);
			if (i != string::npos)
			{
				_parameter = l.substr(4, i-4);
				break;
			}
			/* fall through */
		}

		default:
			_parameter = "";
	}

	return;

malformed:
	throw NetworkException("Malformed SMTP response");
}

void SMTPResponse::parmoverride(string parameter)
{
	_parameter = parameter;
}

void SMTPResponse::msgoverride(string message)
{
	_msgoverride = message;
}

static char* statusMessage(int code)
{
	switch (code)
	{
		case 211: return "System status";
		case 214: return "No help here";
		case 220: return "Service ready";
		case 221: return "Service closing transmission channel";
		case 250: return "Requested mail action okay, completed";
		case 251: return "User not local";
		case 252: return "Cannot VRFY user, but will accept message "
			  		"and attempt delivery";
		case 354: return "Start mail input; end with <CRLF>.<CRLF>";
		case 421: return "Service not available, closing transmission channel";
		case 450: return "Requested mail action not taken: mailbox unavailable";
		case 451: return "Requested action aborted: local error in processing";
		case 452: return "Requested action not taken: insufficient system storage";
		case 500: return "Syntax error, command unrecognized";
		case 501: return "Syntax error in parameters or arguments";
		case 502: return "Command not implemented";
		case 503: return "Bad sequence of commands";
		case 504: return "Command parameter not implemented";
		case 550: return "Requested action not taken: mailbox unavailable";
		case 551: return "User not local";
		case 552: return "Requested mail action aborted: exceeded storage allocation";
		case 553: return "Requested action not taken: mailbox name not allowed";
		case 554: return "Transaction failed";
	}
	return "No message available for this status code";
}

SMTPResponse::operator string ()
{
	stringstream s;

	s << _code
	  << ' ';
	if (_parameter != "")
		s << _parameter << ' ';
	if (_msgoverride == "")
		s << statusMessage(_code);
	else
		s << _msgoverride;

	return s.str();
}

void SMTPResponse::check(int code, string message)
{
	if (_code != code)
		throw NetworkException("Invalid SMTP request: "+message);
}

bool SMTPResponse::issuccess()
{
	int family = _code / 100;
	return (family == 2) || (family == 3);
}

bool SMTPResponse::isretry()
{
	return ((_code / 100) == 4);
}

bool SMTPResponse::iserror()
{
	return ((_code / 100) == 5);
}

/* Revision history
 * $Log$
 */