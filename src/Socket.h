/* Socket.h
 * Wrapper around a generic TCP socket.
 *
 * Copyright (C) 2004 David Given
 * You may distribute under the terms of the GNU General Public
 * License version 2 as specified in the file COPYING that comes with the
 * Spey distribution.
 *
 * $Source$
 * $State$
 */

#ifndef SOCKET_H
#define SOCKET_H

#ifdef GNUTLS
#include <gnutls/gnutls.h>
#include <gcrypt.h>
#endif

struct SocketServer;

struct Socket : uncopyable
{
	Socket();
	~Socket();

	void init(int fd);
	void init(SocketAddress& address);
	void init(int fd, SocketAddress& address);
	void deinit();

	int read(void* buffer, int buflength, int timeoutdelta = -1);
	int write(void* buffer, int buflength);
	string readline();
	void writeline(string l);

	const SocketAddress& getaddress() { return _address; }
	int timeout() { return _timeout; }
	void timeout(int t) { _timeout = t; }

	int getfd() { return _fd; }
	bool connected() { return _fd != -1; }

protected:
	SocketAddress _address;
	int _fd;
	int _timeout;

#ifndef GNUTLS
	public:
		bool issecure() { return false; }
#else
	public:
		bool issecure() { return _issecure; }
		void makesecure();
		
	protected:
		bool _issecure;
		gnutls_session_t _gnutls_session;

		gnutls_certificate_credentials_t _gnutls_certificate_credentials;
		gnutls_anon_server_credentials_t _gnutls_anonymous_credentials;
#endif	
};

#endif

/* Revision history
 * $Log$
 * Revision 1.11  2007/04/19 09:46:12  dtrg
 * Fixed a bug where we were forgetting to tell gnutls that we were a
 * multithreaded application, resulting in it stepping on gcrypt's toes
 * and crashing at irregular intervals.
 *
 * Revision 1.10  2007/02/10 19:46:44  dtrg
 * Added greet-pause support. Moved the trusted hosts check to right after
 * connection so that greet-pause doesn't apply to trusted hosts. Fixed a bug
 * in the AUTH supported that meant that authenticated connections had no
 * extra privileges (oops). Added the ability to reset all statistics on demand.
 *
 * Revision 1.9  2007/02/10 00:24:35  dtrg
 * Added support for TLS connections using the GNUTLS library. A X509
 * certificate and private key must be supplied for most purposes, but if they
 * are not provided anonymous authentication will be used instead (which
 * apparently only GNUTLS supports). Split the relay check up into two
 * separate parts; the trustedhosts table now specifies machines that can be
 * trusted to play nice, and can do relaying and be allowed to bypass the
 * greylisting; and allowedrecipients, which specifies what email address we're
 * expecting to receive. Also fixed some remaining niggles in the AUTH
 * proxy support, but this remains largely untested.
 *
 * Revision 1.8  2007/02/01 18:41:49  dtrg
 * Reworked the SMTP AUTH code so that spey automatically figures out what
 * authentication mechanisms there are by asking the downstream server. The
 * external-auth setting variable is now a boolean. Rearranged various
 * other bits of code and fixed a lot of problems with the man pages.
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
 * Revision 1.6  2004/06/09 18:40:36  dtrg
 * Fixed some tracing where the address of incoming connections was being reported
 * incorrectly in the logs (but correctly in the Received lines of incoming
 * messages).
 *
 * Revision 1.5  2004/06/08 19:58:04  dtrg
 * Fixed a bug where the address of incoming connections was thought to be the
 * address of *this* end of the connection, not the other end. In the process,
 * changed some this->blah instance variables to _blah.
 *
 * Revision 1.4  2004/05/30 01:55:13  dtrg
 * Numerous and major alterations to implement a system for processing more than
 * one message at a time, based around coroutines. Fairly hefty rearrangement of
 * constructors and object ownership semantics. Assorted other structural
 * modifications.
 *
 * Revision 1.3  2004/05/14 23:11:44  dtrg
 * Added decent relaying support. Also converted SocketAddress to use references a
 * lot rather than pass-by-value, out of general tidiness and the hope that it
 * will improve performance a bit.
 *
 * Revision 1.2  2004/05/14 21:28:22  dtrg
 * Added the ability to create a Socket from a raw file descriptor (needed for
 * inetd mode, where we're going to have a socket passed to us on fd 0).
 *
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 */
