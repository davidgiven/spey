/* MessageProcessor.cc
 * Class that handles processing an individual message.
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

MessageProcessor::MessageProcessor(int fd, SocketAddress& address)
{
	_outside.init(fd, address);
	_outside.timeout(Settings::sockettimeout());

	Threadlet::addthreadlet(this);
}

MessageProcessor::~MessageProcessor()
{
}

void MessageProcessor::readinside()
{
	_response.set(_inside);
	SMTPLog() << "s<i "
		  << _response
		  << flush;
}

void MessageProcessor::writeinside()
{
	_inside.writeline(_command);
	SMTPLog() << "s>i "
		  << _command
		  << flush;
}

void MessageProcessor::readoutside()
{
	_command.set(_outside);
	SMTPLog() << "o>s "
		  << _command
		  << flush;
}

void MessageProcessor::writeoutside()
{
	_outside.writeline(_response);
	SMTPLog() << "o<s "
		  << _response
		  << flush;
}

void MessageProcessor::verifydomain(string domain)
{
	string::size_type i;

	i = domain.find('.');
	if (i == string::npos)
		throw MalformedDomainException();
}

void MessageProcessor::verifyaddress(string address)
{
	string::size_type i;

	i = address.find('@');
	if (i == string::npos)
		throw MalformedAddressException();
}

void MessageProcessor::verifyrelay(string address)
{
	DetailLog() << "checking "
		    << address
		    << " from "
		    << _outside.getaddress().getname()
		    << " for relaying"
		    << flush;

	if (!Settings::testrelay(_outside.getaddress(), address))
		throw IllegalRelayingException();
}

void MessageProcessor::process()
{
	SMTPResponse deferrederror;
	bool errorstate = 0;
	bool connected = 0;
	
	_response.set(220);
	_response.parmoverride(Settings::identity());
	writeoutside();

	for (;;)
	{
		try {
			readoutside();
		} catch (InvalidSMTPCommandException e)
		{
			_response.set(500);
			writeoutside();
			if (Settings::intolerant())
				goto abort;
			continue;
		}

		/* Make sure we're connected to the downstream SMTP server. */

		if (!connected)
		{
			try {
				_inside.init(ToAddress);
				_inside.timeout(Settings::sockettimeout());

				/* Check it's okay. */

				readinside();
				_response.check(220, "Invalid banner");
				connected = 1;
			} catch (NetworkException e) {
				/* Something didn't work while making the
				 * connection to the downstream SMTP server.
				 * Bail out cleanly. */
				_response.set(421);
				writeoutside();
				throw e;
			}
		}

		switch (_command.cmd())
		{
			case SMTPCommand::HELP:
				_response.set(214);
				writeoutside();
				continue;

			case SMTPCommand::HELO:
			case SMTPCommand::EHLO:
				try {
					verifydomain(_command.arg());
				} catch (MalformedDomainException e)
				{
					deferrederror.set(554);
					deferrederror.msgoverride("Illegal domain name");
					Statistics::malformedDomain();
					goto error;
				}
				break;

			case SMTPCommand::MAIL:
				try {
					_from = _command.arg();
					if (_from != "")
						verifyaddress(_from);
				} catch (MalformedAddressException e)
				{
					deferrederror.set(551);

					Statistics::malformedAddress();
					goto error;
				}

				break;

			case SMTPCommand::RCPT:
				try {
					string address = _command.arg();

					verifyaddress(address);
					verifyrelay(address);

					switch(greylist(_outside.getaddress() & 0xFFFFFF00,
							_from, address))
					{
						case Accepted:
							break;

						case GreyListed:
							goto greymessage;

						case BlackListed:
							goto blackmessage;
					}
				} catch (MalformedAddressException e)
				{
					deferrederror.set(551);
					Statistics::malformedAddress();
					goto error;
				}
				catch (IllegalRelayingException e)
				{
					deferrederror.set(550);
					deferrederror.msgoverride("Relaying not permitted");

					Statistics::illegalRelaying();
					goto error;
				}
				catch (GreylistedException e)
				{
					goto greymessage;
				}
					
				Statistics::accepted();
				break;

			case SMTPCommand::RSET:
				_from = "";
				break;

			case SMTPCommand::DATA:
				if (errorstate)
					goto error;
				break;

			blackmessage:
				{
					deferrederror.set(554);
					stringstream s;
					s << "You have been blacklisted. "
					  << "Your message will not be accepted.";
					deferrederror.msgoverride(s.str());
					errorstate = 1;

					Statistics::blacklisted();
					break;
				}

			greymessage:
				{
					deferrederror.set(451);
					stringstream s;
					s << "Greylisted - try again in "
					  << (Settings::quarantinetime()/60)
					  << " minutes and your message will be accepted";
					deferrederror.msgoverride(s.str());
					errorstate = 1;

					Statistics::greylisted();
					break;
				}
		}

		writeinside();

		readinside();
		writeoutside();

		if (_response.iserror() && Settings::intolerant())
			goto abort;
		
		switch (_command.cmd())
		{
			case SMTPCommand::DATA:
			{
				if (!_response.issuccess())
					break;

				SMTPLog() << "transferring message body"
					  << flush;

				/* Add our Received: header. */

				{
					stringstream s;
					s << "Received: from "
					  << _outside.getaddress().getname()
					  << " and verified by Spey"
					  << "\n\tconnected from "
					  << (string) _outside.getaddress()
					  << "\n\twith envelope "
					  << _from;
					_inside.writeline(s.str());
				}

				for (;;)
				{
					string s = _outside.readline();
					_inside.writeline(s);
					if (s == ".")
						break;
				}

				SMTPLog() << "body transferred"
					  << flush;

				readinside();
				writeoutside();
				break;
			}

			case SMTPCommand::QUIT:
				return;
		}

		continue;

	error:
		_response = deferrederror;
		writeoutside();
		if (Settings::intolerant())
			goto abort;
		errorstate = 0;
		continue;
	}

abort:
	throw NetworkException("Won't tolerate SMTP errors");
}

int MessageProcessor::debugid()
{
	return _outside.getfd();
}

void MessageProcessor::run()
{
	process();
}

/* Revision history
 * $Log$
 * Revision 1.6  2004/06/22 10:05:37  dtrg
 * Fixed some more logic flow bugs in the blacklist code. (Blacklisted messages
 * were being reported as greylisted.)
 *
 * Revision 1.5  2004/06/21 23:12:46  dtrg
 * Added blacklisting and whitelisting support.
 *
 * Revision 1.4  2004/06/08 19:58:04  dtrg
 * Fixed a bug where the address of incoming connections was thought to be the
 * address of *this* end of the connection, not the other end. In the process,
 * changed some this->blah instance variables to _blah.
 *
 * Revision 1.3  2004/05/30 01:55:13  dtrg
 * Numerous and major alterations to implement a system for processing more than
 * one message at a time, based around coroutines. Fairly hefty rearrangement of
 * constructors and object ownership semantics. Assorted other structural
 * modifications.
 *
 * Revision 1.2  2004/05/14 23:11:44  dtrg
 * Added decent relaying support. Also converted SocketAddress to use references a
 * lot rather than pass-by-value, out of general tidiness and the hope that it
 * will improve performance a bit.
 *
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 *
 */
