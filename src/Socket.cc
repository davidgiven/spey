/* Socket.cc
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

#include "spey.h"
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <time.h>

#ifdef GNUTLS
/* The number of bits to use for Diffie-Helmann key exchange.
 * (From the example.) */
 
#define DH_BITS 1024
#endif

/* Create a new, uninitialised socket. */

Socket::Socket()
{
	_fd = -1;
	_timeout = 0;
#ifdef GNUTLS
	_issecure = false;
#endif
}

/* Destroy an existing socket. */

Socket::~Socket()
{
	deinit();
}

/* Initialise socket from the given file descriptor. */

void Socket::init(int fd)
{
	deinit();
	_fd = fd;
	_timeout = 0;

	DetailLog() << "new explicit slave connection on "
		    << fd;
}

/* Initialise the socket from the given file descriptor, but also mark it as
 * coming from the specified address. */

void Socket::init(int fd, SocketAddress& address)
{
	deinit();

	DetailLog() << "new explicit incoming connection on "
		    << fd
		    << " marked as being from "
		    << address;
	_fd = fd;
	_address = address;
}

/* Initialise the socket to be an outgoing connection to the specified address.
 * */

void Socket::init(SocketAddress& address)
{
	deinit();

	_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (_fd == -1)
		throw NetworkException("Error creating outbound socket", errno);
	DetailLog() << "outbound socket created on fd "
		    << _fd
		    << " to "
		    << address;

	if (address.connectto(_fd) != 0)
	{
		close(_fd);
		_fd = -1;
		throw NetworkException("Error connecting to remote server", errno);
	}
}

void Socket::deinit()
{
#ifdef GNUTLS
	if (_issecure)
	{
		gnutls_deinit(_gnutls_session);
		
		if (_gnutls_certificate_credentials)
		{
			gnutls_certificate_free_credentials(_gnutls_certificate_credentials);
			_gnutls_certificate_credentials = NULL;
		}
		
		if (_gnutls_anonymous_credentials)
		{
			gnutls_anon_free_server_credentials(_gnutls_anonymous_credentials);
			_gnutls_anonymous_credentials = NULL;
		}
		
		_issecure = false;
	}
#endif

	if (_fd != -1)
	{
		close(_fd);
		_fd = -1;
	}
}

/* Return the current time in milliseconds since epoch. */

static uint64_t now()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return ((uint64_t)tv.tv_sec)*1000 + (tv.tv_usec/1000);
}
	
/* Read data from the socket. */

int Socket::read(void* buffer, int buflength, int timeoutdelta)
{
	/* When does the timeout take place? */

	if (timeoutdelta == -1)
		timeoutdelta = _timeout;
		
	uint64_t timeout = now() + (uint64_t)(timeoutdelta*1000);

	/* Read data in concurrent mode. */

	Threadlet::releaseCPUlock();
	for (;;) {
		int delay = timeout - now();
		if (delay < 0)
		{
			Threadlet::takeCPUlock();
			throw NetworkTimeoutException();
		}

		struct pollfd p;
		p.fd = _fd;
		p.events = POLLIN | POLLERR | POLLHUP | POLLPRI;

		if (poll(&p, 1, delay) != 0)
			break;
	}
	Threadlet::takeCPUlock();

#ifdef GNUTLS
	if (_issecure)
		return gnutls_record_recv(_gnutls_session, buffer, buflength);
#endif
	return ::read(_fd, buffer, buflength);
}

/* Write data to the socket. */

int Socket::write(void* buffer, int buflength)
{
	/* Wait in concurrent mode. */

	Threadlet::releaseCPUlock();
	int e;
#ifdef GNUTLS
	if (_issecure)
		e = gnutls_record_send(_gnutls_session, buffer, buflength);
	else
#endif
		e = ::write(_fd, buffer, buflength);
	Threadlet::takeCPUlock();

	return e;
}

/* Read a line of text from the socket; doesn't include the newline (or CRLF)
 * at the end. */

string Socket::readline()
{
	stringstream s;

	for (;;)
	{
		char c;
		if (read(&c, 1) != 1)
			goto eof;
		
		if (c == 13)
			continue;
		if (c == 10)
			break;

		s << c;
	}

	return s.str();

eof:
	throw NetworkException("Socket unexpectedly closed");
}

/* Write a line of text to the socket; a CRLF is added to the end. */

void Socket::writeline(string l)
{
	stringstream s;
	char c;

	for (string::size_type i=0; i<l.length(); i++)
	{
		c = l[i];
		
		/* Convert \n into \r\n. */
		
		if (c == 10)
			s << (char) 13;

		s << c;		
	}

	s << (char) 13
	  << (char) 10;
	
	string ss = s.str();
	if (write((void*) ss.c_str(), ss.size()) == -1)
		throw NetworkException("Socket unexpectedly closed");
}

/* --------------------------- GNU TLS support --------------------------- */

#ifdef GNUTLS

static gnutls_dh_params_t get_dh_params()
{
	static gnutls_dh_params_t dh_params;

	/* Ensure that the Diffie-Helmann parameters have been made and that
	 * they're not too old. */
	
	static time_t freshness = 0;
	if ((time(NULL) - freshness) < (3600L * 24L))
		return dh_params;
	freshness = time(NULL);
	
	/* Regenerate the DH parameters. */
	
	MessageLog() << "Regenerating Diffie-Helmann parameters";
	
	gnutls_dh_params_init(&dh_params);
	gnutls_dh_params_generate2(dh_params, DH_BITS);
	return dh_params;
}
	
void Socket::makesecure()
{
	/* GNUTLS setup. */
		 
	gnutls_init(&_gnutls_session, GNUTLS_SERVER);
	gnutls_set_default_priority(_gnutls_session);

	/* Make the anonymous credentials. */
	
	gnutls_anon_allocate_server_credentials(&_gnutls_anonymous_credentials);
	if (_gnutls_anonymous_credentials)
	{
		gnutls_anon_set_server_dh_params(_gnutls_anonymous_credentials,	
			get_dh_params());
		gnutls_credentials_set(_gnutls_session, GNUTLS_CRD_ANON,
			_gnutls_anonymous_credentials);
	}
	
	/* Make the certificate credentials. */
	
	gnutls_certificate_allocate_credentials(&_gnutls_certificate_credentials);
	bool certificate_valid = false;
	if (_gnutls_certificate_credentials)
	{
		gnutls_certificate_set_dh_params(_gnutls_certificate_credentials,
			get_dh_params());
	
		string privatekeyfile = Settings::tlsprivatekeyfile();
		string certificatefile = Settings::tlscertificatefile();
		int i = gnutls_certificate_set_x509_key_file(_gnutls_certificate_credentials,
			certificatefile.c_str(), privatekeyfile.c_str(), GNUTLS_X509_FMT_PEM);
		certificate_valid = (i == 0);

		i = gnutls_credentials_set(_gnutls_session, GNUTLS_CRD_CERTIFICATE,
			_gnutls_certificate_credentials);
	}
	
	/* Other setup. */

	if (!certificate_valid)
	{
		WarningLog() << "TLS enabled, but no valid certificate --- using anonymous authentication";
		const int kx_prio[] = { GNUTLS_KX_ANON_DH, 0 };
		gnutls_kx_set_priority(_gnutls_session, kx_prio);
	}

  	gnutls_dh_set_prime_bits(_gnutls_session, DH_BITS);
	gnutls_certificate_server_set_request(_gnutls_session, GNUTLS_CERT_REQUEST);
	
	/* This socket is now marked as secure to ensure that TLS teardown happens
	 * correctly, even if the handshake fails. */
	
	_issecure = true;
	
	/* TLS handshake. */
	
	gnutls_transport_set_ptr(_gnutls_session, (gnutls_transport_ptr_t) _fd);
	
	MessageLog() << "Starting TLS handshake";
	Threadlet::releaseCPUlock();
	int e = gnutls_handshake(_gnutls_session);
	Threadlet::takeCPUlock();
	MessageLog() << "Finished TLS handshake: "
	             << gnutls_strerror(e); 

	if (e < 0)
		throw NetworkException("Unable to perform TLS handshake");
}
#endif

/* Revision history
 * $Log$
 * Revision 1.11  2007/02/10 00:24:35  dtrg
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
 * Revision 1.10  2007/01/29 23:05:10  dtrg
 * Due to various unpleasant incompatibilities with ucontext, the
 * entire coroutine implementation has been rewritten to use
 * pthreads instead of user-level scheduling. This should make
 * things far more robust and portable, if a bit more heavyweight.
 * It also has the side effect of drastically simplified threadlet code.
 *
 * Revision 1.9  2005/11/09 10:35:09  dtrg
 * Fixed a problem with the last checkin --- it seems that \n\r is *not*
 * the same as \r\n... thanks to Bernd Rilling for pointing this out.
 *
 * Revision 1.8  2005/11/09 00:02:43  dtrg
 * Fixed an issue where bare linefeeds were being sent to the internal server
 * as part of spey's synthesised Received line. This was causing
 * standard-checking MTAs such as qmail to die.
 *
 * Revision 1.7  2004/11/18 17:57:20  dtrg
 * Rewrote logging system so that it no longer tries to subclass stringstream,
 * that was producing bizarre results on gcc 3.3. Added version tracking to
 * the makefile; spey now knows what version and build number it is, and
 * displays the information in the startup banner. Now properly ignores
 * SIGPIPE, which was causing intermittent silent aborts.
 *
 * Revision 1.6  2004/06/30 20:18:49  dtrg
 * Changed the way sockets are initialised; instead of doing it from the
 * Socket and SocketServer constructors, they're set up as zombies and
 * initialised later with an init() method. This is cleaner, and also allows a
 * cunning new feature: the connection to the downstream SMTP server is now
 * only made once the first valid SMTP command is received from the upstream
 * SMTP server. This means that connections are only made once we're
 * reasonably sure that there's going to be a valid SMTP conversation, which
 * should harden spey against DoS attacks like the ones I get every so often.
 * Also took the opportunity to convert more this->blah instance variables
 * into _blah.
 *
 * Revision 1.5  2004/06/09 18:40:34  dtrg
 * Fixed some tracing where the address of incoming connections was being
 * reported incorrectly in the logs (but correctly in the Received lines of
 * incoming messages).
 *
 * Revision 1.4  2004/06/08 19:58:04  dtrg
 * Fixed a bug where the address of incoming connections was thought to be the
 * address of *this* end of the connection, not the other end. In the process,
 * changed some this->blah instance variables to _blah.
 *
 * Revision 1.3  2004/05/30 01:55:13  dtrg
 * Numerous and major alterations to implement a system for processing more
 * than one message at a time, based around coroutines. Fairly hefty
 * rearrangement of constructors and object ownership semantics. Assorted
 * other structural modifications.
 *
 * Revision 1.2  2004/05/14 21:28:22  dtrg
 * Added the ability to create a Socket from a raw file descriptor (needed for
 * inetd mode, where we're going to have a socket passed to us on fd 0).
 *
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 */
