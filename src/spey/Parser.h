/* Parser.h
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

#ifndef PARSER_H
#define PARSER_H

struct Parser : uncopyable
{
	Parser(string data);

	void seek(string::size_type i);
	string::size_type tell();
	bool compare(string s);
	void expect(string s);
	void whitespace();
	int peek();
	string getword(int (*predicate)(int) = NULL, char delimiter=' ');
	void eol();

protected:
	string data;
	string::size_type index;
	string::size_type length;
};

#endif
