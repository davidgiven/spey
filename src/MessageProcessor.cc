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

MessageProcessor::MessageProcessor(int fd, SocketAddress& address):
	inside(ToAddress),
	outside(fd)
{
	outside.setaddress(address);
	inside.timeout(Settings::sockettimeout());
	outside.timeout(Settings::sockettimeout());

	Threadlet::addthreadlet(this);
}

MessageProcessor::~MessageProcessor()
{
}

void MessageProcessor::readinside()
{
	this->response.set(this->inside);
	SMTPLog() << "s<i "
		  << this->response
		  << flush;
}

void MessageProcessor::writeinside()
{
	this->inside.writeline(this->command);
	SMTPLog() << "s>i "
		  << this->command
		  << flush;
}

void MessageProcessor::readoutside()
{
	this->command.set(this->outside);
	SMTPLog() << "o>s "
		  << this->command
		  << flush;
}

void MessageProcessor::writeoutside()
{
	this->outside.writeline(this->response);
	SMTPLog() << "o<s "
		  << this->response
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
		    << outside.getaddress().getname()
		    << " for relaying"
		    << flush;

	if (!Settings::testrelay(outside.getaddress(), address))
		throw IllegalRelayingException();
}

void MessageProcessor::process()
{
	SMTPResponse deferrederror;
	bool errorstate = 0;
	
	readinside();
	response.check(220, "Invalid banner");
	response.parmoverride(Settings::identity());
	writeoutside();

	for (;;)
	{
		try {
			readoutside();
		} catch (InvalidSMTPCommandException e)
		{
			response.set(500);
			writeoutside();
			if (Settings::intolerant())
				goto abort;
			continue;
		}

		switch (command.cmd())
		{
			case SMTPCommand::HELP:
				response.set(214);
				writeoutside();
				continue;

			case SMTPCommand::HELO:
			case SMTPCommand::EHLO:
				try {
					verifydomain(command.arg());
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
					from = command.arg();
					if (from != "")
						verifyaddress(from);
				} catch (MalformedAddressException e)
				{
					deferrederror.set(551);

					Statistics::malformedAddress();
					goto error;
				}

				break;

			case SMTPCommand::RCPT:
				try {
					string address = command.arg();

					verifyaddress(address);
					verifyrelay(address);

					if (greylist(outside.getaddress() & 0xFFFFFF00,
							from, address))
						goto greymessage;
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
				from = "";
				break;

			case SMTPCommand::DATA:
				if (errorstate)
					goto error;
				break;

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
				}
		}

		writeinside();

		readinside();
		writeoutside();

		if (response.iserror() && Settings::intolerant())
			goto abort;
		
		switch (command.cmd())
		{
			case SMTPCommand::DATA:
			{
				if (!response.issuccess())
					break;

				SMTPLog() << "transferring message body"
					  << flush;

				/* Add our Received: header. */

				{
					stringstream s;
					s << "Received: from "
					  << outside.getaddress().getname()
					  << " and verified by Spey"
					  << "\n\tconnected from "
					  << (string) outside.getaddress()
					  << "\n\twith envelope "
					  << from;
					this->inside.writeline(s.str());
				}

				for (;;)
				{
					string s = this->outside.readline();
					this->inside.writeline(s);
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
		response = deferrederror;
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
	return outside.getfd();
}

void MessageProcessor::run()
{
	process();
}

/* Revision history
 * $Log$
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
