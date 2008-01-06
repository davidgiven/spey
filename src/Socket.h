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
