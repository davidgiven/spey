/* SocketAddress.cc
 * Wrapper around a generic IPv4 socket address.
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
#include <netdb.h>

SocketAddress::SocketAddress()
{
	this->sa.sin_family = AF_INET;
	this->sa.sin_addr.s_addr = INADDR_ANY;
	this->sa.sin_port = 0;
}

SocketAddress::SocketAddress(int fd)
{
	socklen_t i = sizeof(this->sa);
	(void)getsockname(fd, (sockaddr*) &this->sa, &i);
	if (i != sizeof(this->sa))
		WarningLog() << "Warning: socket size changed from "
			     << sizeof(this->sa)
			     << " to "
			     << i
			     << " during call to getsockname()!"
			     << flush;
}

SocketAddress::SocketAddress(const string& name, int port)
{
	this->sa.sin_family = AF_INET;
	this->setname(name);
	this->setport(port);
}

SocketAddress::SocketAddress(const string& name)
{
	this->sa.sin_family = AF_INET;
	this->set(name);
}

SocketAddress::~SocketAddress()
{
}

void SocketAddress::setname(const string& server)
{
	struct hostent* he = gethostbyname(server.c_str());
	if (!he)
	{
		string s = "Host '";
		s += server;
		s += "' not found";
		throw NetworkException(s);
	}

	memcpy(&this->sa.sin_addr.s_addr, he->h_addr, he->h_length);
}

void SocketAddress::setport(int port)
{
	this->sa.sin_port = htons(port);
}

void SocketAddress::set(const string& name)
{
	string::size_type i = name.find(':');
	if (i == string::npos)
	{
		this->setname(name);
		return;
	}

	string port = name.substr(i+1);
	string host = name.substr(0, i);
	
	if (host != "")
		this->setname(host);
	this->setport(atoi(port.c_str()));
}

int SocketAddress::connectto(int fd)
{
	return connect(fd, (sockaddr*) &this->sa,
			(socklen_t) sizeof(this->sa));
}

int SocketAddress::bindto(int fd)
{
	return bind(fd, (sockaddr*) &this->sa,
			(socklen_t) sizeof(this->sa));
}

int SocketAddress::acceptfrom(int fd)
{
	socklen_t i = sizeof(this->sa);
	int r = accept(fd, (sockaddr*) &this->sa, &i);
	if (i != sizeof(this->sa))
		WarningLog() << "Warning: socket size changed from "
			     << sizeof(this->sa)
			     << " to "
			     << i
			     << " during call to accept()!"
			     << flush;
	return r;
}

string SocketAddress::getname() const
{
	struct hostent* he = gethostbyaddr(&this->sa.sin_addr.s_addr,
			sizeof(this->sa.sin_addr.s_addr), AF_INET);
	stringstream s;

	if (he)
		s << he->h_name;
	else
	{
		unsigned int addr = this->sa.sin_addr.s_addr;

		s << ((addr >>  0) & 0xFF)
		  << '.'
		  << ((addr >>  8) & 0xFF)
		  << '.'
		  << ((addr >> 16) & 0xFF)
		  << '.'
		  << ((addr >> 24) & 0xFF);
	}

	return s.str();
}

SocketAddress::operator string () const
{
	stringstream s;
	s << getname()
	  << ':'
	  << ntohs(this->sa.sin_port);

	return s.str();
}

SocketAddress::operator unsigned int () const
{
	return ntohl(sa.sin_addr.s_addr);
}

/* Revision history
 * $Log$
 * Revision 1.2  2004/05/14 21:28:22  dtrg
 * Added the ability to create a Socket from a raw file descriptor (needed for
 * inetd mode, where we're going to have a socket passed to us on fd 0).
 *
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 */
