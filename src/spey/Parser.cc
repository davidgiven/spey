/* Parser.cc
 * Simple string parser.
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
#include <ctype.h>

Parser::Parser(string data)
{
	this->data = data;
	this->index = 0;
	this->length = data.length();
}

void Parser::seek(string::size_type i)
{
	if ((i < 0) || (i >= length))
		throw ParseErrorException();
	index = i;
}

string::size_type Parser::tell()
{
	return index;
}

bool Parser::compare(string s)
{
	string::size_type oldindex = index;

	try {
		expect(s);
		return 1;
	} catch (ParseErrorException e)
	{
		index = oldindex;
		return 0;
	}
}

void Parser::expect(string s)
{
	for (string::size_type i=0; i<s.length(); i++)
	{
		if (index >= length)
			throw ParseErrorException();
		if (tolower(data[index]) != tolower(s[i]))
			throw ParseErrorException();
		index++;
	}
}

void Parser::whitespace()
{
	for (;;)
	{
		if (index >= length)
			throw ParseErrorException();

		char c = data[index];
		if (c != ' ')
			return;
		index++;
	}
}

int Parser::peek()
{
	if (index >= length)
		return 0;
	return data[index];
}

string Parser::getword(int (*predicate)(int), char delimiter)
{
	stringstream s;

	for (;;)
	{
		if (index >= length)
			break;

		char c = data[index];
		if (c == delimiter)
			break;

		if (predicate)
			c = predicate(c);
		
		s << (char)c;
		index++;
	}

	return s.str();
}

void Parser::eol()
{
	for (;;)
	{
		if (index >= length)
			return;

		char c = data[index];
		if (c != ' ')
			throw ParseErrorException();
		index++;
	}
}
