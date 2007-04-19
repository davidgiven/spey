/* CLI.gp
 * genparse data file describing the command line parameters Spey accepts.
 *
 * Copyright (C) 2004 David Given
 * You may distribute under the terms of the GNU General Public
 * License version 2 as specified in the file COPYING that comes with the
 * Spey distribution.
 *
 * $Source$
 * $State$
 */

d / database		string {"/var/lib/spey/spey.db"}
	"Spey configuration file."

f / from                string {"0.0.0.0:25"}
	"Address to listen on."

t / to			string {"localhost:2525"}
	"SMTP server to connect to."

v / verbose		int 3 [0...]
	"Set the verbosity level."

x / foreground		flag
	"Run in the foreground instead of as a daemon."

i / inetd		flag
	"Run in inetd mode instead of as a daemon."

/* Revision history
 * $Log$
 * Revision 1.4  2007/01/31 12:51:51  dtrg
 * Dropping root privileges is now the default. The default database file
 * has been moved into /var/lib/spey/spey.db to assist this.
 *
 * Revision 1.3  2004/05/30 01:55:13  dtrg
 * Numerous and major alterations to implement a system for processing more than
 * one message at a time, based around coroutines. Fairly hefty rearrangement of
 * constructors and object ownership semantics. Assorted other structural
 * modifications.
 *
 * Revision 1.2  2004/05/14 22:01:40  dtrg
 * Added inetd mode, where one message is processed from stdin and then spey exits. Also added proper daemon functionality where spey detaches itself cleanly from the console to go into the background.
 *
 * Revision 1.1  2004/05/01 12:20:20  dtrg
 * Initial version.
 */
