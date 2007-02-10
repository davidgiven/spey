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

void MessageProcessor::process()
{
	SMTPResponse deferrederror;
	bool errorstate = false;
	bool connected = false;
	bool authenticated = false;
	
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

		/* These commands do not require connection to the inside
		 * server. */

		switch (_command.cmd())
		{
			case SMTPCommand::HELP:
				_response.set(214);
				writeoutside();
				continue;

			case SMTPCommand::STARTTLS:
#ifdef GNUTLS
				if (Settings::externaltls())
				{
					if (_outside.issecure())
					{
						/* We're already using TLS --- fail. */
						deferrederror.set(503);
						goto error;
					}
					
					/* Perform the handshake. */
					 
					_response.set(220);
					writeoutside();
					
					_outside.makesecure();
					
					/* RFC2487 decrees that the SMTP session must be reset at this
					 * point; the way we do it is to ensure that we disconnect
					 * from the downstream server. */
					
					if (connected)
						_inside.deinit();
					connected = false;
					errorstate = false;
					authenticated = false;
					continue;
				}
#else
				deferrederror.set(500);
				goto error;
#endif
					
			case SMTPCommand::HELO:
			case SMTPCommand::EHLO:
				try
				{
					verifydomain(_command.arg());
				}
				catch (MalformedDomainException e)
				{
					deferrederror.set(554);
					deferrederror.msgoverride("Illegal domain name");
					Statistics::malformedDomain();
					goto error;
				}
				break;

			case SMTPCommand::MAIL:
				if (!authenticated)
				{
					try
					{
						_from = _command.arg();
						if (_from != "")
							verifyaddress(_from);
					}
					catch (MalformedAddressException e)
					{
						deferrederror.set(501);
	
						Statistics::malformedAddress();
						goto error;
					}
				}
				break;

			case SMTPCommand::RCPT:
				if (!authenticated)
				{
					try {
						string address = _command.arg();
	
						verifyaddress(address);
						
						/* If this is not a trusted machine, do further checks. */
						
						if (!Settings::testtrusted(_outside.getaddress()))
						{
							/* Do we want to accept mail to this email address? */
							
							if (!Settings::testacceptance(address))
								throw IllegalRelayingException();

							/* Do the greylisting. */
														
							switch (greylist(_outside.getaddress() & 0xFFFFFF00,
									_from, address))
							{
								case Accepted:
									break;
		
								case GreyListed:
									goto greymessage;
		
								case BlackListed:
									goto blackmessage;
							}
						}
					}
					catch (MalformedAddressException e)
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
			try
			{
				_inside.init(ToAddress);
				_inside.timeout(Settings::sockettimeout());

				/* Check it's okay. */

				readinside();
				_response.check(220, "Invalid banner");
				connected = true;
			}
			catch (NetworkException e)
			{
				/* Something didn't work while making the
				 * connection to the downstream SMTP server.
				 * Bail out cleanly. */
				_response.set(421);
				writeoutside();
				throw e;
			}
		}

		/* These commands require connection to the inside server. */

		switch (_command.cmd())
		{
			case SMTPCommand::AUTH:
				if (!Settings::externalauth())
				{
					/* Someone's tried to use AUTH,
					 * but we don't support it. */
				
					deferrederror.set(500);
					goto error;
				}
			
				/* Proxy the AUTH request through to
				 * the inside, and the reply. */
				
				writeinside();
				readinside();
				writeoutside();

				/* Are we allowed to authenticate? */

				if (_response.code() != 334)
				{
					/* Continue, rather than a jump to
					 * error, because we need to allow
					 * the user to try multiple auth
					 * types until one works. */
					continue;
				}

				/* Perform the authentication. */

				do
				{
					/* Proxy the client's string. */

					{
						string s = _outside.readline();
						_inside.writeline(s);
					}

					/* Proxy the response. */

					readinside();
					writeoutside();
				}
				while (_response.code() == 334);
				if (_response.code() != 235)
					continue;

				/* Hurray! */

				authenticated = true;
				continue;
		}

		/* Pass on the command, and then fetch the result. */
		
		writeinside();
		readinside();
		
		/* Special tweaks if this is a 250 response; a reply to EHLO or HELO. */
		
		if (_response.code() == 250)
		{
			/* If this is an EHLO, the downstream server will probably have
			 * responded with a huge great wodge of text telling us what it
			 * can do. We want to throw most of this away and replace it with
			 * our own. */
			 
			if (_command.cmd() == SMTPCommand::EHLO)
			{
				vector<string> newcontinuation;
				
				/* Do we want to advertise TLS? */
				
#ifdef GNUTLS
				if (Settings::externaltls())
					newcontinuation.push_back("STARTTLS");
#endif

				/* Do we want to advertise AUTH? */
				
				if (Settings::externalauth())
				{
					/* Find the AUTH line and copy it into the new continuation. */
					
					string auth;
					vector<string>& continuation = _response.continuation();
					for (vector<string>::size_type i=0; i<continuation.size(); i++)
					{
						string s = continuation[i];
						if (s.compare(0, 5, "AUTH ") == 0)
						{
							auth = s;
							break;
						}
					}
					
					if (auth != "")
						newcontinuation.push_back(auth);
				}
				
				/* Replace the existing continuation with our new one. */
				
				_response.continuationoverride(newcontinuation);
			}
				
			/* If the command is a EHLO or HELO, then the RFC is a bit
			 * vague about what the response should be. In one part it
			 * says that only the response code should be valid, but in
			 * another it explicitly states the format that the EHLO
			 * response should have... and yes, there are broken mailers
			 * that require that format. So we need to override the
			 * default minimalist response if the command was EHLO or HELO
			 * here.
			 */
	
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
 * Revision 1.15  2007/02/01 18:41:48  dtrg
 * Reworked the SMTP AUTH code so that spey automatically figures out what
 * authentication mechanisms there are by asking the downstream server. The
 * external-auth setting variable is now a boolean. Rearranged various
 * other bits of code and fixed a lot of problems with the man pages.
 *
 * Revision 1.14  2007/01/31 12:58:25  dtrg
 * Added basic support for upstream AUTH requests based on Juan José
 * Gutiérrez de Quevedoo (juanjo@iteisa.com's patch. AUTH requests are
 * proxied through to the downstream server. Parts of the code still need a
 * rethink but it should all work.
 *
 * Revision 1.13  2007/01/29 23:05:10  dtrg
 * Due to various unpleasant incompatibilities with ucontext, the
 * entire coroutine implementation has been rewritten to use
 * pthreads instead of user-level scheduling. This should make
 * things far more robust and portable, if a bit more heavyweight.
 * It also has the side effect of drastically simplified threadlet code.
 *
 * Revision 1.12  2006/04/26 15:24:16  dtrg
 * Fixed the EHLO/HELO response override; I was returning the incorrect domain.
 * This was causing some MTAs (such as the one Debian uses on murphy.debian.org)
 * to think Spey was evil and just give up, losing the email.
 *
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
 */
