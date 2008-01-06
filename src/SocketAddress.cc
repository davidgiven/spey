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

/* Create a generic socket address. */

SocketAddress::SocketAddress()
{
	_sa.sin_family = AF_INET;
	_sa.sin_addr.s_addr = INADDR_ANY;
	_sa.sin_port = 0;
}

/* Create an address from the given sockaddr_in structure. */

SocketAddress::SocketAddress(const sockaddr_in& sa)
{
	_sa = sa;
}
	
/* Create an address from *this* end of the supplied file descriptor. */

SocketAddress::SocketAddress(int fd)
{
	socklen_t i = sizeof(_sa);
	(void)getsockname(fd, (sockaddr*) &_sa, &i);
	if (i != sizeof(_sa))
		WarningLog() << "Warning: socket size changed from "
			     << sizeof(_sa)
			     << " to "
			     << i
			     << " during call to getsockname()!";
}

/* Create an address from the given hostname and port. */

SocketAddress::SocketAddress(const string& name, int port)
{
	_sa.sin_family = AF_INET;
	this->setname(name);
	this->setport(port);
}

/* Create an address from the combined hostname and port. */

SocketAddress::SocketAddress(const string& name)
{
	_sa.sin_family = AF_INET;
	this->set(name);
}

SocketAddress::~SocketAddress()
{
}

/* Set the hostname part of the address to the supplied string. */

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

	memcpy(&_sa.sin_addr.s_addr, he->h_addr, sizeof(_sa.sin_addr.s_addr));
}

/* Set the port part of the address to the supplied string. */

void SocketAddress::setport(int port)
{
	_sa.sin_port = htons(port);
}

/* Set the complete address to the supplied string. */

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

/* Set the complete address to the supplied sockaddr_in structure. */

void SocketAddress::set(const sockaddr_in& sa)
{
	_sa = sa;
}
	
int SocketAddress::connectto(int fd)
{
	return connect(fd, (sockaddr*) &_sa,
			(socklen_t) sizeof(_sa));
}

int SocketAddress::bindto(int fd)
{
	return bind(fd, (sockaddr*) &_sa,
			(socklen_t) sizeof(_sa));
}

int SocketAddress::acceptfrom(int fd)
{
	socklen_t i = sizeof(_sa);
	int r = accept(fd, (sockaddr*) &_sa, &i);
	if (i != sizeof(_sa))
		WarningLog() << "Warning: socket size changed from "
			     << (int) sizeof(_sa)
			     << " to "
			     << i
			     << " during call to accept()!";
	return r;
}

string SocketAddress::getname() const
{
	unsigned int addr = _sa.sin_addr.s_addr;
	stringstream s;
	bool validname = false;
	
	if (addr != 0x00000000)
	{
	
		struct hostent* he = gethostbyaddr(&_sa.sin_addr.s_addr,
				sizeof(_sa.sin_addr.s_addr), AF_INET);
	
		if (he)
		{
			s << he->h_name;
			validname = true;
		}
	}
	
	if (!validname)
	{
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
	  << ntohs(_sa.sin_port);

	return s.str();
}

SocketAddress::operator unsigned int () const
{
	return ntohl(_sa.sin_addr.s_addr);
}
