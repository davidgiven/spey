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
	string getword(char delimiter=' ');
	void eol();

protected:
	string data;
	string::size_type index;
	string::size_type length;
};

#endif

/* Revision history
 * $Log$
 * Revision 1.2  2004/06/22 21:01:02  dtrg
 * Made a lot of minor tweaks so that spey now builds under gcc 3.3. (3.3 is a lot
 * closer to the C++ standard than 2.95 is; plus, the standard library is now
 * rather different, which means that I'm not allowed to do things like have local
 * variables called errno.)
 *
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 */

