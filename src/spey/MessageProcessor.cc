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
	
	/* Full RFC2822 email address verification is actually extremely
	 * complicated, and we're not going to bother for now. */
}

void MessageProcessor::process()
{
	SMTPResponse deferrederror;
	bool errorstate = false;
	bool connected = false;
	bool authenticated = false;
	bool trusted = Settings::testtrusted(_outside.getaddress());
	 
	if (trusted)
		MessageLog() << "This connection is trusted";
		
	/* Do any RBL checking. */
	
	if (!trusted)
	{
		stringstream rbllist(Settings::rbllist());
		rbllist >> skipws;

		for (;;) 
		{
			/* Read a word. */
			
			string rbl;
			rbllist >> rbl;
			if (rbllist.fail())
				break;
				
			/* Run it through the RBL checker. */
			
			if (rblcheck(_outside.getaddress(), rbl))
			{
				deferrederror.set(554);
				stringstream s;
				s << "5.7.1 Rejected: your IP address is listed in the "
				  << rbl
				  << " RBL";
				deferrederror.msgoverride(s.str());
				Statistics::blackholed();
				goto error;
			}
		}
	}
	
	/* Do the greet-pause. */
	
	if (!trusted)
	{
		int delay = Settings::greetpause();
		if (delay > 0)
		{
			MessageLog() << "doing greet-pause of " << delay << " seconds";
			
			try
			{
				char buf[1];
				_outside.read(buf, 1, delay);
				
				/* On timeout, read() will throw an exception. So if we get
				 * here, data has been read. */
				
				deferrederror.set(554);
				deferrederror.msgoverride("5.7.1 Rejected: protocol violation");
				Statistics::spokeTooSoon();
				goto error;
			}
			catch (NetworkTimeoutException e)
			{
				/* do nothing --- this is what we expected */
			}
		}
	}
	
	/* Write out the banner. */
	
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

			case SMTPCommand::QUIT:
				return;
				
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
					if (!authenticated && !trusted)
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
				if (!authenticated && !trusted)
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
				if (!authenticated && !trusted)
				{
					try {
						string address = _command.arg();
	
						verifyaddress(address);
						
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
					errorstate = true;

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
					errorstate = true;

					Statistics::greylisted();
					break;
				}
		}

		/* Make sure we're connected to the downstream SMTP server. */

		if (!connected)
		{
			try
			{
				_inside.init(ServerProcessor::ToAddress);
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
				switch (Settings::externalauthmode())
				{
					case NoAuth:
					{
						/* Someone's tried to use AUTH,
						 * but we don't support it. */
					
						deferrederror.set(500);
						goto error;
					}

					case ProxyAuth:
					{
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
					
					case InternalAuth:
					{
						/* Because we only support plain text logins,
						 * decline to authenticate if we're not using a secure
						 * connection. */
						
						if (!_outside.issecure())
						{
							deferrederror.set(538);
							goto error;
						}
		
						/* Determine which authenticator to use. */

						Authenticator* auth_cb = NULL;
						string s = _command.arg();
						if (s.compare(0, 6, "plain ") == 0)
						{
							auth_cb = auth_plain;
							s = s.substr(6);
						}
						else if (s == "plain")
						{
							auth_cb = auth_plain;
							s = "";
						}
						else if (s == "login")
						{
							auth_cb = auth_login;
							s = "";
						}
							
						if (!auth_cb)
						{
							/* Unrecognised authentication type. */
							deferrederror.set(535);
							goto error;
						}
						
						/* Now we have an authenticator, run it. */
						
						int result;
						try
						{
							result = auth_cb(_outside, s);
						}
						catch (const AuthenticationCancelledException& e)
						{
							result = 501;
						}
						
						if (result != 235)
						{
							deferrederror.set(result);
							goto error;
						}
						else
						{
							_response.set(result);
							writeoutside();
							
							/* Hurray! */
							
							MessageLog() << "connection authenticated";
							authenticated = true;
						}
						
						continue;
					}
				}
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
				
				switch (Settings::externalauthmode())
				{
					case NoAuth: /* No. */
						break;
						
					case ProxyAuth: /* Yes; advertise whatever downstream
					                 * does. */
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
						break;
					}
					
					case InternalAuth: /* We're doing our own PLAIN auth. */
					{
						newcontinuation.push_back("AUTH PLAIN LOGIN");
						break;
					}
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

				MessageLog() << "transferring message body";

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

				MessageLog() << "body transferred";

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
		errorstate = false;
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
	MessageLog() << "connection from " << (SocketAddress&) _outside.getaddress();
	process();
	MessageLog() << "connection terminating";
}
