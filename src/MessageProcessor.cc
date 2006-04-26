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
		  << _response;
}

void MessageProcessor::writeinside()
{
	_inside.writeline(_command);
	SMTPLog() << "s>i "
		  << _command;
}

void MessageProcessor::readoutside()
{
	_command.set(_outside);
	SMTPLog() << "o>s "
		  << _command;
}

void MessageProcessor::writeoutside()
{
	_outside.writeline(_response);
	SMTPLog() << "o<s "
		  << _response;
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
	/* All valid email addresses must contain a @ character. */
	
	if (address.find('@') == string::npos)
		throw MalformedAddressException();
	
	/* No valid email addresses contain any of these characters, which can be
	 * used for SQL injection. */
	 
	if (address.find_first_of("'\\") != string::npos)
		throw MalformedAddressException();
}

void MessageProcessor::verifyrelay(string address)
{
	DetailLog() << "checking "
		    << address
		    << " from "
		    << _outside.getaddress().getname()
		    << " for relaying";

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
					deferrederror.set(501);

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

		/* Pass on the command, and then fetch the result. */
		
		writeinside();
		readinside();
		
		/* If the command is a EHLO or HELO, then the RFC is a bit vague about
		 * what the response should be. In one part it says that only the
		 * response code should be valid, but in another it explicitly states
		 * the format that the EHLO response should have... and yes, there are
		 * broken mailers that require that format. So we need to override the
		 * default minimalist response if the command was EHLO or HELO here.
		 */
		
		if ((_command.cmd() == SMTPCommand::EHLO) ||
		    (_command.cmd() == SMTPCommand::HELO))
		{
			if (_response.issuccess())
				_response.parmoverride(Settings::identity());
		}
		writeoutside();

		/* If we don't tolerate errors, give up. */
		
		if (_response.iserror() && Settings::intolerant())
			goto abort;
		
		switch (_command.cmd())
		{
			case SMTPCommand::DATA:
			{
				if (!_response.issuccess())
					break;

				SMTPLog() << "transferring message body";

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

				SMTPLog() << "body transferred";

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
 * Revision 1.11  2006/04/25 21:25:59  dtrg
 * Changed the response to EHLO and HELO to more closely comply with an ambiguous bit in RFC2821; despite the fact that the RFC states that only the numeric code should be used, it also describes the response to EHLO and HELO as including the domain that the user gave. Guess what, some MTAs rely on this.
 *
 * Revision 1.10  2005/10/08 21:19:07  dtrg
 * Fixed a SQL injection security flaw. Email address received remotely were being pasted into SQL strings without first checking for ' and \ characters, which could result in a remote attacked executing arbitrary SQL strings on the server... nasty. MessageProcessor::verifyaddress() has now been extended to reject email addresses containing these characters, which are invalid in email addresses anyway. Thanks to Joshua Drake for spotting this one.
 *
 * Revision 1.9  2005/03/04 22:24:48  dtrg
 * The main message processing loop now ensures that it's connected to the local
 * server *after* it's verified the first command, not before. This means that if
 * the first command is bogus --- such as EHLO invaliddomainname --- we won't
 * waste resources connecting. (This was causing a problem on my system due to a
 * misconfigured machine making lots of connections to spey with invalid EHLO
 * statements; lots of copies of exim were being spawned, which was causing
 * inetd's loop-detection code to go into action.)
 *
 * Revision 1.8  2004/11/18 17:57:20  dtrg
 * Rewrote logging system so that it no longer tries to subclass stringstream,
 * that was producing bizarre results on gcc 3.3. Added version tracking to the
 * makefile; spey now knows what version and build number it is, and displays the
 * information in the startup banner. Now properly ignores SIGPIPE, which was
 * causing intermittent silent aborts.
 *
 * Revision 1.7  2004/06/30 20:18:49  dtrg
 * Changed the way sockets are initialised; instead of doing it from the Socket
 * and SocketServer constructors, they're set up as zombies and initialised later
 * with an init() method. This is cleaner, and also allows a cunning new feature:
 * the connection to the downstream SMTP server is now only made once the first
 * valid SMTP command is received from the upstream SMTP server. This means that
 * connections are only made once we're reasonably sure that there's going to be a
 * valid SMTP conversation, which should harden spey against DoS attacks like the
 * ones I get every so often. Also took the opportunity to convert more this->blah
 * instance variables into _blah.
 *
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
