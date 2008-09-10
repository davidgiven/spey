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

	{
		Threadlet::Concurrent c;

		for (;;) {
			int delay = timeout - now();
			if (delay < 0)
				throw NetworkTimeoutException();
	
			struct pollfd p;
			p.fd = _fd;
			p.events = POLLIN | POLLERR | POLLHUP | POLLPRI;
	
			if (poll(&p, 1, delay) != 0)
				break;
		}
	}

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

	Threadlet::Concurrent c;

#ifdef GNUTLS
	if (_issecure)
		return gnutls_record_send(_gnutls_session, buffer, buflength);
	else
#endif
		return ::write(_fd, buffer, buflength);
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
		if (access(privatekeyfile.c_str(), R_OK) == -1)
			throw NetworkException("TLS enabled, but private key is not readable");
		string certificatefile = Settings::tlscertificatefile();
		if (access(privatekeyfile.c_str(), R_OK) == -1)
			throw NetworkException("TLS enabled, but certificate is not readable");
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
	
	int e;
	{
		Threadlet::Concurrent c;
		e = gnutls_handshake(_gnutls_session);
	}

	MessageLog() << "Finished TLS handshake: "
	             << gnutls_strerror(e); 

	if (e < 0)
		throw NetworkException("Unable to perform TLS handshake");
}
#endif
